// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_JoinSession.h"

#include "AsyncAction_Helper.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"
#include "CommonUserSettings.h"

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
	if (!WorldContextObject.IsValid() || !Player.IsValid())
	{
		auto HandleFailure = [this]()
		{
			OnFailure.Broadcast();
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
					SessionPtr->FindSessionById(*UserId.Get(), *SessionNetId, *EmptyId, Delegate);
					bIsSearching = true;

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
	bIsSearching = false;

	if (!WorldContextObject.IsValid() || bIsCancelled)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	if (!bIsCancelled)
	{
		if (bWasSuccessful && SearchResult.IsValid())
		{
			const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
			const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();

			JoinSessionDelegateHandle = SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
			SessionPtr->JoinSession(LocalUserNum, NAME_GameSession, SearchResult);
			bIsJoining = true;

			return ;
		}

		OnFailure.Broadcast();
	}

	SetReadyToDestroy();
}

void UAsyncAction_JoinSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bIsJoining = false;

	if (!WorldContextObject.IsValid() || bIsCancelled)
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	IOnlineSessionPtr SessionPtr = nullptr;
	const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld());
	if (OnlineSubsystem)
	{
		SessionPtr = OnlineSubsystem->GetSessionInterface();
		if (SessionPtr)
		{
			SessionPtr->OnJoinSessionCompleteDelegates.Remove(JoinSessionDelegateHandle);
		}
	}

	if (!bIsCancelled)
	{
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

			const UCommonUserSettings* Settings = GetDefault<UCommonUserSettings>();
			if (Settings->bOverrideServerAddress)
			{
				MapUrl = Settings->ServerAddress;
			}

			Player->ClientTravel(MapUrl, TRAVEL_Absolute);
		}
		else
		{
			OnFailure.Broadcast();
		}
	}

	SetReadyToDestroy();
}

void UAsyncAction_JoinSession::Cancel()
{
	bIsCancelled = true;

	if (bIsJoining && WorldContextObject.IsValid())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnJoinSessionCompleteDelegates.Remove(JoinSessionDelegateHandle);
			}
		}
	}

	Super::Cancel();
}
