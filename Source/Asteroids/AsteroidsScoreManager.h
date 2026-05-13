// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "AsteroidsScoreManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerScoreUpdated, const int, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewHighScore, const int, NewHighScore);

UCLASS()
class ASTEROIDS_API UAsteroidsScoreManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	static UAsteroidsScoreManager* GetScoreManager(const UWorld& World);

	UFUNCTION(BlueprintPure)
	int GetPlayerScore() const { return PlayerScore; }

	void CheckIsHighScore() const;

	UFUNCTION(BlueprintPure)
	bool IsNewHighScore() const;

	void UpdatePlayerScore(const int ScoreUpdateAmount);

	UPROPERTY(BlueprintAssignable)
	FOnPlayerScoreUpdated OnPlayerScoreUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnNewHighScore OnNewHighScore;

private:

	int PlayerScore = 0;
	static constexpr int Max_High_Scores = 5;
};
