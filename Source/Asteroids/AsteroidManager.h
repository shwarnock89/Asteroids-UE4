// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utils/MessageStruct.h"
#include "Utils/Messanger.h"

#include "AsteroidManager.generated.h"

UCLASS()
class ASTEROIDS_API AAsteroidManager : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	AAsteroidManager();

	UFUNCTION()
	void Initialize(const int CurrentLevel);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	const int Screen_Buffer = 40;

	void SpawnLevelInitialAsteroids(const int CurrentLevel);

	int CurrentAsteroidCount = 0;

	void CreateAsteroid(const FVector& StartPos, const EStartSides::START_SIDE StartSide, const ESizes::SIZE Size);

	UFUNCTION()
	void HandleAsteroidDestroyed(FMessage Message);

	int SspawnMultiplier = 0;

	UPROPERTY()
	UMessanger* Messanger = nullptr;
};
