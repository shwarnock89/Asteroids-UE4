#include "AsteroidsPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

AAsteroidsPlayerController::AAsteroidsPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoManageActiveCameraTarget = false;
}

void AAsteroidsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetGameInput();

	AssignCamera();
}

void AAsteroidsPlayerController::SetGameInput()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);

	bShowMouseCursor = false;
}

void AAsteroidsPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitHUD();
}

void AAsteroidsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	InitHUD();
}

void AAsteroidsPlayerController::InitHUD()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!ensureAlways(IsValid(PlayerState) && IsValid(GetPawn())))
	{
		return;
	}

	if (bHUDInitialized)
	{
		return;
	}

	bHUDInitialized = true;

	if (!ensureAlways(IsValid(HUDClass)))
	{
		return;
	}

	HUDWidget = CreateWidget<UUserWidget>(this, HUDClass);
	if (!ensureAlways(IsValid(HUDWidget)))
	{
		return;
	}

	HUDWidget->AddToViewport();
}

void AAsteroidsPlayerController::AssignCamera()
{
	AActor* CameraActor = UGameplayStatics::GetActorOfClass(GetWorld(), ACameraActor::StaticClass());
	if (!ensureAlways(IsValid(CameraActor)))
	{
		return;
	}

	SetViewTargetWithBlend(CameraActor, 0.0f);
}
