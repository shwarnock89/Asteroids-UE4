// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsteroidManager.h"
#include "GameFramework/Actor.h"

#include "Asteroid.generated.h"

UCLASS()
class ASTEROIDS_API AAsteroid : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	AAsteroid();

	ESizes GetSize() const { return Size; }

	void Initialize(const EStartSides InStartSide, const ESizes InSize);

private:

	// Called every frame
	virtual void BeginPlay() override;
	virtual void Tick(const float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	float MoveSpeed = 0.0f;

	ESizes Size = ESizes::None;

	EStartSides StartSide = EStartSides::None;

	FVector MoveDirection = FVector::ZeroVector;

	FVector RotationSpeed = FVector::ZeroVector;

	FRotator Rotation = FRotator::ZeroRotator;
	float Buffer = 0.0f;
};
