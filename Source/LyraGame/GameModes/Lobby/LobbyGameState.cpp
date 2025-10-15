// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "NativeGameplayTags.h"
#include "UI/Misc/LyraToastMessage.h"


UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Toast_Message_Lobby_MemberEvent, "ToastMessage.Lobby.MemberEvent")
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Toast_Message_Lobby_Countdown, "ToastMessage.Lobby.Countdown")



void ALobbyGameState::Multicast_BroadcastMessage_Implementation(const FString& Message)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FLyraToastMessage message;
		message.StringValue = Message;

		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			TAG_Toast_Message_Lobby_MemberEvent,
			message
			);
	}
}


// void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// }

void ALobbyGameState::Multicast_BroadcastCountdown_Implementation(int32 CountdownValue)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		FLyraToastMessage message;
		message.NumberValue = CountdownValue;

		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			TAG_Toast_Message_Lobby_Countdown,
			message
			);
	}
}
