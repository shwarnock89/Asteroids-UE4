// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Utils/HighScoreStruct.h"

#include "AsteroidsSaveGame.generated.h"

UCLASS()
class ASTEROIDS_API UAsteroidsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FHighScore> HighScores;
};
