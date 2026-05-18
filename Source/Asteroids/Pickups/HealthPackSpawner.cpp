// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthPackSpawner.h"

#include "AsteroidManager.h"
#include "AsteroidSettings.h"
#include "Components/CapsuleComponent.h"
#include "HealthPack.h"
#include "WorldBoundsVolume.h"

bool UHealthPackSpawner::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!ensureAlways(IsValid(World)))
	{
		return false;
	}

	if (World->GetNetMode() == NM_Client)
	{
		return false;
	}

	const EWorldType::Type WorldType = World->WorldType;
	return WorldType != EWorldType::Editor && WorldType != EWorldType::EditorPreview && WorldType != EWorldType::Inactive;
}

void UHealthPackSpawner::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UAsteroidManager::StaticClass());

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	UAsteroidManager* AsteroidManager = GetWorld()->GetSubsystem<UAsteroidManager>();
	if (!ensureAlways(IsValid(AsteroidManager)))
	{
		return;
	}

	AsteroidManager->OnUpdateLevel.AddDynamic(this, &UHealthPackSpawner::HandleLevelChanged);
}

void UHealthPackSpawner::HandleLevelChanged(const int)
{
	if (SpawnHealthPackTimerHandle.IsValid())
	{
		return;
	}

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(SpawnHealthPackTimerHandle, this, &UHealthPackSpawner::SpawnHealthPack, SpawnDelay);
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

	const UWorldBoundsVolumeSubsystem* WorldBoundsVolumeSubsystem = GetWorld()->GetSubsystem<UWorldBoundsVolumeSubsystem>();
	if (!ensureAlways(IsValid(WorldBoundsVolumeSubsystem)))
	{
		return;
	}

	FTransform SpawnTransform(FRotator::ZeroRotator, WorldBoundsVolumeSubsystem->GetValidWorldLocation(), FVector(0.5f));
	AActor* HealthPack = GetWorld()->SpawnActorDeferred<AHealthPack>(AsteroidSettings->HealthPackClass, SpawnTransform);
	if (!ensureAlways(HealthPack) && IsValid(HealthPack->GetClass()) && HealthPack->GetClass()->ImplementsInterface(UWorldBoundsHandlingInterface::StaticClass()))
	{
		return;
	}

	const UCapsuleComponent* CapsuleComponent = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(HealthPack);
	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	const FVector Padding(CapsuleComponent->GetScaledCapsuleRadius(), CapsuleComponent->GetScaledCapsuleRadius(), 0.0f);
	const FVector FinalLocation = WorldBoundsVolumeSubsystem->FindClosestPointToLocation(HealthPack->GetActorLocation(), Padding);
	SpawnTransform.SetLocation(FinalLocation);
	HealthPack->FinishSpawning(SpawnTransform);

	HealthPack->OnDestroyed.AddDynamic(this, &UHealthPackSpawner::HandlePickupDestroyed);
}

void UHealthPackSpawner::HandlePickupDestroyed(AActor* Actor)
{
	CurrentHealthPack = nullptr;
	bHealthPackSpawned = false;

	Actor->OnDestroyed.RemoveDynamic(this, &UHealthPackSpawner::HandlePickupDestroyed);

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	const UAsteroidManager* AsteroidManager = GetWorld()->GetSubsystem<UAsteroidManager>();
	if (!ensureAlways(IsValid(AsteroidManager)))
	{
		return;
	}

	if (AsteroidManager->GetCurrentAsteroidCount() < AsteroidManager->GetSpawnMultiplier())
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(SpawnHealthPackTimerHandle, this, &UHealthPackSpawner::SpawnHealthPack, SpawnDelay);
}
