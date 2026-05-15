// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/AsteroidEntitySpawned.h"
#include "WorldBoundsVolume.generated.h"

USTRUCT()
struct FTrackedActor
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* Actor;

	UPROPERTY()
	EHandlingType HandlingType;

	bool operator==(const FTrackedActor& Other) const
	{
		return Actor == Other.Actor && HandlingType == Other.HandlingType;
	}
};

UCLASS()
class ASTEROIDS_API AWorldBoundsVolume : public AActor
{
	GENERATED_BODY()

public:

	AWorldBoundsVolume();

	static bool IsValidWorld();

	static FVector GetValidWorldLocation();

	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere)
	FVector HalfExtents;

private:

	void HandleAsteroidEntitySpawned(AActor* SpawnedActor, const EHandlingType HandlingType);

	UFUNCTION()
	void HandleEntityDestroyed(AActor* DestroyedActor);

	virtual void Tick(const float DeltaTime) override;

	void FlipActorWorldPosition(AActor* Actor) const;

	UPROPERTY()
	TArray<FTrackedActor> TrackedActors;

	FDelegateHandle EntitySpawnedHandle;
};
