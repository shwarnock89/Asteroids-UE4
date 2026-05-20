// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AsteroidManager.generated.h"

class AAsteroid;

UENUM()
enum class ESizes : uint8
{
	Large,
	Medium,
	Small,
	None
};

UENUM()
enum class EStartSides : uint8
{
	Left = 0,
	Right = 1,
	Up = 2,
	Down = 3,
	None = 20
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateLevel, const int, CurrentLevel);

UCLASS()
class ASTEROIDS_API UAsteroidManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	UAsteroidManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	int GetCurrentAsteroidCount() const { return CurrentAsteroidCount; }
	int GetSpawnMultiplier() const { return SpawnMultiplier; }

	UPROPERTY(BlueprintAssignable)
	FOnUpdateLevel OnUpdateLevel;

private:
	const int Screen_Buffer = 40;

	void SpawnLevelInitialAsteroids(const int CurrentLevel);

	int CurrentAsteroidCount = 0;

	AAsteroid* CreateAsteroid(const FVector& StartPos, const EStartSides StartSide, const ESizes Size);

	UFUNCTION()
	void HandleAsteroidDamaged(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	int SpawnMultiplier = 0;
};
