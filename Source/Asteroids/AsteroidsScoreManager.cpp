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

void UAsteroidsScoreManager::SetNewHighScores(const FHighScore& NewHighScore)
{
	UAsteroidsSaveGame* SavedGame = Cast<UAsteroidsSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveGame", 0));
	if (!IsValid(SavedGame))
	{
		SavedGame = Cast<UAsteroidsSaveGame>(UGameplayStatics::CreateSaveGameObject(UAsteroidsSaveGame::StaticClass()));
		TArray<FHighScore> HighScores;
		HighScores.Add(NewHighScore);
		for (int i = 1; i < Max_High_Scores; ++i)
		{
			HighScores.Add(FHighScore(FText::FromString("AAA"), 0));
		}
		SavedGame->HighScores = HighScores;
		UGameplayStatics::SaveGameToSlot(SavedGame, "SaveGame", 0);
		return;
	}
	TArray<FHighScore> HighScores = SavedGame->HighScores;
	const int ScoreCount = HighScores.Num();
	for (int i = 0; i < ScoreCount; ++i)
	{
		if (NewHighScore.HighScore > HighScores[i].HighScore)
		{
			HighScores.Insert(NewHighScore, i);
			break;
		}
	}

	const int NewScoreCount = HighScores.Num();
	if (NewScoreCount == ScoreCount)
	{
		HighScores.Add(NewHighScore);
	}

	if (NewScoreCount > Max_High_Scores)
	{
		HighScores.RemoveAt(NewScoreCount - 1);
	}

	SavedGame->HighScores = HighScores;

	UGameplayStatics::SaveGameToSlot(SavedGame, "SaveGame", 0);
}