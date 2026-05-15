// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsteroidManager.h"
#include "GameFramework/Actor.h"
#include "WorldBoundsVolume.h"

#include "Asteroid.generated.h"

class UCapsuleComponent;

UCLASS()
class ASTEROIDS_API AAsteroid : public AActor, public IWorldBoundsHandlingInterface
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	AAsteroid();

	ESizes GetSize() const { return Size; }

	void Initialize(const EStartSides InStartSide, const ESizes InSize);

	virtual UCapsuleComponent* GetCapsuleComponent_Implementation() const override { return CapsuleComponent; }
	virtual EHandlingType GetHandlingType_Implementation() const override { return EHandlingType::Flip; }

private:

	// Called every frame
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(const float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	float MoveSpeed = 0.0f;

	ESizes Size = ESizes::None;

	EStartSides StartSide = EStartSides::None;

	FVector MoveDirection = FVector::ZeroVector;

	FVector RotationSpeed = FVector::ZeroVector;

	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent = nullptr;

	bool bIsPendingDestroy = false;
};
