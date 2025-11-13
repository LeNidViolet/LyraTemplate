// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_JoinLobby.h"

#include "AsyncAction_Helper.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"
#include "CommonUserSettings.h"


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
					FUniqueNetIdPtr LobbyNetId = FUniqueNetIdString::Create(LobbyId, FName(TEXT("CustomInType")));
					FUniqueNetIdPtr EmptyId = FUniqueNetIdString::Create("", FName(TEXT("EmptyInType")));

					FOnSingleSessionResultCompleteDelegate Delegate = FOnSingleSessionResultCompleteDelegate::CreateUObject(this, &ThisClass::OnSingleSessionResultComplete);
					SessionPtr->FindSessionById(*UserId.Get(), *LobbyNetId, *EmptyId, Delegate);
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

void UAsyncAction_JoinLobby::OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful,
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

			JoinLobbyDelegateHandle = SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinSessionComplete);
			SessionPtr->JoinSession(LocalUserNum, NAME_GameSession, SearchResult);
			bIsJoining = true;

			return ;
		}
		OnFailure.Broadcast();
	}

	SetReadyToDestroy();
}

void UAsyncAction_JoinLobby::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bIsJoining = false;

	if (!WorldContextObject.IsValid() || !Player.IsValid() || bIsCancelled)
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
			SessionPtr->OnJoinSessionCompleteDelegates.Remove(JoinLobbyDelegateHandle);
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
					UE_LOG(LogCommonSessionAsyncAction, Warning, TEXT("UAsyncAction_CreateLobby::OnCreateLobbyCompleted: Failed to update member attributes"));
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

void UAsyncAction_JoinLobby::Cancel()
{
	bIsCancelled = true;

	if (bIsJoining && WorldContextObject.IsValid())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnJoinSessionCompleteDelegates.Remove(JoinLobbyDelegateHandle);
			}
		}
	}

	Super::Cancel();
}
