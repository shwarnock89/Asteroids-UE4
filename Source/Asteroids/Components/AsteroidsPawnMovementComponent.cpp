#include "AsteroidsPawnMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Interfaces/HitReactInterface.h"
#include "WorldBoundsVolume.h"

UAsteroidsPawnMovementComponent::UAsteroidsPawnMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	SetIsReplicatedByDefault(true);
}

void UAsteroidsPawnMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	SetUpdatedComponent(PawnOwner->GetRootComponent());

	if (!ensureAlways(IsValid(PawnOwner) && PawnOwner->Implements<UWorldBoundsHandlingInterface>()))
	{
		return;
	}

	IWorldBoundsHandlingInterface* WorldBoundsHandlingInterface = Cast<IWorldBoundsHandlingInterface>(PawnOwner);
	if (!ensureAlways(WorldBoundsHandlingInterface))
	{
		return;
	}

	OnTeleportHandle = WorldBoundsHandlingInterface->GetOnTeleport().AddUObject(this, &UAsteroidsPawnMovementComponent::HandleTeleportOccurred);

	UCapsuleComponent* CapsuleComponent = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(PawnOwner);
	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &UAsteroidsPawnMovementComponent::HandleOverlap);
}

void UAsteroidsPawnMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!ensureAlways(IsValid(PawnOwner) && PawnOwner->Implements<UWorldBoundsHandlingInterface>()))
	{
		return;
	}

	IWorldBoundsHandlingInterface* WorldBoundsHandlingInterface = Cast<IWorldBoundsHandlingInterface>(PawnOwner);
	if (!ensureAlways(WorldBoundsHandlingInterface))
	{
		return;
	}

	WorldBoundsHandlingInterface->GetOnTeleport().Remove(OnTeleportHandle);

	UCapsuleComponent* CapsuleComponent = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(PawnOwner);
	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentBeginOverlap.RemoveDynamic(this, &UAsteroidsPawnMovementComponent::HandleOverlap);
}

void UAsteroidsPawnMovementComponent::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult& SweepResult)
{
	if (Velocity.IsNearlyZero() && IsValid(OtherActor) && OtherActor->Implements<UHitReactInterface>())
	{
		DoReflection(SweepResult);
	}
}

void UAsteroidsPawnMovementComponent::HandleTeleportOccurred()
{
	if (!ensureAlways(IsValid(GetOwner())))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	++ServerState.TeleportCount;
}

void UAsteroidsPawnMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UAsteroidsPawnMovementComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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
		const FVector Forward = PawnOwner->GetActorForwardVector();
		Velocity += Forward * ThrustStrength * CurrentInput.Y * DeltaTime;
	}

	//---------------------------------
	// DRAG
	//---------------------------------

	Velocity *= FMath::Exp(-LinearDamping * DeltaTime);

	//---------------------------------
	// MOVE
	//---------------------------------

	DoMovement(DeltaTime, NewRot);

	if (bIsServer && Velocity.IsNearlyZero())
	{
		DoStationaryCollisionCheck();
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

void UAsteroidsPawnMovementComponent::DoStationaryCollisionCheck()
{
	if (!ensureAlways(IsValid(GetOwner()) && GetOwner()->HasAuthority()))
	{
		return;
	}

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(UpdatedComponent);
	if (!ensureAlways(IsValid(PrimitiveComponent)))
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams Params;
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), UpdatedComponent->GetComponentQuat(), Params, PrimitiveComponent->GetCollisionShape());
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (!Overlap.bBlockingHit)
		{
			continue;
		}

		if (Overlap.GetActor() == PawnOwner)
		{
			continue;
		}

		if (!Overlap.GetActor()->Implements<UHitReactInterface>())
		{
			return;
		}

		const AActor* OverlapActor = Overlap.GetActor();
		if (!ensureAlways(IsValid(OverlapActor)))
		{
			return;
		}

		FHitResult SyntheticHit;
		SyntheticHit.bBlockingHit = true;
		SyntheticHit.Component = Overlap.Component;
		SyntheticHit.ImpactNormal = (GetActorLocation() - OverlapActor->GetActorLocation()).GetSafeNormal();
		SyntheticHit.Normal = SyntheticHit.ImpactNormal;
		SyntheticHit.Location = GetActorLocation();
		SyntheticHit.HitObjectHandle = Overlap.OverlapObjectHandle;

		FVector IncomingVelocity = OverlapActor->GetVelocity();
		if (IncomingVelocity.IsNearlyZero())
		{
			// Fallback but shouldn't happen
			IncomingVelocity = -SyntheticHit.ImpactNormal * 300.0f;
		}

		Velocity = IncomingVelocity;
		DoReflection(SyntheticHit);
	}
}

void UAsteroidsPawnMovementComponent::DoMovement(const float DeltaTime, const FRotator& NewRotation)
{
	if (Velocity.SizeSquared() < 1.0f)
	{
		Velocity = FVector::ZeroVector;
	}

	const FVector MoveDelta = Velocity * DeltaTime;
	if (MoveDelta.IsNearlyZero())
	{
		return;
	}

	FHitResult Hit;
	SafeMoveUpdatedComponent(Velocity * DeltaTime, NewRotation, true, Hit);

	if (Hit.bBlockingHit)
	{
		DoReflection(Hit);
	}
}

void UAsteroidsPawnMovementComponent::SetInputVector(const FVector2D& Input)
{
	CurrentInput = Input;
}

void UAsteroidsPawnMovementComponent::OnRep_ServerState()
{
	if (ServerState.TeleportCount != LocalTeleportCount)
	{
		if (!ensureAlways(IsValid(UpdatedComponent)))
		{
			return;
		}

		UpdatedComponent->SetWorldLocationAndRotation(ServerState.Position, ServerState.Rotation);

		Velocity = ServerState.Velocity;
		LocalTeleportCount = ServerState.TeleportCount;
	}

	TargetLocation = ServerState.Position;
	TargetVelocity = ServerState.Velocity;
	TargetRotation = ServerState.Rotation;
}

void UAsteroidsPawnMovementComponent::DoReflection(const FHitResult& Hit)
{
	if (!Hit.IsValidBlockingHit())
	{
		return;
	}

	if (!ensureAlways(IsValid(Hit.GetActor()) && Hit.GetActor()->Implements<UHitReactInterface>()))
	{
		return;
	}

	IHitReactInterface::Execute_HandleHitOccurred(Hit.GetActor(), Hit, GetOwner());;

	const FVector Normal = Hit.ImpactNormal.GetSafeNormal();

	// -------------------------------------------------
	// 2. Reflection
	// -------------------------------------------------

	FVector Reflected = FMath::GetReflectionVector(Velocity, Normal);

	// -------------------------------------------------
	// 3. Energy control (bounce feel)
	// -------------------------------------------------

	Reflected *= BounceFactor;

	// -------------------------------------------------
	// 4. Flatten Z (your 2D plane rule)
	// -------------------------------------------------

	Reflected.Z = 0.f;
	Velocity = Reflected;
}
