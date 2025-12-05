// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSGameState.h"

#include "LyraGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/LyraNotificationMessage.h"
#include "Messages/LyraNotificationMessage_Participant.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraDSGameState)

//=============================================================================
// ALyraDSGameState
//=============================================================================

ALyraDSGameState::ALyraDSGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ALyraDSGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ALyraDSGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

// void ALyraDSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// }

void ALyraDSGameState::Broadcast_SessionParticipantEvent(const FOnSessionParticipantEventParameters& Parameters)
{
	FLyraNotificationMessage Message;
	Message.TargetChannel = LyraGameplayTags::Gameplay_Message_Session_MemberEvent;
	Message.PayloadData.InitializeAs<FOnSessionParticipantEventParameters>(Parameters);

	Multicast_MessageToClients(Message);
}

void ALyraDSGameState::Multicast_MessageToClients_Implementation(const FLyraNotificationMessage& Message)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
			if (MessageSubsystem)
			{
				MessageSubsystem->BroadcastMessage(Message.TargetChannel, Message.PayloadData);
			}
		}
	}
}

void ALyraDSGameState::Multicast_ReliableMessageToClients_Implementation(const FLyraNotificationMessage& Message)
{
	Multicast_MessageToClients_Implementation(Message);
}
