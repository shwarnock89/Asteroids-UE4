// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidManager.h"
#include "Asteroid.h"
#include "AsteroidsGameInstance.h"
#include "Utils/AsteroidSettings.h"
#include "Utils/ScreenUtil.h"
#include "WorldBoundsVolume.h"

// Sets default values
AAsteroidManager::AAsteroidManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrentAsteroidCount = 0;
	SspawnMultiplier = 0;
}

void AAsteroidManager::HandleAsteroidDestroyed(FMessage Message)
{
	const FVector AsteroidCurrentPos = Message.currentPosMessage;
	ESizes::SIZE NewAsteroidSize;

	FMessage ScoreMessage = FMessage();
	switch (Message.asteroidSizeMessage)
	{
		case ESizes::Large:
			ScoreMessage.intMessage = 20 * SspawnMultiplier;
			Messanger->UpdatePlayerScore(ScoreMessage);
			NewAsteroidSize = ESizes::Medium;
			for (int k = 0; k < SspawnMultiplier; ++k)
			{
				CreateAsteroid(AsteroidCurrentPos, EStartSides::None, NewAsteroidSize);
			}
			break;
		case ESizes::Medium:
			ScoreMessage.intMessage = 15 * SspawnMultiplier;
			Messanger->UpdatePlayerScore(ScoreMessage);
			NewAsteroidSize = ESizes::Small;
			for (int k = 0; k < SspawnMultiplier; ++k)
			{
				CreateAsteroid(AsteroidCurrentPos, EStartSides::None, NewAsteroidSize);
			}
			break;
		case ESizes::Small:
			ScoreMessage.intMessage = 10 * SspawnMultiplier;
			Messanger->UpdatePlayerScore(ScoreMessage);
			break;
		default:
			checkNoEntry();
	}

	CurrentAsteroidCount -= Message.intMessage;

	if (CurrentAsteroidCount == 0)
	{
		SpawnLevelInitialAsteroids(SspawnMultiplier + 1);
	}
}

void AAsteroidManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AAsteroidManager::Initialize(const int CurrentLevel)
{
	const UAsteroidsGameInstance* GameInstance = static_cast<UAsteroidsGameInstance*>(GetWorld()->GetGameInstance());
	Messanger = GameInstance->GetMessanger();
	Messanger->OnAsteroidDestroyed.AddDynamic(this, &AAsteroidManager::HandleAsteroidDestroyed);
	SpawnLevelInitialAsteroids(CurrentLevel);
}

void AAsteroidManager::SpawnLevelInitialAsteroids(const int CurrentLevel)
{
	SspawnMultiplier = CurrentLevel;
	for (int i = 0; i < SspawnMultiplier; ++i)
	{
		const EStartSides::START_SIDE StartSide = static_cast<EStartSides::START_SIDE>(rand() % 4);

		const FVector StartPos = AWorldBoundsVolume::GetValidWorldLocation();
		CreateAsteroid(StartPos, StartSide, ESizes::Large);
	}

	FMessage Message = FMessage();
	Message.intMessage = SspawnMultiplier;
	Messanger->UpdateLevel(Message);
}

void AAsteroidManager::CreateAsteroid(const FVector& StartPos, const EStartSides::START_SIDE StartSide, const ESizes::SIZE Size)
{
	const FRotator Rotation(0.0f, 0.0f, 0.0f);
	const FActorSpawnParameters SpawnInfo;

	const UAsteroidSettings* AsteroidSettings = GetDefault<UAsteroidSettings>();
	if (!ensureAlways(IsValid(AsteroidSettings)))
	{
		return;
	}

	AAsteroid* Asteroid = Cast<AAsteroid>(GetWorld()->SpawnActor(AsteroidSettings->AsteroidBaseClass, &StartPos, &Rotation, SpawnInfo));
	if (!ensureAlways(IsValid(Asteroid)))
	{
		return;
	}

	Asteroid->Initialize(StartSide, Size);
	CurrentAsteroidCount++;
}

