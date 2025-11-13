// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLobbyPlayerState.h"

#include "Engine/World.h"
#include "CommonUserSubsystem.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLobbyPlayerState)


void ALyraLobbyPlayerState::RPC_SetReady_Implementation(bool bNewReadyState)
{
	if (bIsReady != bNewReadyState)
	{
		// Notify Clients
		bIsReady = bNewReadyState;

		// Notify GameMode
		OnPlayerReadyStateChangedEvent.Broadcast(this);

		OnRep_IsReady();
	}
}

void ALyraLobbyPlayerState::RPC_SetDisplayName_Implementation(FName DisplayName)
{
	if (DisplayName != PlayerDisplayName)
	{
		PlayerDisplayName = DisplayName;
	}
}


void ALyraLobbyPlayerState::OnRep_IsReady()
{
	K2_OnPlayerReadyStateChangedEvent.Broadcast(this);
}


void ALyraLobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (GetPlayerController())
	{
		UCommonUserSubsystem* UserSubsystem = GetGameInstance()->GetSubsystem<UCommonUserSubsystem>();
		if (UserSubsystem)
		{
			const UCommonUserInfo* UserInfo = UserSubsystem->GetUserInfoForLocalPlayerIndex(0);
			if (UserInfo)
			{
				RPC_SetDisplayName(FName(*UserInfo->GetNickname()));
			}
		}
	}
}


void ALyraLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALyraLobbyPlayerState, bIsReady);
	DOREPLIFETIME(ALyraLobbyPlayerState, PlayerDisplayName);
}
