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
		SetReadyToDestroy();
		return ;
	}

	OnFailure.Broadcast(FSessionBasicInfo());
	SetReadyToDestroy();
}
