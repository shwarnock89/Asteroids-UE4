// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/Delegate.h"
#include "MessageStruct.h"
#include "Messanger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamageDealtDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAsteroidDestroyedDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBulletDestroyedDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdatePlayerScoreDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerScoreWasUpdatedDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerDiedDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthPackDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShieldsUpdatedDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateLevelDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewHighScoreDelegate, FMessage, message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFireButtonPressedDelegate);

UCLASS()
class ASTEROIDS_API UMessanger : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPlayerDiedDelegate OnPlayerDied;

	UPROPERTY(BlueprintAssignable)
	FHealthPackDelegate OnHealthPackPickedUp;

	UPROPERTY(BlueprintAssignable)
	FNewHighScoreDelegate OnNewHighScore;

	UFUNCTION(BlueprintCallable)
	void PlayerDied(FMessage message);

	UFUNCTION(BlueprintCallable)
	void FireShot();
};
