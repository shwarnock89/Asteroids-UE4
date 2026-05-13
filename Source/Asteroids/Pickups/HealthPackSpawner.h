// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HealthPackSpawner.generated.h"

class AHealthPack;
/**
 * 
 */
UCLASS()
class ASTEROIDS_API UHealthPackSpawner : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(const float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UHealthPackSpawner, STATGROUP_Tickables); }

	void SpawnHealthPack();

	UFUNCTION()
	void HandlePickupDestroyed(AActor* Actor);

	UPROPERTY()
	TObjectPtr<AHealthPack> CurrentHealthPack;

	float SpawnTimerHealth = 0.0f;

	float SpawnDelay = 5.0f;

	bool bHealthPackSpawned = false;
};
