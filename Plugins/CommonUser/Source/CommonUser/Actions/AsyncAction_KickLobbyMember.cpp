// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_KickLobbyMember.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "CommonSessionSubsystemOssv1.h"
#include "Interfaces/OnlineSessionInterface.h"



UAsyncAction_KickLobbyMember* UAsyncAction_KickLobbyMember::KickLobbyMember(
	UObject* WorldContextObject,
	APlayerController* Player,
	FUniqueNetIdRepl TargetUserUniqueId)
{
	if (!Player || !WorldContextObject || !TargetUserUniqueId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UAsyncAction_KickLobbyMember::KickLobbyMember: Invalid parameters"));
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
			UCommonSessionSubsystemOssv1* SessionSubsystem = UGameInstance::GetSubsystem<UCommonSessionSubsystemOssv1>(WorldContextObject->GetWorld()->GetGameInstance());
			check(SessionSubsystem);

			SessionPtr->RemovePlayerFromSession(
				Player->GetLocalPlayer()->GetLocalPlayerIndex(),
				SessionSubsystem->GetLobbyName(),
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
