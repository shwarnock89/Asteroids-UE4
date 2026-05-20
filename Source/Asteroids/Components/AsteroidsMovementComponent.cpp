// Fill out your copyright notice in the Description page of Project Settings.

#include "AsteroidsMovementComponent.h"

#include "Interfaces/HitReactInterface.h"
#include "Net/UnrealNetwork.h"
#include "WorldBoundsVolume.h"

// Sets default values for this component's properties
UAsteroidsMovementComponent::UAsteroidsMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	SetIsReplicatedByDefault(true);
}

void UAsteroidsMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAsteroidsMovementComponent, ServerState);
}

// Called when the game starts
void UAsteroidsMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(GetOwner())))
	{
		return;
	}

	UpdatedComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());

	IWorldBoundsHandlingInterface* WorldBoundsHandlingInterface = Cast<IWorldBoundsHandlingInterface>(UpdatedComponent->GetOwner());
	if (!ensureAlways(WorldBoundsHandlingInterface))
	{
		return;
	}

	OnTeleportHandle = WorldBoundsHandlingInterface->GetOnTeleport().AddUObject(this, &UAsteroidsMovementComponent::HandleTeleportOccurred);

	IHitReactInterface* HitReactInterface = Cast<IHitReactInterface>(UpdatedComponent->GetOwner());
	if (!ensureAlways(HitReactInterface))
	{
		return;
	}

	HitReactInterface->GetOnHitOccurred().BindUObject(this, &UAsteroidsMovementComponent::HandleHit);
}

void UAsteroidsMovementComponent::HandleHit(const FHitResult& Hit, const AActor* HitActor)
{
	DoAsteroidsReflection(Hit, HitActor);
}

// Called every frame
void UAsteroidsMovementComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AActor* Owner = GetOwner();
	if (!ensureAlways(IsValid(Owner)))
	{
		return;
	}

	//-------------------------------------------------
	// SIMULATED PROXIES
	// Remote pawns only interpolate replicated state
	//-------------------------------------------------

	const bool bIsServer = Owner->HasAuthority();
	if (!bIsServer)
	{
		const FVector SmoothedPosition = FMath::VInterpTo(Owner->GetActorLocation(), TargetPosition, DeltaTime, 10.0f);
		const FQuat SmoothedRotation = FQuat::Slerp(Owner->GetActorQuat(), TargetRotation.Quaternion(), DeltaTime * 10.0f);
		UpdatedComponent->SetWorldLocationAndRotation(SmoothedPosition, SmoothedRotation);
		Velocity = FMath::VInterpTo(Velocity, TargetVelocity, DeltaTime, 8.0f);
		return;
	}

	//---------------------------------
	// SERVER AUTHORITY SIMULATION
	//---------------------------------
	if (bIsServer)
	{
		static constexpr int32 SubSteps = 4; // start with 3–5
		const float StepDT = DeltaTime / SubSteps;

		for (int32 i = 0; i < SubSteps; i++)
		{
			SimulateAsteroid(StepDT);
		}

		ServerState.Position = UpdatedComponent->GetComponentLocation();
		ServerState.Rotation = UpdatedComponent->GetComponentRotation();
		ServerState.Velocity = Velocity;
	}

	//---------------------------------
	// CLIENT AUTONOMOUS (if needed)
	//---------------------------------
}

void UAsteroidsMovementComponent::SimulateAsteroid(const float DeltaTime) const
{
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + Velocity * DeltaTime;
	UpdatedComponent->SetWorldLocation(End);
}

void UAsteroidsMovementComponent::DoAsteroidsReflection(const FHitResult& Hit, const AActor* HitActor)
{
	// if movements are similar ignore
	if (!ensureAlways(IsValid(Hit.GetActor())))
	{
		return;
	}

	const FVector HitActorVelocity = HitActor->GetVelocity();
	const FVector ThisVelocity = Velocity;
	if (FVector::DotProduct(HitActorVelocity.GetSafeNormal(), ThisVelocity.GetSafeNormal()) > MinDotToIgnoreReflection)
	{
		return;
	}

	const FVector Normal = (GetOwner()->GetActorLocation() - Hit.ImpactPoint).GetSafeNormal();
	FVector Reflected = FMath::GetReflectionVector(Velocity, Normal);
	Reflected.Z = 0.0f;
	Velocity = Reflected;
}

void UAsteroidsMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	IWorldBoundsHandlingInterface* WorldBoundsHandlingInterface = Cast<IWorldBoundsHandlingInterface>(UpdatedComponent->GetOwner());
	if (!ensureAlways(WorldBoundsHandlingInterface))
	{
		return;
	}

	WorldBoundsHandlingInterface->GetOnTeleport().Remove(OnTeleportHandle);

	IHitReactInterface* HitReactInterface = Cast<IHitReactInterface>(GetOwner());
	if (!ensureAlways(HitReactInterface))
	{
		return;
	}

	HitReactInterface->GetOnHitOccurred().Unbind();
}

void UAsteroidsMovementComponent::OnRep_ServerState()
{
	if (ServerState.TeleportCount != LocalTeleportCount)
	{
		if (!IsValid(UpdatedComponent))
		{
			return;
		}

		UpdatedComponent->SetWorldLocationAndRotation(ServerState.Position, ServerState.Rotation);

		Velocity = ServerState.Velocity;
		// Sync our local baseline history to match the current count
		LocalTeleportCount = ServerState.TeleportCount;
	}

	TargetPosition = ServerState.Position;
	TargetVelocity = ServerState.Velocity;
	TargetRotation = ServerState.Rotation;
}

void UAsteroidsMovementComponent::SetVelocity(const FVector& InVelocity)
{
	if (!ensureAlways(IsValid(GetOwner())))
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	Velocity = InVelocity;
}

void UAsteroidsMovementComponent::HandleTeleportOccurred()
{
	if (!ensureAlways(IsValid(GetOwner())))
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		return;
	}

	++ServerState.TeleportCount;
}
