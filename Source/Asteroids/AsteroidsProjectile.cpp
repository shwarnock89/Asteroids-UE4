// Copyright 1998-2019 Epic Games, Inc. All Rights Reserve

#include "AsteroidsProjectile.h"

#include "Asteroid.h"
#include "Components/CapsuleComponent.h"

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

void AAsteroidsProjectile::OnHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult&)
{
	if (bIsPendingDestroy)
	{
		return;
	}

	if (!ensureAlways(OtherActor))
	{
		return;
	}

	if (!OtherActor->IsA<AAsteroid>())
	{
		return;
	}

	DestroyProjectile();
	bIsPendingDestroy = true;
}

void AAsteroidsProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
}

void AAsteroidsProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AAsteroidsProjectile::OnHit);
}

void AAsteroidsProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentHit.RemoveDynamic(this, &AAsteroidsProjectile::OnHit);
}
