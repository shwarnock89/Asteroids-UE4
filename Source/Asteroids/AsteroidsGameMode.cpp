// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AsteroidsGameMode.h"
#include "AsteroidsPlayerController.h"
#include "AsteroidsScoreManager.h"
#include "Engine.h"

AAsteroidsGameMode::AAsteroidsGameMode()
{
	// set default pawn class to our character class
	PlayerControllerClass = AAsteroidsPlayerController::StaticClass();
}

void AAsteroidsGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Grab the global persistent Game Instance
	const UGameInstance* GameInstance = GetGameInstance();
	if (!ensureAlways(IsValid(GameInstance)))
	{
		return;
	}

	UAsteroidsScoreManager* ScoreManager = GameInstance->GetSubsystem<UAsteroidsScoreManager>();
	if (!ensureAlways(IsValid(ScoreManager)))
	{
		return;
	}

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	// Inject this server world context into the subsystem
	ScoreManager->RegisterServerWorld(*GetWorld());
}