// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_JoinSession.h"

#include "AsyncAction_Helper.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"

UAsyncAction_JoinSession* UAsyncAction_JoinSession::JoinSession(UObject* WorldContextObject, APlayerController* Player,
	const FString& SessionId, TMap<FName, FEIKAttribute> MemberSettings)
{
	if (!Player || !WorldContextObject || SessionId.IsEmpty())
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_JoinSession::JoinSession: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_JoinSession* Action = NewObject<UAsyncAction_JoinSession>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->SessionId = SessionId;
	Action->MemberSettings = MemberSettings;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_JoinSession::Activate()
{
	Execute_JoinSession();
	Super::Activate();
}

void UAsyncAction_JoinSession::Execute_JoinSession()
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
					SessionPtr->FindSessionById(*UserId.Get(), *SessionNetId, *EmptyId, Delegate);

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

void UAsyncAction_JoinSession::OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful,
	const FOnlineSessionSearchResult& SearchResult)
{
	if (bWasSuccessful && SearchResult.IsValid())
	{
		const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
		const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();

		JoinSessionDelegateHandle = SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		SessionPtr->JoinSession(LocalUserNum, NAME_GameSession, SearchResult);

		return ;
	}

	OnFailure.Broadcast();
	SetReadyToDestroy();
}

void UAsyncAction_JoinSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();
	SessionPtr->OnJoinSessionCompleteDelegates.Remove(JoinSessionDelegateHandle);

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
				UE_LOG(LogCommonSessionAsyncAction, Warning, TEXT("UAsyncAction_CreateSession::OnJoinSessionComplete: Failed to update member attributes"));
			}
		}

		OnSuccess.Broadcast();

		FString MapUrl;
		SessionPtr->GetResolvedConnectString(SessionName, MapUrl);

		UE_LOG(LogCommonSessionAsyncAction, Warning, TEXT("JoinSession Replacing RemoteAddr %s"), *MapUrl);
		MapUrl = "127.0.0.1:7777";
		Player->ClientTravel(MapUrl, TRAVEL_Absolute);

		SetReadyToDestroy();
	}
	else
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
	}
}
