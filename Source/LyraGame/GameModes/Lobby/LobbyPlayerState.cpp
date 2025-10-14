// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"

#include "Engine/World.h"
#include "CommonUserSubsystem.h"
#include "CommonSessionSubsystemOssv1.h"
#include "Net/UnrealNetwork.h"


void ALobbyPlayerState::RPC_SetReady_Implementation(bool bNewReadyState)
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

void ALobbyPlayerState::RPC_SetDisplayName_Implementation(FName DisplayName)
{
	if (DisplayName != PlayerDisplayName)
	{
		PlayerDisplayName = DisplayName;
	}
}


void ALobbyPlayerState::OnRep_IsReady()
{
	K2_OnPlayerReadyStateChangedEvent.Broadcast(this);
}


void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	UCommonSessionSubsystemOssv1* SessionSubsystemOssv1 = GetGameInstance()->GetSubsystem<UCommonSessionSubsystemOssv1>();
	check(SessionSubsystemOssv1);
	if (SessionSubsystemOssv1)
	{
		SessionSubsystemOssv1->LogNetEnvironment(this, GetPlayerController());
	}

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


void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, bIsReady);
	DOREPLIFETIME(ALobbyPlayerState, PlayerDisplayName);
}
