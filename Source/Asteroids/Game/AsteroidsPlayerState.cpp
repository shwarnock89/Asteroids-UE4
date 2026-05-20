// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidsPlayerState.h"

#include "AsteroidsScoreManager.h"

void AAsteroidsPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AAsteroidsPlayerState::OnRep_Score()
{
	const UWorld* World = GetWorld();
	if (!ensureAlways(IsValid(World)))
	{
		return;
	}

	const UAsteroidsScoreManager* ScoreManager = UAsteroidsScoreManager::GetScoreManager(*World);
	if (!ensureAlways(IsValid(ScoreManager)))
	{
		return;
	}

	const int TotalScore = ScoreManager->GetCurrentScore(this);
	OnPlayerScoreUpdated.Broadcast(TotalScore);
}

void AAsteroidsPlayerState::AddToScore(const int ScoreIncrease)
{
	if (!HasAuthority())
	{
		return;
	}

	SetScore(GetScore() + ScoreIncrease);

	if (GetPlayerController()->IsLocalController())
	{
		if (!ensureAlways(IsValid(GetWorld())))
		{
			return;
		}

		const UAsteroidsScoreManager* ScoreManager = UAsteroidsScoreManager::GetScoreManager(*GetWorld());
		if (!ensureAlways(IsValid(ScoreManager)))
		{
			return;
		}

		OnPlayerScoreUpdated.Broadcast(ScoreManager->GetCurrentScore(this));
	}
}
