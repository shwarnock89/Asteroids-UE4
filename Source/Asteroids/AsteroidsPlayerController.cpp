#include "AsteroidsPlayerController.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

AAsteroidsPlayerController::AAsteroidsPlayerController(const FObjectInitializer& InitializerModule)
	: Super(InitializerModule)
{
	bAutoManageActiveCameraTarget = false;
}

void AAsteroidsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AssignCamera();
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
