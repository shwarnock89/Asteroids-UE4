#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "AsteroidsPlayerController.generated.h"

UCLASS()
class AAsteroidsPlayerController : public APlayerController
{
	GENERATED_BODY()

	AAsteroidsPlayerController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	void AssignCamera();
};