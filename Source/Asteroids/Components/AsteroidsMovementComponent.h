#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "AsteroidsMovementComponent.generated.h"

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
	bool bTeleported = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UAsteroidsMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:

	UAsteroidsMovementComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetInputVector(const FVector2D& Input);

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

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FShipState ServerState;

	FVector Velocity;

	FVector2D CurrentInput;

	FVector TargetLocation = FVector::ZeroVector;
	FVector TargetVelocity = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	UFUNCTION()
	void OnRep_ServerState();

	UFUNCTION()
	virtual void HandleTeleportOccurred();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};