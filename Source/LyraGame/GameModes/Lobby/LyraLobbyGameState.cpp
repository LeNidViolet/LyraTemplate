// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLobbyGameState.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "UI/Misc/LyraToastMessage.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLobbyGameState)



void ALyraLobbyGameState::Multicast_BroadcastMessage_Implementation(const FString& Message)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		FLyraToastMessage message;
		message.StringValue = Message;

		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			MessageSubsystem->BroadcastMessage(
				LyraGameplayTags::Gameplay_Message_Lobby_MemberEvent,
				message
				);
		}
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

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		FLyraToastMessage message;
		message.NumberValue = CountdownValue;

		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			MessageSubsystem->BroadcastMessage(
			LyraGameplayTags::Gameplay_Message_Lobby_Countdown,
			message
			);
		}
	}
}
