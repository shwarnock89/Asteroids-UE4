// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "WorldBoundsVolume.h"

#include "AsteroidsPawn.generated.h"

class UAsteroidsMovementComponent;
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

	AAsteroidsPawn(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void FireShot();

	void HandleHealthPackPickedUp(const float HealthIncreaseAmount);

	// Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(const float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UCapsuleComponent* GetCapsuleComponent_Implementation() const override { return CapsuleComponent; }
	virtual EHandlingType GetHandlingType_Implementation() const override { return EHandlingType::Flip; }

private:

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Weapons, EditDefaultsOnly)
	FVector GunOffset = FVector(90.f, 0.f, 0.f);

	/* How fast the weapon will fire */
	UPROPERTY(Category = Weapons, EditDefaultsOnly)
	float FireRate = 0.1f;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditDefaultsOnly)
	USoundBase* FireSound;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerHealthUpdated OnPlayerHealthUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerShieldUpdated OnPlayerShieldUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnPlayerDied OnPlayerDied;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float PlayerMaxHealth = 100.0f;

	UPROPERTY(Replicated)
	bool bIsDead = false;

	UPROPERTY(Replicated)
	float PlayerCurrentHealth = 100.0f;

	UPROPERTY(Replicated)
	float PlayerCurrentShields = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float PlayerMaxShields = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float ShieldRegenDelay = 6.0f;

	UPROPERTY(Replicated)
	float ShieldRegenTimer = 0.0f;

	UPROPERTY(Replicated)
	bool bShieldTimerActive = false;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float ShieldRegenRate = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	int MaxBullets = 2;

	UPROPERTY(Replicated)
	int CurrentBullets = 0;

	/* Flag to control firing  */
	UPROPERTY(Replicated)
	bool bCanFire = true;

	/** Handle for efficient management of ShotTimerExpired timer */
	UPROPERTY(Replicated)
	FTimerHandle TimerHandle_ShotTimerExpired;

	UPROPERTY(Replicated)
	bool bDamageTimerActive = false;

	UPROPERTY(Replicated)
	bool bIsProcessingHit = false;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float DamageTimeDelay = 1.0f;

	UPROPERTY(Replicated)
	float CurrentDamageTimeDelay = 0.0f;

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
	// Our master space velocity that the server controls and replicates to everyone

	UPROPERTY()
	UCapsuleComponent* CapsuleComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	UAsteroidsMovementComponent* MovementComponent = nullptr;

	UFUNCTION(Server, Reliable)
	void Server_DealDamage(float Damage);

	UFUNCTION(Server, Reliable)
	void Server_RegenerateShields(const float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FInputActionValue& Value);

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void HandleMovement(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_HandleShieldDamage(const float DamageAmount);

	UFUNCTION(Server, Reliable)
	void Server_HandleHealthDamage(const float DamageAmount);

	UFUNCTION(Server, Reliable)
	void Server_HandleDeath();

	/* Handler for the fire timer expiry */
	UFUNCTION(Server, Reliable)
	void Server_ShotTimerExpired();

	UFUNCTION()
	void HandleBulletDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void DestroyPawn();

	UFUNCTION(Server, Unreliable)
	void Server_SetInput(FVector2D Input);
};