// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateSessionFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateSessionSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinSessionFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnJoinSessionSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNoJoinableSessionsFound);

UCLASS()
class ASTEROIDS_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void CreateSession();

	UFUNCTION(BlueprintCallable)
	void JoinSession();

	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionFailed OnCreateSessionFailed;

	UPROPERTY(BlueprintAssignable)
	FOnCreateSessionSuccess OnCreateSessionSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionFailed OnJoinSessionFailed;

	UPROPERTY(BlueprintAssignable)
	FOnJoinSessionSuccess OnJoinSessionSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnNoJoinableSessionsFound OnNoJoinableSessionsFound;

private:

	void OnCreateCompleted(const FName SessionName, const bool bWasSuccessful);

	void OnStartCompleted(const FName SessionName, const bool bWasSuccessful);

	void OnFindSessionCompleted(const bool bWasSuccessful);

	void OnJoinSessionCompleted(FName SessionName, const EOnJoinSessionCompleteResult::Type Result);

	void HandleJoinSession(const FOnlineSessionSearchResult& SearchResult);

	IOnlineSessionPtr GetOnlineSessionPtr() const;

	FUniqueNetIdPtr GetUserId() const;

	FDelegateHandle OnCreateSessionComplete_DelegateHandle;
	FDelegateHandle OnStartSessionComplete_DelegateHandle;
	FDelegateHandle OnFindSessionComplete_DelegateHandle;
	FDelegateHandle OnJoinSessionComplete_DelegateHandle;

	TSharedPtr<FOnlineSessionSearch> SearchObject = nullptr;

	TArray<FUniqueNetIdPtr> UserMidCreateJoin;
};
