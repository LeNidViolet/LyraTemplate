// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_GetLobbyBasicInfo.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_GetLobbyBasicInfo* UAsyncAction_GetLobbyBasicInfo::GetLobbyBasicInfo(
	UObject* WorldContextObject,
	APlayerController* Player,
	const FString& LobbyId,
	TArray<FName> ExposeAttributes)
{
	if (!Player || !WorldContextObject || LobbyId.IsEmpty())
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_GetLobbyBasicInfo::GetLobbyBasicInfo: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_GetLobbyBasicInfo* Action = NewObject<UAsyncAction_GetLobbyBasicInfo>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->LobbyId = LobbyId;
	Action->ExposeAttributes = ExposeAttributes;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_GetLobbyBasicInfo::Activate()
{
	Execute_GetLobbyBasicInfo();
	Super::Activate();
}

void UAsyncAction_GetLobbyBasicInfo::Execute_GetLobbyBasicInfo()
{
	if (!WorldContextObject.IsValid() || !Player.IsValid())
	{
		auto HandleFailure = [this]()
		{
			OnFailure.Broadcast(FLobbyBasicInfo());
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
					FUniqueNetIdPtr LobbyNetId = FUniqueNetIdString::Create(LobbyId, FName(TEXT("CustomInType")));
					FUniqueNetIdPtr EmptyId = FUniqueNetIdString::Create("", FName(TEXT("EmptyInType")));

					FOnSingleSessionResultCompleteDelegate Delegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &ThisClass::OnSingleSessionResultComplete);
					SessionPtr->FindSessionById(*UserId, *LobbyNetId, *EmptyId, Delegate);
					bIsSearching = true;

					return ;
				}
			}
		}
	}

	auto HandleFailure = [this]()
	{
		OnFailure.Broadcast(FLobbyBasicInfo());
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

void UAsyncAction_GetLobbyBasicInfo::OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful,
	const FOnlineSessionSearchResult& SearchResult)
{
	bIsSearching = false;

	if (!WorldContextObject.IsValid() || bIsCancelled)
	{
		OnFailure.Broadcast(FLobbyBasicInfo());
		SetReadyToDestroy();
		return;
	}

	if (!bIsCancelled)
	{
		if (bWasSuccessful && SearchResult.IsValid())
		{
			FLobbyBasicInfo Result;
			Result.LobbyId = SearchResult.GetSessionIdStr();
			if (SearchResult.Session.OwningUserId.IsValid())
			{
				Result.OwnerId = SearchResult.Session.OwningUserId->ToString();
			}
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
			OnFailure.Broadcast(FLobbyBasicInfo());
		}
	}

	SetReadyToDestroy();
}

void UAsyncAction_GetLobbyBasicInfo::Cancel()
{
	bIsCancelled = true;

	if (bIsSearching && WorldContextObject.IsValid())
	{
		if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				// nothing to cancel in this case
			}
		}
	}

	Super::Cancel();
}
