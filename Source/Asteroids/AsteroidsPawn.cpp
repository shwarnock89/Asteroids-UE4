
#include "AsteroidsPawn.h"

#include "Asteroid.h"
#include "AsteroidsGameInstance.h"
#include "AsteroidsProjectile.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Utils/AsteroidEntitySpawned.h"
#include "Utils/HighScoreCalculator.h"

UE_DEFINE_GAMEPLAY_TAG(FireComponentTag, "Component.Fire");
UE_DEFINE_GAMEPLAY_TAG(SmokeComponentTag, "Component.Smoke");
UE_DEFINE_GAMEPLAY_TAG(ExplosionComponentTag, "Component.Explosion");
UE_DEFINE_GAMEPLAY_TAG(ShipComponentTag, "Component.Ship");

AAsteroidsPawn::AAsteroidsPawn()
{
	// Movement
	MoveSpeed = FVector::ZeroVector;

	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;

	PlayerMaxHealth = 100.0f;
	PlayerCurrentHealth = 100.0f;

	PlayerMaxShields = 100.0f;
	PlayerCurrentShields = 100.0f;
	ShieldRegenDelay = 6.0f;
	ShieldRegenTimer = 0.0f;
	ShieldRegenRate = 10.0f;
	bShieldTimerActive = false;
	DamageTimeDelay = 1;
	CurrentDamageTimeDelay = 0;
	bDamageTimerActive = false;

	CurrentBullets = 0;

	PlayerScore = 0;

	bIsDead = false;
}

void AAsteroidsPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent)

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!ensureAlways(IsValid(PlayerController)))
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!ensureAlways(IsValid(Subsystem)))
	{
		return;
	}

	Subsystem->AddMappingContext(DefaultMappingContext, 0);

	// Cast to EnhancedInputComponent
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	if (!ensureAlways(IsValid(EnhancedInputComponent)))
	{
		return;
	}

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAsteroidsPawn::HandleMovement);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AAsteroidsPawn::FireShot);
}

void AAsteroidsPawn::HandleMovement(const FInputActionValue& Value)
{
	LastInput = Value.Get<FVector2D>();
}

void AAsteroidsPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FireComponent = FindComponentByTag<UParticleSystemComponent>(FireComponentTag.GetTag().GetTagName());
	ExplosionComponent = FindComponentByTag<UParticleSystemComponent>(ExplosionComponentTag.GetTag().GetTagName());
	SmokeComponent = FindComponentByTag<UParticleSystemComponent>(SmokeComponentTag.GetTag().GetTagName());
	ShipMeshComponent = FindComponentByTag<UStaticMeshComponent>(ShipComponentTag.GetTag().GetTagName());
}

void AAsteroidsPawn::BeginPlay()
{
	Super::BeginPlay();

	UAsteroidEntitySpawnerSubsystem::OnAsteroidEntitySpawned.ExecuteIfBound(this, EHandlingType::Flip);

	UCapsuleComponent* CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AAsteroidsPawn::OnBeginOverlap);
	CapsuleComponent->OnComponentEndOverlap.AddDynamic(this, &AAsteroidsPawn::OnEndOverlap);

	FVector WorldLocation = GetActorLocation();
	WorldLocation.Z = 0.0f;
	SetActorLocation(WorldLocation);

	const UAsteroidsGameInstance* GameInstance = static_cast<UAsteroidsGameInstance*>(GetWorld()->GetGameInstance());
	Messenger = GameInstance->GetMessanger();
	Messenger->OnFireButtonPressed.AddDynamic(this, &AAsteroidsPawn::FireShot);
	Messenger->OnBulletDestroyed.AddDynamic(this, &AAsteroidsPawn::HandleBulletDestroyed);
	Messenger->OnUpdatePlayerScore.AddDynamic(this, &AAsteroidsPawn::HandleUpdatePlayerScore);
	Messenger->OnHealthPackPickedUp.AddDynamic(this, &AAsteroidsPawn::HandleHealthPackPickedUp);

	if (!ensureAlways(IsValid(ExplosionComponent)))
	{
		return;
	}

	ExplosionComponent->DeactivateSystem();
}

