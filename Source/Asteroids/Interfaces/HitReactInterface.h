// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitReactInterface.generated.h"

DECLARE_DELEGATE_TwoParams(FOnHitOccurred, const FHitResult&, const AActor*);

UINTERFACE()
class UHitReactInterface : public UInterface
{
	GENERATED_BODY()
};

class ASTEROIDS_API IHitReactInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent)
	void HandleHitOccurred(const FHitResult& HitResult, const AActor* HitActor);
	void HandleHitOccurred_Implementation(const FHitResult& HitResult, const AActor* HitActor) { OnHit.ExecuteIfBound(HitResult, HitActor); }

	FOnHitOccurred& GetOnHitOccurred() { return OnHit; }

private:

	FOnHitOccurred OnHit;
};
