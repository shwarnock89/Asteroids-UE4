// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AsteroidsGameMode.h"

#include "AsteroidsGameState.h"
#include "AsteroidsPawn.h"
#include "AsteroidsPlayerController.h"
#include "AsteroidsPlayerState.h"

AAsteroidsGameMode::AAsteroidsGameMode()
{
	// set default pawn class to our character class
	PlayerControllerClass = AAsteroidsPlayerController::StaticClass();
	DefaultPawnClass = AAsteroidsPawn::StaticClass();
	PlayerStateClass = AAsteroidsPlayerState::StaticClass();
	GameStateClass = AAsteroidsGameState::StaticClass();
}

void AAsteroidsGameMode::BeginPlay()
{
	Super::BeginPlay();

	AAsteroidsGameState* AsteroidsGameState = Cast<AAsteroidsGameState>(GameState);
	if (!ensureAlways(IsValid(AsteroidsGameState)))
	{
		return;
	}

	AsteroidsGameState->SetGameModeType(EGameModeType::CoOp);
}