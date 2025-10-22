// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_KickLobbyMember.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_KickLobbyMember* UAsyncAction_KickLobbyMember::KickLobbyMember(
	UObject* WorldContextObject,
	APlayerController* Player,
	FUniqueNetIdRepl TargetUserUniqueId)
{
	if (!Player || !WorldContextObject || !TargetUserUniqueId.IsValid())
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_KickLobbyMember::KickLobbyMember: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_KickLobbyMember* Action = NewObject<UAsyncAction_KickLobbyMember>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->TargetUserUniqueId = TargetUserUniqueId;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_KickLobbyMember::Activate()
{
	Execute_KickLobbyMember();
	Super::Activate();
}

void UAsyncAction_KickLobbyMember::Execute_KickLobbyMember()
{
	if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
	{
		if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->RemovePlayerFromSession(
				Player->GetLocalPlayer()->GetLocalPlayerIndex(),
				NAME_GameSession,
				*TargetUserUniqueId.GetUniqueNetId()
				);
		}
	}

	auto HandleResult = [this]()
	{
		OnSuccess.Broadcast();
		SetReadyToDestroy();
	};

	UWorld* World = WorldContextObject->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(HandleResult));
	}
	else
	{
		HandleResult();
	}
}
