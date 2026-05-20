// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AsteroidsMovementComponent.generated.h"

USTRUCT()
struct FAsteroidState
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	bool bTeleported = false;
};


UCLASS()
class ASTEROIDS_API UAsteroidsMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	// Sets default values for this component's properties
	UAsteroidsMovementComponent();

	void SetVelocity(const FVector& InVelocity);

private:

	virtual void BeginPlay() override;
	virtual void TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void HandleTeleportOccurred();

	void DoAsteroidsReflection(const FHitResult& Hit, const AActor* HitActor);

	UFUNCTION()
	void OnRep_ServerState();

	void SimulateAsteroid(const float DeltaTime) const;

	void HandleHit(const FHitResult& Hit, const AActor* HitActor);

	UPROPERTY(EditDefaultsOnly)
	float MinDotToIgnoreReflection = 0.8f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState);
	FAsteroidState ServerState;

	FVector Velocity = FVector::ZeroVector;

	FVector TargetPosition = FVector::ZeroVector;
	FVector TargetVelocity = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> UpdatedComponent = nullptr;

	FDelegateHandle OnTeleportHandle;
};
