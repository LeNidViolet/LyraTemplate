// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLobbyGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "UI/Misc/LyraToastMessage.h"






void ALyraLobbyGameState::Multicast_BroadcastMessage_Implementation(const FString& Message)
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
			LyraGameplayTags::ToastMessage_Lobby_MemberEvent,
			message
			);
	}
}


// void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// }

void ALyraLobbyGameState::Multicast_BroadcastCountdown_Implementation(int32 CountdownValue)
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
			LyraGameplayTags::ToastMessage_Lobby_Countdown,
			message
			);
	}
}