void AAsteroidsPawn::HandleBulletDestroyed(const FMessage Message)
{
	CurrentBullets -= Message.intMessage;
}

void AAsteroidsPawn::DestroyPawn()
{
	Destroy();
}

void AAsteroidsPawn::DealDamage(const float Damage)
{
	if (!bDamageTimerActive)
	{
		if (PlayerCurrentShields > 0.0f)
		{
			ShieldRegenTimer = 0.0f;
			bShieldTimerActive = true;
			PlayerCurrentShields -= Damage;
			FMessage Message;
			Message.floatMessage = PlayerCurrentShields / PlayerMaxShields;
			Message.typeMessage = EMessageTypes::Float;
			Messenger->ShieldsUpdated(Message);
			return;
		}

		PlayerCurrentHealth -= Damage;

		if (PlayerCurrentHealth <= 0.0f)
		{
			if (UHighScoreCalculator::IsNewHighScore(PlayerScore) && !bIsDead)
			{
				FMessage Message;
				Message.intMessage = PlayerScore;
				Messenger->NewHighScore(Message);
			}
			else if (!bIsDead)
			{
				FMessage Message = FMessage();
				Message.intMessage = PlayerScore;
				Messenger->PlayerDied(Message);
			}

			if (!ensureAlways(IsValid(SmokeComponent) && IsValid(FireComponent)))
			{
				return;
			}

			SmokeComponent->SetHiddenInGame(true);
			FireComponent->SetHiddenInGame(true);

			FTimerHandle UnusedHandle;
			GetWorldTimerManager().SetTimer(UnusedHandle, 1.0f, false);

			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (!ensureAlways(IsValid(PlayerController)))
			{
				return;
			}

			DisableInput(PlayerController);
			MoveSpeed = FVector::ZeroVector;

			if (!ensureAlways(IsValid(ShipMeshComponent)))
			{
				return;
			}

			ShipMeshComponent->SetHiddenInGame(true);

			if (!ensureAlways(IsValid(ExplosionComponent)))
			{
				return;
			}

			ExplosionComponent->ActivateSystem();

			GetWorldTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateUObject(this, &AAsteroidsPawn::DestroyPawn), 1.0f, false);

			bIsDead = true;
		}
		else
		{
			ShieldRegenTimer = 0.0f;
			bShieldTimerActive = true;
			FMessage Message = FMessage();
			Message.floatMessage = PlayerCurrentHealth / PlayerMaxHealth;
			Messenger->UpdatePlayerHealth(Message);
		}

		if (PlayerCurrentHealth < 50.0f)
		{
			if (!ensureAlways(IsValid(SmokeComponent)))
			{
				return;
			}

			SmokeComponent->ActivateSystem();
		}

		if (PlayerCurrentHealth < 25.0f)
		{
			if (!ensure(IsValid(SmokeComponent) && IsValid(FireComponent)))
			{
				return;
			}

			SmokeComponent->DeactivateSystem();
			FireComponent->ActivateSystem();
		}
	}
}

void AAsteroidsPawn::HandleUpdatePlayerScore(FMessage Message)
{
	PlayerScore += Message.intMessage;

	Message.intMessage = PlayerScore;
	Messenger->PlayerScoreWasUpdated(Message);
}

