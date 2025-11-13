// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_CreateLobby.h"

#include "AsyncAction_Helper.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_CreateLobby* UAsyncAction_CreateLobby::CreateLobby(
	UObject* WorldContextObject,
	APlayerController* Player,
	const TMap<FName, FEIKAttribute> LobbySettings,
	const TMap<FName, FEIKAttribute> MemberSettings,
	int32 NumberOfPublicConnections)
{
	if (!IsValid(Player) || !IsValid(WorldContextObject) || NumberOfPublicConnections <= 0)
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_CreateLobby::CreateLobby: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_CreateLobby* Action = NewObject<UAsyncAction_CreateLobby>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->LobbySettings = LobbySettings;
	Action->MemberSettings = MemberSettings;
	Action->NumberOfPublicConnections = NumberOfPublicConnections;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_CreateLobby::Activate()
{
	Execute_CreateLobby();
	Super::Activate();
}

void UAsyncAction_CreateLobby::Execute_CreateLobby()
{
	if (!WorldContextObject.IsValid() || !Player.IsValid())
	{
		auto HandleFailure = [this]()
		{
			OnFailure.Broadcast("");
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
			FOnlineSessionSettings SessionCreationInfo;
			SessionCreationInfo.bIsDedicated = false;
			SessionCreationInfo.bAllowInvites = true;
			SessionCreationInfo.bIsLANMatch = false;
			SessionCreationInfo.NumPublicConnections = NumberOfPublicConnections;
			SessionCreationInfo.NumPrivateConnections = 0;
			SessionCreationInfo.bUseLobbiesIfAvailable = true;
			SessionCreationInfo.bUseLobbiesVoiceChatIfAvailable = true;
			SessionCreationInfo.bUsesPresence = true;
			SessionCreationInfo.bAllowJoinViaPresence = true;
			SessionCreationInfo.bAllowJoinViaPresenceFriendsOnly = true;
			SessionCreationInfo.bShouldAdvertise = true;
			SessionCreationInfo.bAllowJoinInProgress = true;

			FName TemplateName(NAME_GameSession);
			SessionCreationInfo.Set(SETTING_SESSION_TEMPLATE_NAME, TemplateName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);

			for (const TPair<FName, FEIKAttribute>& LobbySetting : LobbySettings)
			{
				FOnlineSessionSetting Setting;
				Setting.AdvertisementType = EOnlineDataAdvertisementType::ViaOnlineService;
				Setting.Data = LobbySetting.Value.ToVariantData();
				SessionCreationInfo.Set(LobbySetting.Key, Setting);
			}

			ULocalPlayer* LocalPlayer = Player->GetLocalPlayer();
			if (LocalPlayer)
			{
				FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
				if (UserId.IsValid())
				{
					CreateLobbyDelegateHandle = SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreateLobbyCompleted);
					SessionPtr->CreateSession(*UserId, NAME_GameSession, SessionCreationInfo);
					bIsCreating = true;

					return;
				}
			}
		}
	}


	auto HandleFailure = [this]()
	{
		OnFailure.Broadcast("");
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

void UAsyncAction_CreateLobby::OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful)
{
	bIsCreating = false;

	if (!WorldContextObject.IsValid() || bIsCancelled)
	{
		OnFailure.Broadcast("");
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
			SessionPtr->OnCreateSessionCompleteDelegates.Remove(CreateLobbyDelegateHandle);
		}
	}


	bool bGotSession = false;
	if (bWasSuccessful)
	{
		if (const FOnlineSession* CurrentSession = SessionPtr->GetNamedSession(SessionName))
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

			OnSuccess.Broadcast(CurrentSession->SessionInfo.Get()->GetSessionId().ToString());
			bGotSession = true;
		}
	}

	if (!bGotSession)
	{
		OnFailure.Broadcast("");
	}

	SetReadyToDestroy();
}

void UAsyncAction_CreateLobby::Cancel()
{
	bIsCancelled = true;

	if (bIsCreating && WorldContextObject.IsValid())
	{
		if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if (const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnCreateSessionCompleteDelegates.Remove(CreateLobbyDelegateHandle);
				SessionPtr->DestroySession(NAME_GameSession);	// TODO 这里是否需要 DestroySession ?
				bIsCreating = false;
			}
		}
	}

	Super::Cancel();
}
