// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_JoinLobby.h"

#include "AsyncAction_Helper.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_JoinLobby* UAsyncAction_JoinLobby::JoinLobby(
	UObject* WorldContextObject,
	APlayerController* Player,
	const FString& LobbyId,
	TMap<FName, FEIKAttribute> MemberSettings)
{
	if (!Player || !WorldContextObject || LobbyId.IsEmpty())
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_JoinLobby::JoinLobby: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_JoinLobby* Action = NewObject<UAsyncAction_JoinLobby>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->LobbyId = LobbyId;
	Action->MemberSettings = MemberSettings;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_JoinLobby::Activate()
{
	Execute_JoinLobby();
	Super::Activate();
}

void UAsyncAction_JoinLobby::Execute_JoinLobby()
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
					FUniqueNetIdPtr LobbyNetId = FUniqueNetIdString::Create(LobbyId, FName(TEXT("CustomInType")));
					FUniqueNetIdPtr EmptyId = FUniqueNetIdString::Create("", FName(TEXT("EmptyInType")));

					FOnSingleSessionResultCompleteDelegate Delegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &ThisClass::OnSingleSessionResultComplete);
					SessionPtr->FindSessionById(*UserId.Get(), *LobbyNetId, *EmptyId, Delegate);

					return;
				}
			}
		}
	}

	auto HandleFailure = [this]()
	{
		OnFailure.Broadcast();
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

void UAsyncAction_JoinLobby::OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful,
	const FOnlineSessionSearchResult& SearchResult)
{
	if (bWasSuccessful && SearchResult.IsValid())
	{
		const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
		const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();

		JoinLobbyDelegateHandle = SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		SessionPtr->JoinSession(LocalUserNum, NAME_GameSession, SearchResult);

		return ;
	}

	OnFailure.Broadcast();
	SetReadyToDestroy();
}

void UAsyncAction_JoinLobby::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	SessionPtr->OnJoinSessionCompleteDelegates.Remove(JoinLobbyDelegateHandle);

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (MemberSettings.Num() > 0)
		{
			bool bOk = AsyncAction_Helper::UpdateMemberAttributes(
				WorldContextObject.Get(),
				Player->GetLocalPlayer()->GetLocalPlayerIndex(),
				SessionName,
				MemberSettings);
			if (!bOk)
			{
				UE_LOG(LogCommonSessionAsyncAction, Warning, TEXT("UAsyncAction_CreateLobby::OnCreateLobbyCompleted: Failed to update member attributes"));
			}
		}

		OnSuccess.Broadcast();

		FString MapUrl;
		SessionPtr->GetResolvedConnectString(SessionName, MapUrl);
		Player->ClientTravel(MapUrl, TRAVEL_Absolute);

		SetReadyToDestroy();
	}
	else
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
	}
}
