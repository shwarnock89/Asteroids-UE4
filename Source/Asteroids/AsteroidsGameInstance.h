// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Utils/Messanger.h"
#include "AsteroidsGameInstance.generated.h"

UCLASS()
class ASTEROIDS_API UAsteroidsGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	UMessanger* GetMessanger() const;

	UAsteroidsGameInstance();

private:

	UPROPERTY()
	UMessanger* Messenger = nullptr;
};
