// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_FindLobby.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"




UAsyncAction_FindLobby* UAsyncAction_FindLobby::FindLobby(
	UObject* WorldContextObject,
	APlayerController* Player,
	TMap<FName, FEIKAttribute> LobbySettings,
	int32 MaxResults)
{
	if (!WorldContextObject || !Player)
	{
		UE_LOG(LogTemp, Error, TEXT("UAsyncAction_FindLobby::FindLobby: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_FindLobby* Action = NewObject<UAsyncAction_FindLobby>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->LobbySettings = LobbySettings;
	Action->MaxResults = MaxResults;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_FindLobby::Activate()
{
	Execute_FindLobby();
	Super::Activate();
}

void UAsyncAction_FindLobby::Execute_FindLobby()
{
	if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
	{
		if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionSearch->MaxSearchResults = MaxResults;
			SessionSearch->bIsLanQuery = false;

			FName TemplateName(NAME_GameSession);
			SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
			SessionSearch->QuerySettings.Set(SETTING_SESSION_TEMPLATE_NAME, TemplateName.ToString(), EOnlineComparisonOp::Equals);

			for (const TPair<FName, FEIKAttribute>& LobbySetting : LobbySettings)
			{
				switch (LobbySetting.Value.AttributeType)
				{
				case EEIKAttributeType::Integer:
					SessionSearch->QuerySettings.Set(LobbySetting.Key, LobbySetting.Value.IntValue, EOnlineComparisonOp::Equals);
					break;
				case EEIKAttributeType::Bool:
					SessionSearch->QuerySettings.Set(LobbySetting.Key, LobbySetting.Value.BoolValue, EOnlineComparisonOp::Equals);
					break;
				case EEIKAttributeType::String:
					SessionSearch->QuerySettings.Set(LobbySetting.Key, LobbySetting.Value.StringValue, EOnlineComparisonOp::Equals);
					break;
				default:
					break;
				}
			}

			SessionSearch->PingBucketSize = 50;
			FindLobbyDelegateHandle = SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindLobbyCompleted);

			ULocalPlayer* LocalPlayer = Player->GetLocalPlayer();
			if (LocalPlayer)
			{
				FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
				if (UserId.IsValid())
				{
					SessionPtr->FindSessions(*UserId, SessionSearch.ToSharedRef());

					return ;
				}
			}
		}
	}


	auto HandleFailure = [this]()
	{
		OnFailure.Broadcast(TArray<FString>());
		SetReadyToDestroy();
	};

	UWorld* World = WorldContextObject->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(HandleFailure));
	}
	else
	{
		HandleFailure();
	}
}


void UAsyncAction_FindLobby::OnFindLobbyCompleted(bool bWasSuccessful)
{
	const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	SessionPtr->OnFindSessionsCompleteDelegates.Remove(FindLobbyDelegateHandle);

	if (bWasSuccessful)
	{
		TArray<FString> Lobbies;
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			Lobbies.Add(Result.GetSessionIdStr());
		}
		OnSuccess.Broadcast(Lobbies);
	}
	else
	{
		OnFailure.Broadcast(TArray<FString>());
	}
	SetReadyToDestroy();
}
