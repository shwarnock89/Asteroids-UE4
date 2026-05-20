// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AsteroidsPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerScoreUpdated, const int, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewHighScore, const FHighScoreEventData&, Data);

UCLASS()
class ASTEROIDS_API AAsteroidsPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	void AddToScore(const int ScoreIncrease);

	UPROPERTY(BlueprintAssignable)
	FOnNewHighScore OnNewHighScore;

private:

	virtual void BeginPlay() override;

	virtual void OnRep_Score() override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerScoreUpdated OnPlayerScoreUpdated;
};
