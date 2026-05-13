// Fill out your copyright notice in the Description page of Project Settings.


#include "AsteroidsGameInstance.h"

UAsteroidsGameInstance::UAsteroidsGameInstance()
{
	Messenger = NewObject<UMessanger>();
}

UMessanger* UAsteroidsGameInstance::GetMessanger() const
{
	return Messenger;
}
