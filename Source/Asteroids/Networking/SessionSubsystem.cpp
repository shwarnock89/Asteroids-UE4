// Fill out your copyright notice in the Description page of Project Settings.

#include "SessionSubsystem.h"

#include "Asteroids/Utils/AsteroidSettings.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

IOnlineSessionPtr USessionSubsystem::GetOnlineSessionPtr() const
{
	const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld(), NAME_None);
	if (!ensureAlways(OnlineSubsystem))
	{
		return nullptr;
	}

	const IOnlineSessionPtr Sessions = OnlineSubsystem->GetSessionInterface();
	if (!ensureAlways(Sessions))
	{
		return nullptr;
	}

	return Sessions;
}

void USessionSubsystem::CreateSession()
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		OnCreateSessionFailed.Broadcast();
		return;
	}

	if (UserMidCreateJoin.Contains(UniqueId))
	{
		OnCreateSessionFailed.Broadcast();
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		OnCreateSessionFailed.Broadcast();
		return;
	}

	OnCreateSessionComplete_DelegateHandle = Sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateCompleted));

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = 4;
	Settings.bShouldAdvertise = true;
	Settings.bAllowJoinInProgress = true;
	Settings.bIsLANMatch = false;
	Settings.bUsesPresence = true;
	Settings.bAllowJoinViaPresence = true;
	Settings.bUseLobbiesIfAvailable = false;

	Sessions->CreateSession(*UniqueId, NAME_GameSession, Settings);
	UserMidCreateJoin.Emplace(UniqueId);
}

void USessionSubsystem::OnCreateCompleted(const FName, const bool bWasSuccessful)
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnCreateSessionFailed.Broadcast();
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnCreateSessionFailed.Broadcast();
		return;
	}

	Sessions->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionComplete_DelegateHandle);

	if (bWasSuccessful)
	{
		OnStartSessionComplete_DelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnStartCompleted));
		Sessions->StartSession(NAME_GameSession);
		return;
	}

	if (!bWasSuccessful)
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnCreateSessionFailed.Broadcast();
	}
}

void USessionSubsystem::OnStartCompleted(const FName, const bool bWasSuccessful)
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnCreateSessionFailed.Broadcast();
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnCreateSessionFailed.Broadcast();
		return;
	}

	Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionComplete_DelegateHandle);

	if (bWasSuccessful)
	{
		const UAsteroidSettings* AsteroidsSettings = GetDefault<UAsteroidSettings>();
		if (!ensureAlways(IsValid(AsteroidsSettings) && !AsteroidsSettings->AsteroidsWorld.IsNull()))
		{
			UserMidCreateJoin.Remove(UniqueId);
			OnCreateSessionFailed.Broadcast();
			return;
		}

		UGameplayStatics::OpenLevelBySoftObjectPtr(this, AsteroidsSettings->AsteroidsWorld, true, TEXT("listen"));
		OnCreateSessionSuccess.Broadcast();
		UserMidCreateJoin.Remove(UniqueId);
	}
	else
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnCreateSessionFailed.Broadcast();
	}
}

FUniqueNetIdPtr USessionSubsystem::GetUserId() const
{
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!ensureAlways(IsValid(PlayerController)))
	{
		return nullptr;
	}

	const APlayerState* PlayerState = ToRawPtr(PlayerController->PlayerState);
	if (!ensureAlways(IsValid(PlayerState)))
	{
		return nullptr;
	}

	return PlayerState->GetUniqueId().GetUniqueNetId();
}

void USessionSubsystem::JoinSession()
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		OnCreateSessionFailed.Broadcast();
		return;
	}

	if (UserMidCreateJoin.Contains(UniqueId))
	{
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		OnJoinSessionFailed.Broadcast();
		return;
	}

	OnFindSessionComplete_DelegateHandle = Sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnFindSessionCompleted));

	SearchObject = MakeShareable(new FOnlineSessionSearch);
	SearchObject->MaxSearchResults = 1;
	SearchObject->bIsLanQuery = false;

	Sessions->FindSessions(*UniqueId, SearchObject.ToSharedRef());
	UserMidCreateJoin.Emplace(UniqueId);
}

void USessionSubsystem::OnFindSessionCompleted(const bool bWasSuccessful)
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		OnCreateSessionFailed.Broadcast();
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnJoinSessionFailed.Broadcast();
		return;
	}

	Sessions->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionComplete_DelegateHandle);
	if (!ensureAlways(SearchObject.IsValid() && !SearchObject->SearchResults.IsEmpty()))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnJoinSessionFailed.Broadcast();
		return;
	}

	if (bWasSuccessful)
	{
		HandleJoinSession(SearchObject->SearchResults[0]);
	}
	else
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnJoinSessionFailed.Broadcast();
	}
}

void USessionSubsystem::HandleJoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		OnJoinSessionFailed.Broadcast();
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnJoinSessionFailed.Broadcast();
		return;
	}

	OnJoinSessionComplete_DelegateHandle = Sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &USessionSubsystem::OnJoinSessionCompleted));

	Sessions->JoinSession(*UniqueId, NAME_GameSession, SearchResult);
}

void USessionSubsystem::OnJoinSessionCompleted(FName, const EOnJoinSessionCompleteResult::Type Result)
{
	const FUniqueNetIdPtr UniqueId = GetUserId();
	if (!ensureAlways(UniqueId.IsValid() && UniqueId->IsValid()))
	{
		OnJoinSessionFailed.Broadcast();
		return;
	}

	const IOnlineSessionPtr Sessions = GetOnlineSessionPtr();
	if (!ensureAlways(Sessions))
	{
		UserMidCreateJoin.Remove(UniqueId);
		OnJoinSessionFailed.Broadcast();
		return;
	}

	Sessions->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionComplete_DelegateHandle);

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Client travel to the server
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (!ensureAlways(IsValid(PlayerController)))
		{
			UserMidCreateJoin.Remove(UniqueId);
			OnJoinSessionFailed.Broadcast();
			return;
		}

		FString ConnectString;
		if (Sessions->GetResolvedConnectString(NAME_GameSession, ConnectString))
		{
			UE_LOG_ONLINE_SESSION(Log, TEXT("Join session: traveling to %s"), *ConnectString);
			PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
			OnJoinSessionSuccess.Broadcast();
			UserMidCreateJoin.Remove(UniqueId);
			return;
		}
	}

	OnJoinSessionFailed.Broadcast();
}
