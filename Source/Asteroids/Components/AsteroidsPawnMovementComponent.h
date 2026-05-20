#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "AsteroidsPawnMovementComponent.generated.h"

USTRUCT()
struct FShipState
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	uint8 TeleportCount = 0;
};

UCLASS()
class UAsteroidsPawnMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:

	UAsteroidsPawnMovementComponent();

	void SetInputVector(const FVector2D& Input);

	FVector GetVelocity() const { return Velocity; }

private:

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float ThrustStrength = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float RotationSpeed = 180.f;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MaxSpeed = 2500.f;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float LinearDamping = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float BounceFactor = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MinBounceSpeed = 300.0f;

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FShipState ServerState;

	FVector Velocity;

	FVector2D CurrentInput;

	FVector TargetLocation = FVector::ZeroVector;
	FVector TargetVelocity = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	uint8 LocalTeleportCount = 0;

	virtual void BeginPlay() override;
	virtual void TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void DoMovement(const float DeltaTime, const FRotator& NewRotation);

	FDelegateHandle OnHitHandle;
	FDelegateHandle OnTeleportHandle;

	UFUNCTION()
	void OnRep_ServerState();

	void DoReflection(const FHitResult& Hit);

	UFUNCTION()
	virtual void HandleTeleportOccurred();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};