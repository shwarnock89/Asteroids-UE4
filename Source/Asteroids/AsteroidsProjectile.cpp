// Copyright 1998-2019 Epic Games, Inc. All Rights Reserve

#include "AsteroidsProjectile.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

AAsteroidsProjectile::AAsteroidsProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bStartWithTickEnabled = true;

	InitialLifeSpan = 1.5f;

	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 2. High-performance network frequencies
	bAlwaysRelevant = true;
}

void AAsteroidsProjectile::DestroyProjectile()
{
	if (!HasAuthority())
	{
		return;
	}

	Destroy();
}

void AAsteroidsProjectile::OnHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult&)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsPendingDestroy)
	{
		return;
	}

	if (!ensureAlways(OtherActor))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, 10.0f, GetInstigator()->GetController(), this, nullptr);
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
