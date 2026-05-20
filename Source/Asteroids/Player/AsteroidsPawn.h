// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "WorldBoundsVolume.h"

#include "AsteroidsPawn.generated.h"

class UAsteroidsHealthComponent;
class UAsteroidsPawnMovementComponent;
class UCapsuleComponent;
class AAsteroidsProjectile;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UE_DECLARE_GAMEPLAY_TAG_EXTERN(ShipComponentTag);

UCLASS(Blueprintable)
class AAsteroidsPawn : public APawn, public IWorldBoundsHandlingInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	UAsteroidsHealthComponent* GetHealthComponent() const { return HealthComponent;}

private:

	AAsteroidsPawn(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Fire")
	void FireShot();

	// Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual FVector GetVelocity() const override;

	virtual UCapsuleComponent* GetCapsuleComponent_Implementation() const override { return CapsuleComponent; }
	virtual EHandlingType GetHandlingType_Implementation() const override { return EHandlingType::Flip; }

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Weapons, EditDefaultsOnly)
	FVector GunOffset = FVector(90.f, 0.f, 0.f);

	/* How fast the weapon will fire */
	UPROPERTY(Category = Weapons, EditDefaultsOnly)
	float FireRate = 0.1f;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditDefaultsOnly)
	USoundBase* FireSound;

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

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FireAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<AAsteroidsProjectile> ProjectileClass = nullptr;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ShipMeshComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAsteroidsPawnMovementComponent> MovementComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAsteroidsHealthComponent> HealthComponent = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerColor)
	FLinearColor PlayerColor = FLinearColor();

	UFUNCTION()
	void OnRep_PlayerColor() const;

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FInputActionValue& Value);

	void HandleMovement(const FInputActionValue& Value);

	/* Handler for the fire timer expiry */
	UFUNCTION(Server, Reliable)
	void Server_ShotTimerExpired();

	UFUNCTION()
	void HandleBulletDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void DestroyPawn();

	UFUNCTION(Server, Unreliable)
	void Server_SetInput(const FVector2D& Input);

	void SetPlayerColor() const;

	UFUNCTION()
	void HandlePlayerDeath();
};