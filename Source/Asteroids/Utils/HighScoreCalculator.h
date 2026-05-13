// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HighScoreStruct.h"
#include "HighScoreCalculator.generated.h"

/**
 * Utility class for setting high scores list
 */
UCLASS(Blueprintable)
class ASTEROIDS_API UHighScoreCalculator : public UObject
{
	GENERATED_BODY()

public:
	bool static IsNewHighScore(const int NewHighScore);

	UFUNCTION(BlueprintCallable, Category = "Save High Score")
	static void SetNewHighScores(const FHighScore& NewHighScore);

private:
	static constexpr int Max_High_Scores = 5;
};
