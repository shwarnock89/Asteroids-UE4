// Fill out your copyright notice in the Description page of Project Settings.

#include "HighScoreCalculator.h"
#include "Asteroids/AsteroidsSaveGame.h"
#include "HighScoreStruct.h"
#include "Kismet/GameplayStatics.h"

bool UHighScoreCalculator::IsNewHighScore(const int NewHighScore)
{
	const UAsteroidsSaveGame* SavedGame = Cast<UAsteroidsSaveGame>(UGameplayStatics::LoadGameFromSlot("SaveGame", 0));
	if (!IsValid(SavedGame))
	{
		return true;
	}

	TArray<FHighScore> HighScores = SavedGame->HighScores;
	if (HighScores.Num() < Max_High_Scores)
	{
		return true;
	}

	for (int i = 0; i < Max_High_Scores; ++i)
	{
		if (NewHighScore > HighScores[i].HighScore)
		{
			return true;
		}
	}

	return false;
}

void UHighScoreCalculator::SetNewHighScores(const FHighScore& NewHighScore)
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