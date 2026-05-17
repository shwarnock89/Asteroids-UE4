
#include "AsteroidsPawn.h"

#include "Asteroid.h"
#include "AsteroidsMovementComponent.h"
#include "AsteroidsProjectile.h"
#include "AsteroidsScoreManager.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"

UE_DEFINE_GAMEPLAY_TAG(FireComponentTag, "Component.Fire");
UE_DEFINE_GAMEPLAY_TAG(SmokeComponentTag, "Component.Smoke");
UE_DEFINE_GAMEPLAY_TAG(ExplosionComponentTag, "Component.Explosion");
UE_DEFINE_GAMEPLAY_TAG(ShipComponentTag, "Component.Ship");

AAsteroidsPawn::AAsteroidsPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	NetDormancy = DORM_Never;
	SetNetUpdateFrequency(60.0f);
	SetMinNetUpdateFrequency(30.0f);

	MovementComponent = CreateDefaultSubobject<UAsteroidsMovementComponent>(TEXT("AsteroidsMovementComponent"));
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
	const FVector2D Input = Value.Get<FVector2D>();

	MovementComponent->SetInputVector(Input);

	Server_SetInput(Input);
}

void AAsteroidsPawn::Server_SetInput_Implementation(const FVector2D& Input)
{
	MovementComponent->SetInputVector(Input);
}

void AAsteroidsPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FireComponent = FindComponentByTag<UParticleSystemComponent>(FireComponentTag.GetTag().GetTagName());
	ExplosionComponent = FindComponentByTag<UParticleSystemComponent>(ExplosionComponentTag.GetTag().GetTagName());
	SmokeComponent = FindComponentByTag<UParticleSystemComponent>(SmokeComponentTag.GetTag().GetTagName());
	ShipMeshComponent = FindComponentByTag<UStaticMeshComponent>(ShipComponentTag.GetTag().GetTagName());
	CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
}

void AAsteroidsPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		static const TArray Palette =
		{
			FLinearColor::Red,
			FLinearColor::Green,
			FLinearColor::Blue,
			FLinearColor::Yellow,
		};

		static int CurrentIndex = 0;
		PlayerColor = Palette[CurrentIndex % Palette.Num()];
		++CurrentIndex;
		SetPlayerColor();
	}
}

void AAsteroidsPawn::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	if (HasAuthority())
	{
		SetReplicateMovement(false);
	}

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AAsteroidsPawn::OnHit);

	FVector WorldLocation = GetActorLocation();
	WorldLocation.Z = 0.0f;
	SetActorLocation(WorldLocation);
}

void AAsteroidsPawn::HandleBulletDestroyed(AActor*)
{
	--CurrentBullets;
}

void AAsteroidsPawn::DestroyPawn()
{
	Destroy();
}

void AAsteroidsPawn::Server_HandleShieldDamage_Implementation(const float DamageAmount)
{
	if (PlayerCurrentShields > 0.0f)
	{
		ShieldRegenTimer = 0.0f;
		bShieldTimerActive = true;
		PlayerCurrentShields -= DamageAmount;

		const float CurrentShieldPercentage = PlayerCurrentShields / PlayerMaxShields;
		OnPlayerShieldUpdated.Broadcast(CurrentShieldPercentage);
	}
}

void AAsteroidsPawn::Server_HandleDeath_Implementation()
{
	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	const UAsteroidsScoreManager* ScoreManager = UAsteroidsScoreManager::GetScoreManager(*GetWorld());
	if (!ensureAlways(IsValid(ScoreManager)))
	{
		return;
	}

	if (!bIsDead)
	{
		ScoreManager->CheckIsHighScore();
		OnPlayerDied.Broadcast();
	}

	if (!ensureAlways(IsValid(SmokeComponent) && IsValid(FireComponent)))
	{
		return;
	}

	SmokeComponent->SetHiddenInGame(true);
	FireComponent->SetHiddenInGame(true);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!ensureAlways(IsValid(PlayerController)))
	{
		return;
	}

	DisableInput(PlayerController);

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

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateUObject(this, &AAsteroidsPawn::DestroyPawn), 1.0f, false);

	bIsDead = true;
}

