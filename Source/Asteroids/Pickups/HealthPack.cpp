// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthPack.h"
#include "Asteroids/AsteroidsGameInstance.h"
#include "Asteroids/AsteroidsPawn.h"
#include "Asteroids/Utils/ScreenUtil.h"
#include "Asteroids/WorldBoundsVolume.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

void AHealthPack::BeginPlay()
{
	Super::BeginPlay();

	const FVector WorldLocation = AWorldBoundsVolume::GetValidWorldLocation();
	SetActorLocation(WorldLocation);

	SetActorScale3D(FVector(.5, .5, .5));

	USphereComponent* SphereComponent = FindComponentByClass<USphereComponent>();
	if (!ensureAlways(IsValid(SphereComponent)))
	{
		return;
	}

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AHealthPack::OnBeginOverlap);
}

void AHealthPack::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	USphereComponent* SphereComponent = FindComponentByClass<USphereComponent>();
	if (!ensureAlways(IsValid(SphereComponent)))
	{
		return;
	}

	SphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AHealthPack::OnBeginOverlap);
}

void AHealthPack::OnBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if (!ensureAlways(IsValid(OtherComp) && IsValid(OtherComp->GetOwner())))
	{
		return;
	}

	if (OtherComp->GetOwner()->IsA<AAsteroidsPawn>())
	{
		FMessage message = FMessage();
		message.floatMessage = HealthIncrease;
		UAsteroidsGameInstance* gameInstance = (UAsteroidsGameInstance*) this->GetWorld()->GetGameInstance();
		gameInstance->GetMessanger()->HealthPackPickedUp(message);
		Destroy();
	}
}