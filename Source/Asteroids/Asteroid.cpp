// Fill out your copyright notice in the Description page of Project Settings.

#include "Asteroid.h"

#include "AsteroidsMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 2. High-performance network frequencies
	bAlwaysRelevant = true;

	// 3. Initialize the Projectile Movement Component
	ProjectileMovement = CreateDefaultSubobject<UAsteroidsMovementComponent>(TEXT("ProjectileMovement"));

	// Configure the component for a frictionless, zero-gravity arcade environment
}

void AAsteroid::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
}

void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetReplicateMovement(true);
	}

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentHit.AddDynamic(this, &AAsteroid::OnHit);
}

void AAsteroid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (!ensureAlways(IsValid(CapsuleComponent)))
	{
		return;
	}

	CapsuleComponent->OnComponentHit.RemoveDynamic(this, &AAsteroid::OnHit);
}

// Called every frame
void AAsteroid::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		const FVector RotationDelta = RotationSpeed * DeltaTime;
		Rotation.Add(RotationDelta.X, RotationDelta.Y, 0);
		SetActorRotation(Rotation);
	}
}

bool AAsteroid::Initialize_Validate(const EStartSides InStartSide, const ESizes InSize)
{
	return InSize != ESizes::None;
}

void AAsteroid::Initialize_Implementation(const EStartSides InStartSide, const ESizes InSize)
{
	StartSide = InStartSide;
	Size = InSize;

	FVector MoveDirection = FVector::ZeroVector;

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
			break;
		case ESizes::Medium:
			MoveDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0);
			Scale = FMath::RandRange(4.0f, 6.0f);
			SetActorScale3D(FVector(Scale));
			break;
		case ESizes::Small:
			MoveDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0);
			Scale = FMath::RandRange(2.0f, 3.0f);
			SetActorScale3D(FVector(Scale));
			break;
		default: ;
	}

	ProjectileMovement->SetVelocity(MoveDirection * FMath::RandRange(500.0f, 1000.0f));

	const float Speed = FMath::RandRange(20, 120);
	RotationSpeed = FVector(Speed, 0, Speed);
	Rotation = FRotator(FMath::RandRange(0, 360), FMath::RandRange(0, 360), FMath::RandRange(0, 360));
}

void AAsteroid::OnHit(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, FVector, const FHitResult&)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ensureAlways(IsValid(OtherActor)))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(OtherActor, 10.0f, nullptr, this, nullptr);
}
