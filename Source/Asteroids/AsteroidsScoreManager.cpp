// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidsScoreManager.h"

#include "AsteroidsSaveGame.h"
#include "Kismet/GameplayStatics.h"

UAsteroidsScoreManager* UAsteroidsScoreManager::GetScoreManager(const UWorld& World)
{
	const UGameInstance* GameInstance = World.GetGameInstance();
	if (!ensureAlways(IsValid(GameInstance)))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UAsteroidsScoreManager>();
}

void UAsteroidsScoreManager::UpdatePlayerScore(const int ScoreUpdateAmount)
{
	PlayerScore += ScoreUpdateAmount;
	OnPlayerScoreUpdated.Broadcast(PlayerScore);
}

bool UAsteroidsScoreManager::IsNewHighScore() const
{
	const UAsteroidsSaveGame* SavedGame = Cast<UAsteroidsSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveGame", 0));
	if (!IsValid(SavedGame))
	{
		return false;
	}

	TArray<FHighScore> HighScores = SavedGame->HighScores;
	if (HighScores.Num() < Max_High_Scores)
	{
		return false;
	}

	for (int i = 0; i < Max_High_Scores; ++i)
	{
		if (PlayerScore > HighScores[i].HighScore)
		{
			return true;
		}
	}

	return false;
}

void UAsteroidsScoreManager::CheckIsHighScore() const
{
	if (IsNewHighScore())
	{
		OnNewHighScore.Broadcast(PlayerScore);
	}
}
