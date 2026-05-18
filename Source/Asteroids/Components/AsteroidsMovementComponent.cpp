#include "AsteroidsMovementComponent.h"

#include "GameFramework/Pawn.h"
#include "WorldBoundsVolume.h"

UAsteroidsMovementComponent::UAsteroidsMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UAsteroidsMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	SetUpdatedComponent(PawnOwner->GetRootComponent());

	if (!ensureAlways(IsValid(PawnOwner) && IsValid(PawnOwner->GetClass()) && PawnOwner->GetClass()->ImplementsInterface(UWorldBoundsHandlingInterface::StaticClass())))
	{
		return;
	}

	IWorldBoundsHandlingInterface* WorldBoundsHandlingInterface = Cast<IWorldBoundsHandlingInterface>(PawnOwner);
	if (!ensureAlways(WorldBoundsHandlingInterface))
	{
		return;
	}

	WorldBoundsHandlingInterface->GetOnTeleport().AddDynamic(this, &UAsteroidsMovementComponent::HandleTeleportOccurred);
}

void UAsteroidsMovementComponent::HandleTeleportOccurred()
{
	ServerState.bTeleported = true;
}

void UAsteroidsMovementComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UAsteroidsMovementComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ensureAlways(IsValid(PawnOwner)))
	{
		return;
	}

	if (!ensureAlways(IsValid(UpdatedComponent)))
	{
		return;
	}

	//-------------------------------------------------
	// SIMULATED PROXIES
	// Remote pawns only interpolate replicated state
	//-------------------------------------------------

	const bool bIsServer = PawnOwner->HasAuthority();
	const bool bIsAutonomous = PawnOwner->IsLocallyControlled();
	if (!bIsServer && !bIsAutonomous)
	{
		const FVector SmoothedPosition = FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, 10.0f);
		const FQuat SmoothedRotation = FQuat::Slerp(PawnOwner->GetActorQuat(), TargetRotation.Quaternion(), DeltaTime * 10.0f);
		UpdatedComponent->SetWorldLocationAndRotation(SmoothedPosition, SmoothedRotation);
		Velocity = FMath::VInterpTo(Velocity, TargetVelocity, DeltaTime, 8.0f);
		return;
	}

	//---------------------------------
	// ROTATION
	//---------------------------------

	FRotator NewRot = PawnOwner->GetActorRotation();
	NewRot.Yaw += CurrentInput.X * RotationSpeed * DeltaTime;
	PawnOwner->SetActorRotation(NewRot);

	//---------------------------------
	// THRUST
	//---------------------------------

	if (!FMath::IsNearlyZero(CurrentInput.Y))
	{
		const FVector Forward = NewRot.Vector();
		Velocity += Forward * ThrustStrength * CurrentInput.Y * DeltaTime;
	}

	//---------------------------------
	// DRAG
	//---------------------------------

	Velocity *= FMath::Exp(-LinearDamping * DeltaTime);

	//---------------------------------
	// MOVE
	//---------------------------------

	FHitResult Hit;
	SafeMoveUpdatedComponent(Velocity * DeltaTime, NewRot, true, Hit);

	//---------------------------------
	// COLLISION REFLECTION
	//---------------------------------

	if (Hit.IsValidBlockingHit())
	{
		Velocity = FMath::GetReflectionVector(Velocity, Hit.Normal);
		Velocity.Z = 0.f;
	}

	//-------------------------------------------------
	// AUTONOMOUS PROXY RECONCILIATION
	//-------------------------------------------------
	if (bIsAutonomous && !bIsServer)
	{
		const FVector Error = TargetLocation - GetActorLocation();
		const float ErrorSizeSq = Error.SizeSquared();

		//-------------------------------------------------
		// LARGE ERROR = HARD SNAP
		//-------------------------------------------------
		if (ErrorSizeSq > FMath::Square(250.0f))
		{
			PawnOwner->SetActorLocation(TargetLocation);
			PawnOwner->SetActorRotation(TargetRotation);
			Velocity = TargetVelocity;
		}

		//-------------------------------------------------
		// SMALL ERROR = SOFT CORRECTION
		//-------------------------------------------------
		else
		{
			static constexpr float CorrectionSpeed = 8.0f;

			const FVector Correction = Error * FMath::Clamp(DeltaTime * CorrectionSpeed, 0.0f, 1.0f);
			PawnOwner->AddActorWorldOffset(Correction);

			Velocity = FMath::VInterpTo(Velocity, TargetVelocity, DeltaTime, 8.0f);

			const FQuat SmoothedRotation = FQuat::Slerp(PawnOwner->GetActorQuat(), TargetRotation.Quaternion(), DeltaTime * 8.f);
			PawnOwner->SetActorRotation(SmoothedRotation);
		}
	}

	//-------------------------------------------------
	// CLEAR INPUT
	//-------------------------------------------------
	CurrentInput = FVector2D::ZeroVector;

	//-------------------------------------------------
	// SERVER UPDATES AUTHORITATIVE STATE
	//-------------------------------------------------
	if (bIsServer)
	{
		ServerState.Position = UpdatedComponent->GetComponentLocation();
		ServerState.Rotation = UpdatedComponent->GetComponentRotation();
		ServerState.Velocity = Velocity;
	}
}

void UAsteroidsMovementComponent::SetInputVector(const FVector2D& Input)
{
	CurrentInput = Input;
}

void UAsteroidsMovementComponent::OnRep_ServerState()
{
	if (!ensureAlways(IsValid(PawnOwner)))
	{
		return;
	}

	if (ServerState.bTeleported)
	{
		UpdatedComponent->SetWorldLocationAndRotation(ServerState.Position, ServerState.Rotation);

		Velocity = ServerState.Velocity;
	}

	TargetLocation = ServerState.Position;
	TargetVelocity = ServerState.Velocity;
	TargetRotation = ServerState.Rotation;
}
