// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Utils/Messanger.h"
#include "GameFramework/GameModeBase.h"
#include "AsteroidManager.h"
#include "AsteroidsGameMode.generated.h"

UCLASS(MinimalAPI)
class AAsteroidsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAsteroidsGameMode();

protected:
	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	UPROPERTY()
	AAsteroidManager* AsteroidManager;

	UFUNCTION()
	void InitializeAsteroidManager() const;
};



