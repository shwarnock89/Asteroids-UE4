// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "WorldBoundsVolume.h"

#include "AsteroidsPawn.generated.h"

class UCapsuleComponent;
class AAsteroidsProjectile;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(FireComponentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(SmokeComponentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ExplosionComponentTag);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(ShipComponentTag);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerHealthUpdated, const float, PlayerHealthPercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerShieldUpdated, const float, PlayerCurrentShields);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);

UCLASS(Blueprintable)
class AAsteroidsPawn : public APawn, public IWorldBoundsHandlingInterface
{
	GENERATED_BODY()

public:

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere)
	FVector GunOffset = FVector(90.f, 0.f, 0.f);

	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere)
	float FireRate = 0.1f;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	USoundBase* FireSound;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerHealthUpdated OnPlayerHealthUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerShieldUpdated OnPlayerShieldUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerDied OnPlayerDied;

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void FireShot();

	void HandleHealthPackPickedUp(const float HealthIncreaseAmount);

	// Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(const float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual UCapsuleComponent* GetCapsuleComponent_Implementation() const override { return CapsuleComponent; }
	virtual EHandlingType GetHandlingType_Implementation() const override { return EHandlingType::Flip; }

private:

	bool bIsDead = false;

	float PlayerCurrentHealth = 100.0f;
	float PlayerMaxHealth = 100.0f;

	float PlayerCurrentShields = 100.0f;
	float PlayerMaxShields = 100.0f;
	float ShieldRegenDelay = 6.0f;
	float ShieldRegenTimer = 0.0f;
	bool bShieldTimerActive = false;
	float ShieldRegenRate = 10.0f;

	UPROPERTY(EditDefaultsOnly)
	float MaxSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly)
	int MaxBullets = 2;

	int CurrentBullets = 0;

	FVector MoveSpeed = FVector::ZeroVector;

	FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);

	/* Flag to control firing  */
	bool bCanFire = true;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	bool bDamageTimerActive = false;
	bool bIsProcesingHit = false;
	float DamageTimeDelay = 1.0f;
	float CurrentDamageTimeDelay = 0.0f;

	UPROPERTY(EditDefaultsOnly)
	float RotationSpeed = 100.0f;

	UPROPERTY(EditDefaultsOnly)
	float ThrustStrength = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* FireAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AAsteroidsProjectile> ProjectileClass = nullptr;

	/* The Smoke component*/
	UPROPERTY()
	UParticleSystemComponent* SmokeComponent = nullptr;

	/* The Fire Component */
	UPROPERTY()
	UParticleSystemComponent* FireComponent = nullptr;

	/* The Explosion Component */
	UPROPERTY()
	UParticleSystemComponent* ExplosionComponent = nullptr;

	UPROPERTY()
	UStaticMeshComponent* ShipMeshComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

	FVector2D LastInput = FVector2D::ZeroVector;

	void DealDamage(float Damage);

	void RegenerateShields(const float DeltaSeconds);

	void Fire(const FInputActionValue& Value);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void HandleMovement(const FInputActionValue& Value);

	void HandleShieldDamage(const float DamageAmount);

	void HandleHealthDamage(const float DamageAmount);

	void HandleDeath();

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	UFUNCTION()
	void HandleBulletDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void DestroyPawn();
};