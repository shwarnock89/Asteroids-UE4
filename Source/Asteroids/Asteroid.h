// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsteroidManager.h"
#include "Interfaces/HitReactInterface.h"
#include "WorldBoundsVolume.h"

#include "Asteroid.generated.h"

class UAsteroidsMovementComponent;
class UCapsuleComponent;

UCLASS()
class ASTEROIDS_API AAsteroid : public AActor, public IWorldBoundsHandlingInterface, public IHitReactInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	AAsteroid();

	ESizes GetSize() const { return Size; }

	UFUNCTION(Server, Reliable, WithValidation)
	void Initialize(const EStartSides InStartSide, const ESizes InSize);

private:

	virtual UCapsuleComponent* GetCapsuleComponent_Implementation() const override { return CapsuleComponent; }
	virtual EHandlingType GetHandlingType_Implementation() const override { return EHandlingType::Flip; }

	// Called every frame
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	ESizes Size = ESizes::None;

	EStartSides StartSide = EStartSides::None;

	FRotator Rotation = FRotator::ZeroRotator;

	FVector RotationSpeed = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly)
	FFloatRange SpeedRange = FFloatRange();

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

	// This component automatically handles all multiplayer position smoothing natively
	UPROPERTY(VisibleAnywhere, Category = "Movement", meta = (AllowPrivateAccess))
	TObjectPtr<UAsteroidsMovementComponent> ProjectileMovement = nullptr;

	bool bIsPendingDestroy = false;
	bool bIsProcessingHit = false;
};
