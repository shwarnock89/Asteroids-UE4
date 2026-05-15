
#include "AsteroidsPawn.h"

#include "Asteroid.h"
#include "AsteroidsProjectile.h"
#include "AsteroidsScoreManager.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

UE_DEFINE_GAMEPLAY_TAG(FireComponentTag, "Component.Fire");
UE_DEFINE_GAMEPLAY_TAG(SmokeComponentTag, "Component.Smoke");
UE_DEFINE_GAMEPLAY_TAG(ExplosionComponentTag, "Component.Explosion");
UE_DEFINE_GAMEPLAY_TAG(ShipComponentTag, "Component.Ship");

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
	CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
}

void AAsteroidsPawn::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
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

void AAsteroidsPawn::HandleShieldDamage(const float DamageAmount)
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

void AAsteroidsPawn::HandleDeath()
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

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, FTimerDelegate::CreateUObject(this, &AAsteroidsPawn::DestroyPawn), 1.0f, false);

	bIsDead = true;
}

void AAsteroidsPawn::HandleHealthDamage(const float DamageAmount)
{
	PlayerCurrentHealth -= DamageAmount;

	if (PlayerCurrentHealth <= 0.0f)
	{
		HandleDeath();
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

void AAsteroidsPawn::DealDamage(const float Damage)
{
	if (!bShieldTimerActive)
	{
		HandleShieldDamage(Damage);
		return;
	}

	if (!bDamageTimerActive)
	{
		HandleHealthDamage(Damage);
	}
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

		const float CurrentShieldPercentage = PlayerCurrentShields / PlayerMaxShields;
		OnPlayerShieldUpdated.Broadcast(CurrentShieldPercentage);
	}
}

void AAsteroidsPawn::FireShot()
{
	Fire(FInputActionValue());
}

void AAsteroidsPawn::Fire(const FInputActionValue&)
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

void AAsteroidsPawn::OnHit(UPrimitiveComponent*, AActor*OtherActor, UPrimitiveComponent* OtherComp, FVector, const FHitResult& Hit)
{
	if (bIsProcesingHit)
	{
		return;
	}

	bIsProcesingHit = true;

	if (!ensureAlways(OtherActor))
	{
		return;
	}

	if (OtherComp->GetOwner()->IsA<AAsteroid>())
	{
		DealDamage(10);

		// 1. Calculate Reflection
		FVector ReflectedVelocity = FMath::GetReflectionVector(MoveSpeed, Hit.ImpactNormal);
		ReflectedVelocity.Z = 0.0f;

		// 2. The "Stationary" Check
		// If MoveSpeed was 0, ReflectedVelocity will be 0. We need to override it.
		static constexpr float MinKnockback = 400.0f; // Adjust this for "arcade" feel

		if (ReflectedVelocity.SizeSquared() < FMath::Square(MinKnockback))
		{
			// If we aren't moving fast enough, explode away from the impact normal
			MoveSpeed = Hit.ImpactNormal.GetSafeNormal2D() * MinKnockback;
		}
		else
		{
			// Otherwise, use the bounced velocity
			MoveSpeed = ReflectedVelocity * 0.8f;
		}

		// 3. Position Correction (Crucial)
		// IMPORTANT: Set bSweep to FALSE here.
		// If true, the correction itself might be blocked by the asteroid you just hit.
		FVector Correction = Hit.ImpactNormal;
		Correction.Z = 0.0f;
		AddActorWorldOffset(Correction * 5.0f, false);

		// 3. The "Safety Net" Check
		// Now we manually check if our new position is overlapping something else
		TArray<AActor*> OverlappingActors;
		GetOverlappingActors(OverlappingActors, AAsteroid::StaticClass());

		if (OverlappingActors.Num() > 0)
		{
			// If we landed inside ANOTHER asteroid, handle it here.
			// Option A: Just deal damage again.
			// Option B: Nudge again in the new direction.
			DealDamage(10);
		}
	}

	bIsProcesingHit = false;
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

