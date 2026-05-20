// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidsGameState.h"

#include "Net/UnrealNetwork.h"

AAsteroidsGameState::AAsteroidsGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void AAsteroidsGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AAsteroidsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteroidsGameState, GameModeType);
	DOREPLIFETIME(AAsteroidsGameState, TeamScore);
}
