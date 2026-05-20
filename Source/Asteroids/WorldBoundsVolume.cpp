// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldBoundsVolume.h"

#include "Components/CapsuleComponent.h"

void UWorldBoundsVolumeSubsystem::SetWorldBoundsVolume(AWorldBoundsVolume& InWorldBoundsVolume)
{
	WorldBoundsVolume = &InWorldBoundsVolume;
	OnWorldBoundsVolumeSpawned.ExecuteIfBound(WorldBoundsVolume);
}

FVector UWorldBoundsVolumeSubsystem::FindClosestPointToLocation(const FVector& Location, const FVector& Padding) const
{
	FBox Bounds = WorldBoundsVolume->GetComponentsBoundingBox();
	Bounds.Min.X += Padding.X;
	Bounds.Min.Y += Padding.Y;
	Bounds.Max.X -= Padding.X;
	Bounds.Max.Y -= Padding.Y;

	FVector ClosestPoint = Bounds.GetClosestPointTo(Location);
	ClosestPoint.Z = 0.0f;
	return ClosestPoint;
}

FVector UWorldBoundsVolumeSubsystem::GetValidWorldLocation(const FVector& OptionalPadding) const
{
	if (!ensureAlways(IsValid(WorldBoundsVolume)))
	{
		return FVector::ZeroVector;
	}

	FBox Bounds = WorldBoundsVolume->GetComponentsBoundingBox();
	Bounds.Min.X += OptionalPadding.X;
	Bounds.Min.Y += OptionalPadding.Y;
	Bounds.Max.X -= OptionalPadding.X;
	Bounds.Max.Y -= OptionalPadding.Y;

	FVector WorldPosition = FMath::RandPointInBox(Bounds);
	WorldPosition.Z = 0.0f;
	return WorldPosition;
}

// Sets default values
AWorldBoundsVolume::AWorldBoundsVolume()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create a dummy scene component to hold the Transform
	USceneComponent* DummyRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	RootComponent = DummyRoot;
}

// Called when the game starts or when spawned
void AWorldBoundsVolume::BeginPlay()
{
	Super::BeginPlay();

	if (!ensureAlways(IsValid(GetWorld())))
	{
		return;
	}

	UWorldBoundsVolumeSubsystem* WorldBoundsVolumeSubsystem = GetWorld()->GetSubsystem<UWorldBoundsVolumeSubsystem>();
	if (!ensureAlways(IsValid(WorldBoundsVolumeSubsystem)))
	{
		return;
	}

	WorldBoundsVolumeSubsystem->SetWorldBoundsVolume(*this);

	if (!ensureAlways(IsValid(GetCollisionComponent())))
	{
		return;
	}

	GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &AWorldBoundsVolume::HandleEndOverlap);
}

void AWorldBoundsVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!ensureAlways(IsValid(GetCollisionComponent())))
	{
		return;
	}

	GetCollisionComponent()->OnComponentEndOverlap.RemoveDynamic(this, &AWorldBoundsVolume::HandleEndOverlap);
}

void AWorldBoundsVolume::HandleEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ensureAlways(IsValid(OtherActor) && OtherActor->Implements<UWorldBoundsHandlingInterface>()))
	{
		return;
	}

	switch (IWorldBoundsHandlingInterface::Execute_GetHandlingType(OtherActor))
	{
		case EHandlingType::Despawn:
			OtherActor->Destroy();
			break;
		case EHandlingType::Flip:
			FlipActorWorldPosition(*OtherActor);
			break;
		default:
			checkNoEntry();
	}
}

// Called every frame
void AWorldBoundsVolume::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Red, false);
	}
}

void AWorldBoundsVolume::FlipActorWorldPosition(AActor& Actor) const
{
	const FVector CurrentLocation = Actor.GetActorLocation();
	const UCapsuleComponent* CapsuleComponent = IWorldBoundsHandlingInterface::Execute_GetCapsuleComponent(&Actor);
	if (!ensureAlways(IsValid(CapsuleComponent)))

	{
		return;
	}

	FVector NewLocation = CurrentLocation;
	const FBox Bounds = GetComponentsBoundingBox();

	// Check X Bounds (Left/Right)
	if (CurrentLocation.X > Bounds.Max.X)
	{
		NewLocation.X = Bounds.Min.X + 10.0f;
	}
	else if (CurrentLocation.X < Bounds.Min.X)

	{
		NewLocation.X = Bounds.Max.X - 10.0f;
	}

	// Check Y Bounds (Top/Bottom)
	if (CurrentLocation.Y > Bounds.Max.Y)

	{
		NewLocation.Y = Bounds.Min.Y + 10.0f;
	}

	else if (CurrentLocation.Y < Bounds.Min.Y)

	{
		NewLocation.Y = Bounds.Max.Y - 10.0f;
	}

	if (NewLocation != CurrentLocation)

	{
		// Teleport to the opposite side

		Actor.SetActorLocation(NewLocation);
		IWorldBoundsHandlingInterface::Execute_FireOnTeleportEvent(&Actor);
	}
}