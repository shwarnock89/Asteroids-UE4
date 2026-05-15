// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthPackSpawner.h"

#include "Asteroids/Utils/AsteroidSettings.h"
#include "Asteroids/WorldBoundsVolume.h"
#include "HealthPack.h"

void UHealthPackSpawner::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHealthPackSpawner::Deinitialize()
{
	Super::Deinitialize();
	if (IsValid(CurrentHealthPack))
	{
		CurrentHealthPack->OnDestroyed.RemoveDynamic(this, &UHealthPackSpawner::HandlePickupDestroyed);
	}
}

void UHealthPackSpawner::SpawnHealthPack()
{
	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	const UAsteroidSettings* AsteroidSettings = GetDefault<UAsteroidSettings>();
	if (!ensureAlways(IsValid(AsteroidSettings)))
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, AWorldBoundsVolume::GetValidWorldLocation(), FVector(0.5f));
	AActor* HealthPack = GetWorld()->SpawnActor(AsteroidSettings->HealthPackClass, &SpawnTransform);
	if (!ensureAlways(IsValid(HealthPack)))
	{
		return;
	}

	HealthPack->OnDestroyed.AddDynamic(this, &UHealthPackSpawner::HandlePickupDestroyed);
}

void UHealthPackSpawner::HandlePickupDestroyed(AActor* Actor)
{
	CurrentHealthPack = nullptr;
	bHealthPackSpawned = false;

	Actor->OnDestroyed.RemoveDynamic(this, &UHealthPackSpawner::HandlePickupDestroyed);
}

void UHealthPackSpawner::Tick(const float DeltaTime)
{
	if (bHealthPackSpawned)
	{
		return;
	}

	SpawnTimerHealth += DeltaTime;

	if (SpawnTimerHealth > SpawnDelay)
	{
		SpawnTimerHealth = 0.0f;
		SpawnHealthPack();
	}
}
