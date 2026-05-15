// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldBoundsVolume.h"

#include "AsteroidsProjectile.generated.h"

UCLASS()
class AAsteroidsProjectile : public AActor, public IWorldBoundsHandlingInterface
{
	GENERATED_BODY()

	AAsteroidsProjectile();

	virtual UCapsuleComponent* GetCapsuleComponent_Implementation() const override { return CapsuleComponent; }
	virtual EHandlingType GetHandlingType_Implementation() const override { return EHandlingType::Despawn; }

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void DestroyProjectile();

	UPROPERTY()
	UCapsuleComponent* CapsuleComponent = nullptr;

	bool bIsPendingDestroy = false;
};