void AAsteroidsPawn::Server_HandleHealthDamage_Implementation(const float DamageAmount)
{
	PlayerCurrentHealth -= DamageAmount;

	if (PlayerCurrentHealth <= 0.0f)
	{
		Server_HandleDeath();
	}
	else
	{
		ShieldRegenTimer = 0.0f;
		bDamageTimerActive = true;
		const float CurrentHealthPercentage = PlayerCurrentHealth / PlayerMaxHealth;
		OnPlayerHealthUpdated.Broadcast(CurrentHealthPercentage);
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

void AAsteroidsPawn::Server_DealDamage_Implementation(const float Damage)
{
	if (!bShieldTimerActive)
	{
		Server_HandleShieldDamage(Damage);
		return;
	}

	if (!bDamageTimerActive)
	{
		Server_HandleHealthDamage(Damage);
	}
}

void AAsteroidsPawn::Tick(const float DeltaSeconds)
{
	if (HasAuthority())
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
			Server_RegenerateShields(DeltaSeconds);
		}

		if (ShieldRegenTimer >= ShieldRegenDelay)
		{
			ShieldRegenTimer = 0.0f;
			bShieldTimerActive = false;
		}
	}
}

void AAsteroidsPawn::Server_RegenerateShields_Implementation(const float DeltaSeconds)
{
	if (PlayerCurrentShields < PlayerMaxShields)
	{
		PlayerCurrentShields += ShieldRegenRate * DeltaSeconds;
		if (PlayerCurrentShields > 100.0f)
		{
			PlayerCurrentShields = 100.0f;
		}

		const float CurrentShieldPercentage = PlayerCurrentShields / PlayerMaxShields;
		OnPlayerShieldUpdated.Broadcast(CurrentShieldPercentage);
	}
}

void AAsteroidsPawn::FireShot()
{
	Server_Fire(FInputActionValue());
}

void AAsteroidsPawn::Server_Fire_Implementation(const FInputActionValue&)
{
	if (!bCanFire)
	{
		return;
	}

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

		AActor* SpawnedActor = World->SpawnActor(ProjectileClass, &SpawnLocation, &FireRotation);
		if (!ensureAlways(IsValid(SpawnedActor)))
		{
			return;
		}

		SpawnedActor->OnDestroyed.AddDynamic(this, &AAsteroidsPawn::HandleBulletDestroyed);

		World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AAsteroidsPawn::Server_ShotTimerExpired, FireRate);

		// try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		bCanFire = false;
		CurrentBullets++;
	}
}

void AAsteroidsPawn::Server_ShotTimerExpired_Implementation()
{
	bCanFire = true;
}

void AAsteroidsPawn::OnHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult&)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsProcessingHit)
	{
		return;
	}

	bIsProcessingHit = true;

	if (!ensureAlways(OtherActor))
	{
		return;
	}

	if (OtherActor->IsA<AAsteroid>())
	{
		Server_DealDamage(10);
	}

	bIsProcessingHit = false;
}

void AAsteroidsPawn::HandleHealthPackPickedUp(const float HealthIncreaseAmount)
{
	PlayerCurrentHealth += HealthIncreaseAmount;

	if (PlayerCurrentHealth > 100)
	{
		PlayerCurrentHealth = 100;
	}

	const float PlayerHealthPercentage = PlayerCurrentHealth / PlayerMaxHealth;
	OnPlayerHealthUpdated.Broadcast(PlayerHealthPercentage);

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

void AAsteroidsPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteroidsPawn, bIsDead);
	DOREPLIFETIME(AAsteroidsPawn, PlayerCurrentHealth);
	DOREPLIFETIME(AAsteroidsPawn, PlayerCurrentShields);
	DOREPLIFETIME(AAsteroidsPawn, ShieldRegenTimer);
	DOREPLIFETIME(AAsteroidsPawn, bShieldTimerActive);
	DOREPLIFETIME(AAsteroidsPawn, CurrentBullets);
	DOREPLIFETIME(AAsteroidsPawn, bCanFire);
	DOREPLIFETIME(AAsteroidsPawn, TimerHandle_ShotTimerExpired);
	DOREPLIFETIME(AAsteroidsPawn, bDamageTimerActive);
	DOREPLIFETIME(AAsteroidsPawn, bIsProcessingHit);
	DOREPLIFETIME(AAsteroidsPawn, CurrentDamageTimeDelay);
	DOREPLIFETIME(AAsteroidsPawn, PlayerColor);
}

void AAsteroidsPawn::OnRep_PlayerColor() const
{
	SetPlayerColor();
}

void AAsteroidsPawn::SetPlayerColor() const
{
	if (!ensureAlways(IsValid(ShipMeshComponent)))
	{
		return;
	}

	UMaterialInstanceDynamic* MaterialInstanceDynamic = ShipMeshComponent->CreateAndSetMaterialInstanceDynamic(0);
	if (!ensureAlways(IsValid(MaterialInstanceDynamic)))
	{
		return;
	}

	MaterialInstanceDynamic->SetVectorParameterValue("DiffuseColor", PlayerColor);
}
