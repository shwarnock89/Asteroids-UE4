// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "AsteroidsScoreManager.generated.h"

struct FHighScore;

USTRUCT(BlueprintType)
struct FHighScoreEventData
{
	GENERATED_BODY()

	FHighScoreEventData(APawn& InPawn, const int InNewHighScore)
		: Player(&InPawn), NewHighScore(InNewHighScore)
	{
	}

	FHighScoreEventData()
	{
	}

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APawn> Player = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int NewHighScore = 0;
};

UCLASS()
class ASTEROIDS_API UAsteroidsScoreManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	static UAsteroidsScoreManager* GetScoreManager(const UWorld& World);

	void UpdatePlayerScore(const int ScoreUpdateAmount, const APawn& Pawn) const;

	UFUNCTION(BlueprintCallable)
	int GetCurrentScore(const APlayerState* PlayerState) const;

private:

	void CheckIsHighScore(const APlayerState& PlayerState) const;

	UFUNCTION(BlueprintCallable)
	static void SetNewHighScores(const FHighScore& NewHighScore);

	UFUNCTION(BlueprintPure)
	bool IsNewHighScore(const APlayerState* PlayerState) const;

	static constexpr int Max_High_Scores = 5;

	bool bIsServerAuthoritative = false;
};
