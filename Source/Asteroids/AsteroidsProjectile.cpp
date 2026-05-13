// Copyright 1998-2019 Epic Games, Inc. All Rights Reserve

#include "AsteroidsProjectile.h"
#include "AsteroidsGameInstance.h"
#include "Components/StaticMeshComponent.h"
#include "Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Utils/AsteroidEntitySpawned.h"
#include "Utils/ScreenUtil.h"

AAsteroidsProjectile::AAsteroidsProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bStartWithTickEnabled = true;

	InitialLifeSpan = 1.5f;
}

void AAsteroidsProjectile::DestroyProjectile()
{
	Destroy();
}

void AAsteroidsProjectile::OnBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	DestroyProjectile();
}

void AAsteroidsProjectile::BeginPlay()
{
	Super::BeginPlay();

	UAsteroidEntitySpawnerSubsystem::OnAsteroidEntitySpawned.ExecuteIfBound(this, EHandlingType::Despawn);

	UAsteroidsGameInstance* GameInstance = static_cast<UAsteroidsGameInstance*>(GetWorld()->GetGameInstance());
	Messenger = GameInstance->GetMessanger();

	USphereComponent* SphereComp = FindComponentByClass<USphereComponent>();
	if (!ensureAlways(IsValid(SphereComp)))
	{
		return;
	}

	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAsteroidsProjectile::OnBeginOverlap);
}

void AAsteroidsProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	FMessage Message = FMessage();
	Message.intMessage = 1;
	Messenger->BulletDestroyed(Message);
	
	USphereComponent* SphereComp = FindComponentByClass<USphereComponent>();
	if (!ensureAlways(IsValid(SphereComp)))
	{
		return;
	}

	SphereComp->OnComponentBeginOverlap.RemoveDynamic(this, &AAsteroidsProjectile::OnBeginOverlap);
}
