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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamScoreChanged, const int, TeamScore);

UCLASS()
class ASTEROIDS_API AAsteroidsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	AAsteroidsGameState(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure)
	EGameModeType GetGameModeType() const { return GameModeType; }

	UFUNCTION(BlueprintCallable)
	void SetGameModeType(const EGameModeType NewGameModeType) { GameModeType = NewGameModeType; }

	void AddToTeamScore(const int ScoreIncrease);

	int GetTeamScore() const { return TeamScore; }

	UPROPERTY(BlueprintAssignable)
	FOnTeamScoreChanged OnTeamScoreChanged;

private:

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_TeamScore();

	UPROPERTY(Replicated)
	EGameModeType GameModeType = EGameModeType::CoOp;

	UPROPERTY(ReplicatedUsing = OnRep_TeamScore)
	int TeamScore = 0;
};
