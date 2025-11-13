// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_GetSessionBasicInfo.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_GetSessionBasicInfo* UAsyncAction_GetSessionBasicInfo::GetSessionBasicInfo(UObject* WorldContextObject,
	APlayerController* Player, const FString& SessionId, TArray<FName> ExposeAttributes)
{
	if (!Player || !WorldContextObject || SessionId.IsEmpty())
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_GetSessionBasicInfo::GetSessionBasicInfo: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_GetSessionBasicInfo* Action = NewObject<UAsyncAction_GetSessionBasicInfo>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->SessionId = SessionId;
	Action->ExposeAttributes = ExposeAttributes;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_GetSessionBasicInfo::Activate()
{
	Execute_GetSessionBasicInfo();
	Super::Activate();
}

void UAsyncAction_GetSessionBasicInfo::Execute_GetSessionBasicInfo()
{
	if (!WorldContextObject.IsValid() || !Player.IsValid())
	{
		auto HandleFailure = [this]()
		{
			OnFailure.Broadcast(FSessionBasicInfo());
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
			ULocalPlayer* LocalPlayer = Player->GetLocalPlayer();
			if (LocalPlayer)
			{
				FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
				if (UserId.IsValid())
				{
					FUniqueNetIdPtr SessionNetId = FUniqueNetIdString::Create(SessionId, FName(TEXT("CustomInType")));
					FUniqueNetIdPtr EmptyId = FUniqueNetIdString::Create("", FName(TEXT("EmptyInType")));

					FOnSingleSessionResultCompleteDelegate Delegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &ThisClass::OnSingleSessionResultComplete);
					SessionPtr->FindSessionById(*UserId, *SessionNetId, *EmptyId, Delegate);
					bIsSearching = true;

					return ;
				}
			}
		}
	}

	auto HandleFailure = [this]()
	{
		OnFailure.Broadcast(FSessionBasicInfo());
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

void UAsyncAction_GetSessionBasicInfo::OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful,
	const FOnlineSessionSearchResult& SearchResult)
{
	bIsSearching = false;

	if (!WorldContextObject.IsValid() || bIsCancelled)
	{
		OnFailure.Broadcast(FSessionBasicInfo());
		SetReadyToDestroy();
		return;
	}

	if (!bIsCancelled)
	{
		if (bWasSuccessful && SearchResult.IsValid())
		{
			FSessionBasicInfo Result;
			Result.SessionId = SearchResult.GetSessionIdStr();

			Result.PingInMs = SearchResult.PingInMs;
			Result.NumPublicConnections = SearchResult.Session.SessionSettings.NumPublicConnections;
			Result.NumOpenPublicConnections = SearchResult.Session.NumOpenPublicConnections;

			for (const TTuple<FName, FOnlineSessionSetting>& Setting : SearchResult.Session.SessionSettings.Settings)
			{
				if (ExposeAttributes.Contains(Setting.Key.ToString()))
				{
					FEIKAttribute Attribute(Setting.Value.Data);
					Result.Attributes.Add(Setting.Key, Attribute);
				}
			}

			OnSuccess.Broadcast(Result);
		}
		else
		{
			OnFailure.Broadcast(FSessionBasicInfo());
		}
	}

	SetReadyToDestroy();
}

void UAsyncAction_GetSessionBasicInfo::Cancel()
{
	bIsCancelled = true;

	if (bIsSearching && WorldContextObject.IsValid())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				// nothing to cancel for single session query
			}
		}
	}

	Super::Cancel();
}
