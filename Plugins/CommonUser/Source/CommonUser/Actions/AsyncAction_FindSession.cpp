// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_FindSession.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "AsyncAction_LogChannel.h"


UAsyncAction_FindSession* UAsyncAction_FindSession::FindSession(UObject* WorldContextObject,
	APlayerController* Player, TMap<FName, FEIKAttribute> SessionSettings, int32 MaxResults)
{
	if (!WorldContextObject || !Player)
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncActioin_FindSession::FindSession: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_FindSession* Action = NewObject<UAsyncAction_FindSession>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->SessionSettings = SessionSettings;
	Action->MaxResults = MaxResults;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_FindSession::Activate()
{
	Execute_FindSession();
	Super::Activate();
}

void UAsyncAction_FindSession::Execute_FindSession()
{
	if (!WorldContextObject.IsValid() || !Player.IsValid())
	{
		auto HandleFailure = [this]()
		{
			OnFailure.Broadcast(TArray<FString>());
			SetReadyToDestroy();
		};

		if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
		{
			World->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateLambda(HandleFailure));
		}
		else
		{
			HandleFailure();
		}
		return;
	}

	if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
	{
		if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionSearch->MaxSearchResults = MaxResults;
			SessionSearch->bIsLanQuery = false;

			FName TemplateName(NAME_GameSession);
			SessionSearch->QuerySettings.Set(SETTING_SESSION_TEMPLATE_NAME, TemplateName.ToString(), EOnlineComparisonOp::Equals);

			for (const TPair<FName, FEIKAttribute>& SessionSetting : SessionSettings)
			{
				switch (SessionSetting.Value.AttributeType)
				{
				case EEIKAttributeType::Integer:
					SessionSearch->QuerySettings.Set(SessionSetting.Key, SessionSetting.Value.IntValue, EOnlineComparisonOp::Equals);
					break;
				case EEIKAttributeType::Bool:
					SessionSearch->QuerySettings.Set(SessionSetting.Key, SessionSetting.Value.BoolValue, EOnlineComparisonOp::Equals);
					break;
				case EEIKAttributeType::String:
					SessionSearch->QuerySettings.Set(SessionSetting.Key, SessionSetting.Value.StringValue, EOnlineComparisonOp::Equals);
					break;
				default:
					break;
				}
			}

			SessionSearch->PingBucketSize = 50;
			FindSessionDelegateHandle = SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionCompleted);

			ULocalPlayer* LocalPlayer = Player->GetLocalPlayer();
			if (LocalPlayer)
			{
				FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
				if (UserId.IsValid())
				{
					SessionPtr->FindSessions(*UserId, SessionSearch.ToSharedRef());
					bIsSearching = true;

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

void UAsyncAction_FindSession::OnFindSessionCompleted(bool bWasSuccessful)
{
	bIsSearching = false;

	if (!WorldContextObject.IsValid() || bIsCancelled)
	{
		OnFailure.Broadcast(TArray<FString>());
		SetReadyToDestroy();
		return;
	}

	const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	if (OnlineSubsystem)
	{
		const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SessionPtr->OnFindSessionsCompleteDelegates.Remove(FindSessionDelegateHandle);
		}
	}

	if (!bIsCancelled)
	{
		if (bWasSuccessful)
		{
			TArray<FString> Sessions;
			for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
			{
				Sessions.Add(Result.GetSessionIdStr());
			}
			OnSuccess.Broadcast(Sessions);
		}
		else
		{
			OnFailure.Broadcast(TArray<FString>());
		}
	}

	SetReadyToDestroy();
}

void UAsyncAction_FindSession::Cancel()
{
	bIsCancelled = true;

	if (bIsSearching && WorldContextObject.IsValid())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnFindSessionsCompleteDelegates.Remove(FindSessionDelegateHandle);
				SessionPtr->CancelFindSessions();
				bIsSearching = false;
			}
		}
	}

	Super::Cancel();
}
