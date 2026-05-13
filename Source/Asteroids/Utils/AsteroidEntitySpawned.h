#pragma once

#include "CoreMinimal.h"

#include "AsteroidEntitySpawned.generated.h"

UENUM()
enum class EHandlingType : uint8
{
	Flip,
	Despawn
};

DECLARE_DELEGATE_TwoParams(FAsteroidEntitySpawned, AActor*, const EHandlingType);

UCLASS()
class UAsteroidEntitySpawnerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	inline static FAsteroidEntitySpawned OnAsteroidEntitySpawned;
};
