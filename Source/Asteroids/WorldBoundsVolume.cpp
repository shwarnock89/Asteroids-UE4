// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldBoundsVolume.h"

#include "Kismet/GameplayStatics.h"
#include "Utils/AsteroidEntitySpawned.h"

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

bool AWorldBoundsVolume::IsValidWorld()
{
	return UGameplayStatics::GetActorOfClass(GWorld, StaticClass()) != nullptr;
}

FVector AWorldBoundsVolume::GetValidWorldLocation()
{
	const AWorldBoundsVolume* WorldBoundsVolume = Cast<AWorldBoundsVolume>(UGameplayStatics::GetActorOfClass(GWorld, AWorldBoundsVolume::StaticClass()));
	if (!ensureAlways(IsValid(WorldBoundsVolume)))
	{
		return FVector::ZeroVector;
	}

	FVector WorldPosition = FMath::RandPointInBox(FBox(-WorldBoundsVolume->HalfExtents, WorldBoundsVolume->HalfExtents));
	WorldPosition.Z = 0.0f;
	return WorldPosition;
}

// Called when the game starts or when spawned
void AWorldBoundsVolume::BeginPlay()
{
	Super::BeginPlay();

	UAsteroidEntitySpawnerSubsystem::OnAsteroidEntitySpawned.BindUObject(this, &AWorldBoundsVolume::HandleAsteroidEntitySpawned);
}

void AWorldBoundsVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UAsteroidEntitySpawnerSubsystem::OnAsteroidEntitySpawned.Unbind();
}

void AWorldBoundsVolume::HandleAsteroidEntitySpawned(AActor* SpawnedActor, const EHandlingType HandlingType)
{
	FTrackedActor TrackedActor;
	TrackedActor.Actor = SpawnedActor;
	TrackedActor.HandlingType = HandlingType;
	TrackedActors.Add(TrackedActor);
	SpawnedActor->OnDestroyed.AddDynamic(this, &AWorldBoundsVolume::HandleEntityDestroyed);
}

void AWorldBoundsVolume::HandleEntityDestroyed(AActor* DestroyedActor)
{
	for (const FTrackedActor& TrackedActor : TrackedActors)
	{
		if (DestroyedActor == TrackedActor.Actor)
		{
			TrackedActors.RemoveSingleSwap(TrackedActor);
			break;
		}
	}
}

// Called every frame
void AWorldBoundsVolume::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (int32 i = TrackedActors.Num() - 1; i >= 0; --i)
	{
		const FTrackedActor& TrackedActor = TrackedActors[i];
		if (!FMath::PointBoxIntersection(TrackedActor.Actor->GetActorLocation(), FBox(-HalfExtents, HalfExtents)))
		{
			switch (TrackedActor.HandlingType)
			{
				case EHandlingType::Despawn:
					TrackedActor.Actor->Destroy();
					break;
				case EHandlingType::Flip:
				{
					FlipActorWorldPosition(TrackedActor.Actor);
					break;
				}

				default:
					checkNoEntry();
			}
		}
	}

	if (GetWorld()->WorldType == EWorldType::Editor)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), HalfExtents, FColor::Red, false);
	}
}

void AWorldBoundsVolume::FlipActorWorldPosition(AActor* Actor) const
{
	if (!ensureAlways(IsValid(Actor)))
	{
		return;
	}

	const FVector CurrentLocation = Actor->GetActorLocation();
	FVector NewLocation = CurrentLocation;
	const FBox Bounds = FBox(-HalfExtents, HalfExtents);

	// Check X Bounds (Left/Right)
	if (CurrentLocation.X > Bounds.Max.X)
	{
		NewLocation.X = Bounds.Min.X;
	}
	else if (CurrentLocation.X < Bounds.Min.X)
	{
		NewLocation.X = Bounds.Max.X;
	}

	// Check Y Bounds (Top/Bottom)
	if (CurrentLocation.Y > Bounds.Max.Y)
	{
		NewLocation.Y = Bounds.Min.Y;
	}
	else if (CurrentLocation.Y < Bounds.Min.Y)
	{
		NewLocation.Y = Bounds.Max.Y;
	}

	if (NewLocation != CurrentLocation)
	{
		// Teleport to the opposite side
		Actor->SetActorLocation(NewLocation);
	}
}
