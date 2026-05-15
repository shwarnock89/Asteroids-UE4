// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidManager.h"

#include "Asteroid.h"
#include "AsteroidsScoreManager.h"
#include "Components/CapsuleComponent.h"
#include "Utils/AsteroidSettings.h"
#include "WorldBoundsVolume.h"

// Sets default values
UAsteroidManager::UAsteroidManager()
{
	CurrentAsteroidCount = 0;
	SpawnMultiplier = 0;
}

void UAsteroidManager::HandleAsteroidDestroyed(AActor* DestroyedActor)
{
	const AAsteroid* Asteroid = Cast<AAsteroid>(DestroyedActor);
	const FVector AsteroidCurrentPos = Asteroid->GetActorLocation();
	ESizes NewAsteroidSize = ESizes::None;
	float ScoreIncreaseAmount = 0;
	bool bShouldSpawnNewAsteroids = true;
	switch (Asteroid->GetSize())
	{
		case ESizes::Large:
			ScoreIncreaseAmount = 20 * SpawnMultiplier;
			NewAsteroidSize = ESizes::Medium;
			break;
		case ESizes::Medium:
			ScoreIncreaseAmount = 15 * SpawnMultiplier;
			NewAsteroidSize = ESizes::Small;
			break;
		case ESizes::Small:
			ScoreIncreaseAmount = 10 * SpawnMultiplier;
			bShouldSpawnNewAsteroids = false;
			break;
		default:
			checkNoEntry();
	}

	--CurrentAsteroidCount;

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	UAsteroidsScoreManager* ScoreManager = UAsteroidsScoreManager::GetScoreManager(*GetWorld());
	if (!ensureAlways(IsValid(ScoreManager)))
	{
		return;
	}

	ScoreManager->UpdatePlayerScore(ScoreIncreaseAmount);

	if (bShouldSpawnNewAsteroids)
	{
		const UWorldBoundsVolumeSubsystem* WorldBoundsVolumeSubsystem = GetWorld()->GetSubsystem<UWorldBoundsVolumeSubsystem>();
		if (!ensureAlways(IsValid(WorldBoundsVolumeSubsystem)))
		{
			return;
		}

		for (int k = 0; k < SpawnMultiplier; ++k)
		{
			AAsteroid* NewAsteroid = CreateAsteroid(AsteroidCurrentPos, EStartSides::None, NewAsteroidSize);
			if (!ensureAlways(IsValid(NewAsteroid) && IsValid(NewAsteroid->GetClass()) && NewAsteroid->GetClass()->ImplementsInterface(UWorldBoundsHandlingInterface::StaticClass())))
			{
				return;
			}

			const UCapsuleComponent* CapsuleComponent = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(NewAsteroid);
			if (!ensureAlways(IsValid(CapsuleComponent)))
			{
				return;
			}

			const FVector Padding(CapsuleComponent->GetScaledCapsuleRadius(), CapsuleComponent->GetScaledCapsuleRadius(), 0.0f);
			const FVector LocationInVolume = WorldBoundsVolumeSubsystem->FindClosestPointToLocation(NewAsteroid->GetActorLocation(), Padding);
			NewAsteroid->SetActorLocation(LocationInVolume);
		}
	}

	if (CurrentAsteroidCount == 0)
	{
		SpawnLevelInitialAsteroids(SpawnMultiplier + 1);
	}
}

bool UAsteroidManager::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!ensureAlways(IsValid(World)))
	{
		return false;
	}

	return World->WorldType != EWorldType::Editor && World->WorldType != EWorldType::Inactive && World->WorldType != EWorldType::EditorPreview;
}

void UAsteroidManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UWorldBoundsVolumeSubsystem::StaticClass());

	UWorldBoundsVolumeSubsystem* WorldBoundsVolumeSubsystem = GetWorld()->GetSubsystem<UWorldBoundsVolumeSubsystem>();
	if (!ensureAlways(IsValid(WorldBoundsVolumeSubsystem)))
	{
		return;
	}

	WorldBoundsVolumeSubsystem->OnWorldBoundsVolumeSpawned.BindLambda([this] (AWorldBoundsVolume* WorldBoundsVolume)
	{
		SpawnLevelInitialAsteroids(1);
	});
}

void UAsteroidManager::SpawnLevelInitialAsteroids(const int CurrentLevel)
{
	SpawnMultiplier = CurrentLevel;

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	const UWorldBoundsVolumeSubsystem* WorldBoundsVolumeSubsystem = GetWorld()->GetSubsystem<UWorldBoundsVolumeSubsystem>();
	if (!ensureAlways(IsValid(WorldBoundsVolumeSubsystem)))
	{
		return;
	}

	for (int i = 0; i < SpawnMultiplier; ++i)
	{
		const EStartSides StartSide = static_cast<EStartSides>(rand() % 4);

		const FVector StartPos = WorldBoundsVolumeSubsystem->GetValidWorldLocation();
		CreateAsteroid(StartPos, StartSide, ESizes::Large);
	}

	OnUpdateLevel.Broadcast(SpawnMultiplier);
}

AAsteroid* UAsteroidManager::CreateAsteroid(const FVector& StartPos, const EStartSides StartSide, const ESizes Size)
{
	const FRotator Rotation(0.0f, 0.0f, 0.0f);
	const FActorSpawnParameters SpawnInfo;

	const UAsteroidSettings* AsteroidSettings = GetDefault<UAsteroidSettings>();
	if (!ensureAlways(IsValid(AsteroidSettings)))
	{
		return nullptr;
	}

	AAsteroid* Asteroid = Cast<AAsteroid>(GetWorld()->SpawnActor(AsteroidSettings->AsteroidBaseClass, &StartPos, &Rotation, SpawnInfo));
	if (!ensureAlways(IsValid(Asteroid)))
	{
		return nullptr;
	}

	Asteroid->Initialize(StartSide, Size);
	Asteroid->OnDestroyed.AddDynamic(this, &UAsteroidManager::HandleAsteroidDestroyed);
	CurrentAsteroidCount++;
	return Asteroid;
}

