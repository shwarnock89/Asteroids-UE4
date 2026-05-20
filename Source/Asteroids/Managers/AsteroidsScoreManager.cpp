// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidsScoreManager.h"

#include "AsteroidsGameState.h"
#include "AsteroidsPlayerState.h"
#include "AsteroidsSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
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

void UAsteroidsScoreManager::UpdatePlayerScore(const int ScoreUpdateAmount, const APawn& Pawn) const
{
	const UWorld* World = GetWorld();
	if (!ensureAlways(IsValid(World)))
	{
		return;
	}

	if (!World->GetAuthGameMode())
	{
		return;
	}

	AAsteroidsGameState* AsteroidsGameState = Cast<AAsteroidsGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!ensureAlways(IsValid(AsteroidsGameState)))
	{
		return;
	}

	AsteroidsGameState->AddToTeamScore(ScoreUpdateAmount);

	AAsteroidsPlayerState* PlayerState = Cast<AAsteroidsPlayerState>(Pawn.GetPlayerState());
	if (!ensureAlways(IsValid(PlayerState)))
	{
		return;
	}

	PlayerState->AddToScore(ScoreUpdateAmount);
}

bool UAsteroidsScoreManager::IsNewHighScore(const APlayerState* PlayerState) const
{
	if (!ensureAlways(IsValid(PlayerState)))
	{
		return false;
	}

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

	const int PlayerScore = static_cast<int>(PlayerState->GetScore());
	for (int i = 0; i < Max_High_Scores; ++i)
	{
		if (PlayerScore > HighScores[i].HighScore)
		{
			return true;
		}
	}

	return false;
}

void UAsteroidsScoreManager::CheckIsHighScore(const APlayerState& PlayerState) const
{
	if (IsNewHighScore(&PlayerState))
	{
		if (!ensureAlways(IsValid(PlayerState.GetPawn())))
		{
			return;
		}

		const AAsteroidsPlayerState* AsteroidsPlayerState = Cast<AAsteroidsPlayerState>(&PlayerState);
		if (!ensureAlways(IsValid(AsteroidsPlayerState)))
		{
			return;
		}

		AsteroidsPlayerState->OnNewHighScore.Broadcast(FHighScoreEventData(*PlayerState.GetPawn(), PlayerState.GetScore()));
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

int UAsteroidsScoreManager::GetCurrentScore(const APlayerState* PlayerState) const
{
	const AAsteroidsGameState* AsteroidsGameState = Cast<AAsteroidsGameState>(UGameplayStatics::GetGameState(PlayerState));
	if (!ensureAlways(IsValid(AsteroidsGameState)))
	{
		return -1;
	}

	switch (AsteroidsGameState->GetGameModeType())
	{
		case EGameModeType::CoOp:
		{
			return AsteroidsGameState->GetTeamScore();
		}

		case EGameModeType::Competitive:
		{
			if (!ensureAlways(IsValid(PlayerState)))
			{
				return -1;
			}

			return PlayerState->GetScore();
		}

		default:
			checkNoEntry();
			return -1;
	}
}
