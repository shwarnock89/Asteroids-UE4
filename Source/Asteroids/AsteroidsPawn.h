// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

#include "AsteroidsPawn.generated.h"

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
class AAsteroidsPawn : public APawn
{
	GENERATED_BODY()

public:
	AAsteroidsPawn();

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere)
	FVector GunOffset;

	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere)
	float FireRate;

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

private:

	bool bIsDead;

	float PlayerCurrentHealth;
	float PlayerMaxHealth;

	float PlayerCurrentShields;
	float PlayerMaxShields;
	float ShieldRegenDelay;
	float ShieldRegenTimer;
	bool bShieldTimerActive;
	float ShieldRegenRate;

	UPROPERTY(EditDefaultsOnly)
	float MaxSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly)
	int MaxBullets = 2;

	int CurrentBullets = 0;

	bool bIsOverlappingAsteroid = false;

	FVector MoveSpeed = FVector(0.0f, 0.0f, 0.0f);

	FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);

	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	bool bDamageTimerActive;
	float DamageTimeDelay;
	float CurrentDamageTimeDelay;

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

	UPROPERTY(EditDefaultsOnly)
	float SpeedFactor = 10.0f;

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

	FVector2D LastInput = FVector2D::ZeroVector;

	void DealDamage(float Damage);

	void RegenerateShields(const float DeltaSeconds);

	void Fire(const FInputActionValue& Value);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HandleMovement(const FInputActionValue& Value);

	/* Handler for the fire timer expiry */
	void ShotTimerExpired();

	void HandleBulletDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void DestroyPawn();
};