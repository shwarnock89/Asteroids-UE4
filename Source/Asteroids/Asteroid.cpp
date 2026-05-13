// Fill out your copyright notice in the Description page of Project Settings.

#include "Asteroid.h"
#include "AsteroidsGameInstance.h"
#include "AsteroidsProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Utils/AsteroidEntitySpawned.h"
#include "Utils/ScreenUtil.h"

// Sets default values
AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	USphereComponent* SphereCollision = FindComponentByClass<USphereComponent>();
	if (!ensureAlways(IsValid(SphereCollision)))
	{
		return;
	}

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AAsteroid::OnBeginOverlap);
}

void AAsteroid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	USphereComponent* SphereCollision = FindComponentByClass<USphereComponent>();
	if (!ensureAlways(IsValid(SphereCollision)))
	{
		return;
	}

	SphereCollision->OnComponentBeginOverlap.RemoveDynamic(this, &AAsteroid::OnBeginOverlap);
}

// Called every frame
void AAsteroid::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector CurrentLocation = GetActorLocation();
	const FVector Movement = MoveDirection * MoveSpeed + CurrentLocation;
	SetActorLocation(Movement);

	const FVector RotationDelta = RotationSpeed * DeltaTime;
	Rotation.Add(RotationDelta.X, RotationDelta.Y, 0);
	SetActorRotation(Rotation);
}

void AAsteroid::Initialize(const EStartSides::START_SIDE InStartSide, const ESizes::SIZE InSize)
{
	MoveSpeed = FMath::RandRange(5, 10);
	StartSide = InStartSide;
	Size = InSize;

	UAsteroidEntitySpawnerSubsystem::OnAsteroidEntitySpawned.ExecuteIfBound(this, EHandlingType::Flip);

	float Scale = 0.0f;
	switch (Size)
	{
		case ESizes::Large:
		{
			const float Coefficient = FMath::RandRange(-0.9f, 0.9f);
			switch (StartSide)
			{
				case EStartSides::Left:
					MoveDirection = FVector(FMath::RandRange(0.2f, 1.0f), Coefficient, 0);
					break;
				case EStartSides::Right:
					MoveDirection = FVector(FMath::RandRange(-1.0f, -0.2f), Coefficient, 0.0f);
					break;
				case EStartSides::Up:
					MoveDirection = FVector(Coefficient, FMath::RandRange(0.2f, 1.0f), 0);
					break;
				case EStartSides::Down:
					MoveDirection = FVector(Coefficient, FMath::RandRange(-1.0f, -0.2f), 0);
					break;
				default:
					checkNoEntry();
			}
		}

			Scale = FMath::RandRange(7.0f, 9.0f);
			SetActorScale3D(FVector(Scale));
			Buffer = 75.0f;
			break;
		case ESizes::Medium:
			MoveDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0);
			Scale = FMath::RandRange(4.0f, 6.0f);
			SetActorScale3D(FVector(Scale));
			Buffer = 45.0f;
			break;
		case ESizes::Small:
			MoveDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0);
			Scale = FMath::RandRange(2.0f, 3.0f);
			SetActorScale3D(FVector(Scale));
			Buffer = 20.0f;
			break;
		default: ;
	}

	float Speed = FMath::RandRange(20, 120);
	RotationSpeed = FVector(Speed, 0, Speed);
	Rotation = FRotator(FMath::RandRange(0, 360), FMath::RandRange(0, 360), FMath::RandRange(0, 360));
}

void AAsteroid::OnBeginOverlap(UPrimitiveComponent*, AActor*, UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if (!ensureAlways(OtherComp && OtherComp->GetOwner()))
	{
		return;
	}

	if (OtherComp->GetOwner()->IsA<AAsteroidsProjectile>())
	{
		FMessage message = FMessage();
		message.intMessage = 1;
		message.asteroidSizeMessage = Size;
		message.currentPosMessage = GetActorLocation();
		UAsteroidsGameInstance* gameInstance = (UAsteroidsGameInstance*)GetWorld()->GetGameInstance();
		gameInstance->GetMessanger()->AsteroidDestroyed(message);
		Destroy();
	}
}
