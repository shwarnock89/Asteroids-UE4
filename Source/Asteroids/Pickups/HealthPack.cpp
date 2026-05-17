// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthPack.h"

#include "Asteroids/AsteroidsHealthComponent.h"
#include "Asteroids/AsteroidsPawn.h"
#include "Components/CapsuleComponent.h"

AHealthPack::AHealthPack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
}

void AHealthPack::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &AHealthPack::OnBeginOverlap);
}

void AHealthPack::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AHealthPack::OnBeginOverlap);
}

void AHealthPack::OnBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if (!ensureAlways(IsValid(OtherComp) && IsValid(OtherComp->GetOwner())))
	{
		return;
	}

	AAsteroidsPawn* AsteroidsPawn = Cast<AAsteroidsPawn>(OtherComp->GetOwner());
	if (!IsValid(AsteroidsPawn))
	{
		return;
	}

	UAsteroidsHealthComponent* HealthComponent = AsteroidsPawn->GetHealthComponent();
	if (!ensureAlways(IsValid(HealthComponent)))
	{
		return;
	}

	HealthComponent->HandleHealthPackPickedUp(HealthIncrease);
	Destroy();
}