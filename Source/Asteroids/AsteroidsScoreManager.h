// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "AsteroidsScoreManager.generated.h"

struct FHighScore;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerScoreUpdated, const int, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewHighScore, const int, NewHighScore);

UCLASS()
class ASTEROIDS_API UAsteroidsScoreManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	static UAsteroidsScoreManager* GetScoreManager(const UWorld& World);

	virtual void RegisterServerWorld(UWorld& InServerWorld);

	UFUNCTION(BlueprintPure)
	int GetPlayerScore() const { return PlayerScore; }

	void CheckIsHighScore() const;

	UFUNCTION(BlueprintCallable)
	static void SetNewHighScores(const FHighScore& NewHighScore);

	// A helper function you can call anywhere in C++ or BP to check authority
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Networking", meta = (WorldContext = "WorldContextObject"))
	bool IsServerAuthority(UObject* WorldContextObject) const;

	UFUNCTION(BlueprintPure)
	bool IsNewHighScore() const;

	void UpdatePlayerScore(const int ScoreUpdateAmount);

	UPROPERTY(BlueprintAssignable)
	FOnPlayerScoreUpdated OnPlayerScoreUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnNewHighScore OnNewHighScore;

private:

	// Tracks the pointer of the active server world
	UPROPERTY()
	TWeakObjectPtr<UWorld> AuthoritativeServerWorld = nullptr;

	int PlayerScore = 0;
	static constexpr int Max_High_Scores = 5;

	bool bIsServerAuthoritative = false;
};
