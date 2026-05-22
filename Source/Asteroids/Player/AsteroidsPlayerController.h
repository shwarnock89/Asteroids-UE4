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

	virtual void OnPossess(APawn* InPawn) override;

	void AssignCamera();

	virtual void OnRep_PlayerState() override;

	void SetGameInput();

	void InitHUD();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> HUDClass = nullptr;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget = nullptr;

	bool bHUDInitialized = false;
};