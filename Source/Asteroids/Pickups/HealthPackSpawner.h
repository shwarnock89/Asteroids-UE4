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
class ASTEROIDS_API UHealthPackSpawner : public UWorldSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	void SpawnHealthPack();

	UFUNCTION()
	void HandleLevelChanged(const int CurrentLevel);

	UFUNCTION()
	void HandlePickupDestroyed(AActor* Actor);

	UPROPERTY()
	TObjectPtr<AHealthPack> CurrentHealthPack;

	FTimerHandle SpawnHealthPackTimerHandle;

	float SpawnTimerHealth = 0.0f;

	float SpawnDelay = 5.0f;

	bool bHealthPackSpawned = false;
};
