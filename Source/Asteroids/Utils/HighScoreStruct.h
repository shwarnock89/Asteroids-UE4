#pragma once

#include "HighScoreStruct.generated.h"

USTRUCT(BlueprintType)
struct FHighScore
{
	GENERATED_BODY()

	FHighScore()
	{
		Initials = FText();
		HighScore = NULL;
	}

	FHighScore(const FText& InInitials, const int InScore)
	{
		Initials = InInitials;
		HighScore = InScore;
	}

	bool IsValid() const { return HighScore > 0; }

	UPROPERTY(BlueprintReadWrite)
	FText Initials;

	UPROPERTY(BlueprintReadWrite)
	int HighScore;
};