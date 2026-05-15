// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"

#include "WorldBoundsVolume.generated.h"

class AWorldBoundsVolume;
class UCapsuleComponent;

UENUM()
enum class EHandlingType : uint8
{
	Despawn,
	Flip
};

UINTERFACE()
class UWorldBoundsHandlingInterface : public UInterface
{
	GENERATED_BODY()
};

class IWorldBoundsHandlingInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	UCapsuleComponent* GetCapsuleComponent() const;

	UFUNCTION(BlueprintNativeEvent)
	EHandlingType GetHandlingType() const;
};

DECLARE_DELEGATE_OneParam(FOnWorldBoundsVolumeSpawned, AWorldBoundsVolume*);

UCLASS()
class UWorldBoundsVolumeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	FVector GetValidWorldLocation(const FVector& OptionalPadding = FVector::ZeroVector) const;

	FVector FindClosestPointToLocation(const FVector& Location, const FVector& Padding) const;

	void SetWorldBoundsVolume(AWorldBoundsVolume& InWorldBoundsVolume);

	FOnWorldBoundsVolumeSpawned OnWorldBoundsVolumeSpawned;

private:

	UPROPERTY()
	TObjectPtr<AWorldBoundsVolume> WorldBoundsVolume;
};

UCLASS()
class ASTEROIDS_API AWorldBoundsVolume : public ATriggerBox
{
	GENERATED_BODY()

public:

	AWorldBoundsVolume();

	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void FlipActorWorldPosition(AActor& Actor) const;

	FDelegateHandle EntitySpawnedHandle;
};