void AAsteroidsPawn::Tick(const float DeltaSeconds)
{
	if (bDamageTimerActive)
	{
		CurrentDamageTimeDelay += DeltaSeconds;
	}

	if (CurrentDamageTimeDelay >= DamageTimeDelay)
	{
		bDamageTimerActive = false;
		CurrentDamageTimeDelay = 0;
	}

	if (bShieldTimerActive)
	{
		ShieldRegenTimer += DeltaSeconds;
	}
	else
	{
		RegenerateShields(DeltaSeconds);
	}

	if (ShieldRegenTimer >= ShieldRegenDelay)
	{
		ShieldRegenTimer = 0.0f;
		bShieldTimerActive = false;
	}

	// 1. Apply Rotation
	if (!FMath::IsNearlyZero(LastInput.X))
	{
		AddActorLocalRotation(FRotator(0.f, LastInput.X * RotationSpeed * DeltaSeconds, 0.f));
	}

	// 2. Apply Thrust to Velocity
	if (!FMath::IsNearlyZero(LastInput.Y))
	{
		const FVector Forward = GetActorForwardVector();
		MoveSpeed += Forward * ThrustStrength * LastInput.Y * DeltaSeconds;
	}

	// 3. Apply persistent Movement (The Drift)
	if (!MoveSpeed.IsNearlyZero())
	{
		AddActorWorldOffset(MoveSpeed * DeltaSeconds, true);

		// 4. Apply a tiny bit of Space Drag
		MoveSpeed -= MoveSpeed * 0.1f * DeltaSeconds;
	}

	// Reset inputs so they don't "stick" if HandleMovement isn't called next frame
	LastInput = FVector2D::ZeroVector;

	if (bIsOverlappingAsteroid && !bDamageTimerActive)
	{
		DealDamage(10);
		bDamageTimerActive = true;
	}
}

void AAsteroidsPawn::RegenerateShields(const float DeltaSeconds)
{
	if (PlayerCurrentShields < PlayerMaxShields)
	{
		PlayerCurrentShields += ShieldRegenRate * DeltaSeconds;
		if (PlayerCurrentShields > 100.0f)
		{
			PlayerCurrentShields = 100.0f;
		}
		FMessage Message;
		Message.floatMessage = PlayerCurrentShields / PlayerMaxShields;
		Message.typeMessage = EMessageTypes::Float;
		Messenger->ShieldsUpdated(Message);
	}
}

void AAsteroidsPawn::FireShot()
{
	Fire(FInputActionValue());
}

void AAsteroidsPawn::Fire(const FInputActionValue&)
{
	// If it's ok to fire again
	if (CurrentBullets < 2)
	{
		const FVector FireDirection = GetActorForwardVector();
		const FRotator FireRotation = FireDirection.Rotation();
		// Spawn projectile at an offset from this pawn
		const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

		UWorld* const World = GetWorld();
		if (!ensureAlways(IsValid(World)))
		{
			return;
		}

		// spawn the projectile
		if (!ensureAlways(IsValid(ProjectileClass)))
		{
			return;
		}

		World->SpawnActor(ProjectileClass, &SpawnLocation, &FireRotation);

		bCanFire = false;

		World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AAsteroidsPawn::ShotTimerExpired, FireRate);

		// try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		bCanFire = false;
		CurrentBullets++;
	}
}

void AAsteroidsPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void AAsteroidsPawn::OnBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if (!ensureAlways(IsValid(OtherComp) && IsValid(OtherComp->GetOwner())))
	{
		return;
	}

	if (OtherComp->GetOwner()->IsA<AAsteroid>())
	{
		bIsOverlappingAsteroid = true;
		DealDamage(10);
		bDamageTimerActive = true;
	}
}

void AAsteroidsPawn::OnEndOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent* OtherComp, int32)
{
	if (!ensureAlways(IsValid(OtherComp) && IsValid(OtherComp->GetOwner())))
	{
		return;
	}

	if (OtherComp->GetOwner()->IsA<AAsteroid>())
	{
		bIsOverlappingAsteroid = false;
	}
}

void AAsteroidsPawn::HandleHealthPackPickedUp(const FMessage Message)
{
	PlayerCurrentHealth += Message.floatMessage;

	if (PlayerCurrentHealth > 100)
	{
		PlayerCurrentHealth = 100;
	}

	FMessage NewHealthMessage = FMessage();
	NewHealthMessage.floatMessage = PlayerCurrentHealth / PlayerMaxHealth;
	Messenger->UpdatePlayerHealth(NewHealthMessage);

	if (PlayerCurrentHealth <= 25.0f)
	{
		if (!ensureAlways(IsValid(SmokeComponent) && IsValid(FireComponent)))
		{
			return;
		}

		SmokeComponent->ActivateSystem();
		FireComponent->DeactivateSystem();
	}
	else if (PlayerCurrentHealth <= 50.0f)
	{
		if (!ensureAlways(IsValid(SmokeComponent)))
		{
			return;
		}

		SmokeComponent->DeactivateSystem();
	}

	
}

