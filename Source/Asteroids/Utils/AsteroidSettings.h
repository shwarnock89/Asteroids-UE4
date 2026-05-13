// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AsteroidSettings.generated.h"

class AHealthPack;
class AAsteroid;

UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Asteroid Settings"))
class ASTEROIDS_API UAsteroidSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	virtual FName GetSectionName() const override { return FName("Asteroid Settings"); }

	virtual FName GetContainerName() const override { return FName("Project"); }

	virtual FName GetCategoryName() const override { return FName("Game"); }

	UPROPERTY(Config, EditDefaultsOnly)
	TSubclassOf<AAsteroid> AsteroidBaseClass = nullptr;

	UPROPERTY(Config, EditDefaultsOnly)
	TSubclassOf<AHealthPack> HealthPackClass = nullptr;
};
