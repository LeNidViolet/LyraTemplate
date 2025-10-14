// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraToastMessageSubsystem.h"

#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"


UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Toast_Message, "ToastMessage")
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Toast_Message_Lobby_MemberEvent, "ToastMessage.Lobby.MemberEvent")
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Toast_Message_Lobby_Countdown, "ToastMessage.Lobby.Countdown")


void ULyraToastMessageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	ToastMessageListener = MessageSystem.RegisterListener<FLyraToastMessage>(
		TAG_Toast_Message,
		[this](FGameplayTag Channel, const FLyraToastMessage& Payload)
		{
			if (this)
			{
				this->HandleToastMessage(Channel, Payload);
			}
		},
		EGameplayMessageMatch::PartialMatch
		);
}

void ULyraToastMessageSubsystem::Deinitialize()
{
	if (ToastMessageListener.IsValid())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.UnregisterListener(ToastMessageListener);
	}

	Super::Deinitialize();
}

void ULyraToastMessageSubsystem::HandleToastMessage(FGameplayTag Channel, const FLyraToastMessage& Payload)
{
	if (Channel.MatchesTagExact(TAG_Toast_Message_Lobby_MemberEvent))
	{
		OnToastLobbyMemberEventReveived.Broadcast(Channel, Payload);
	}
	else if (Channel.MatchesTagExact(TAG_Toast_Message_Lobby_Countdown))
	{
		OnToastLobbyCountdownReveived.Broadcast(Channel, Payload);
	}
	else
	{
		OnToastMessageReceived.Broadcast(Channel, Payload);
	}
}
