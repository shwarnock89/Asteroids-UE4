// Fill out your copyright notice in the Description page of Project Settings.

#include "Messanger.h"

void UMessanger::PlayerDied(FMessage message)
{
	OnPlayerDied.Broadcast(message);
}

void UMessanger::FireShot()
{
	//OnFireButtonPressed.Broadcast();
}
