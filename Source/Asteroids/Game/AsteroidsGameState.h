// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "AsteroidsGameState.generated.h"

UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	CoOp,
	Competitive
};

UCLASS()
class ASTEROIDS_API AAsteroidsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	AAsteroidsGameState(const FObjectInitializer& ObjectInitializer);

	EGameModeType GetGameModeType() const { return GameModeType; }

	UFUNCTION(BlueprintCallable)
	void SetGameModeType(const EGameModeType NewGameModeType) { GameModeType = NewGameModeType; }

	void AddToTeamScore(const int ScoreIncrease) { TeamScore += ScoreIncrease; }

	int GetTeamScore() const { return TeamScore; }

private:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	EGameModeType GameModeType = EGameModeType::CoOp;

	UPROPERTY(Replicated)
	int TeamScore = 0;
};
