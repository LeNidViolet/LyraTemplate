// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSGameState.h"

#include "LyraGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/LyraNotificationMessage.h"
#include "Messages/LyraNotificationMessage_Participant.h"
#include "Messages/LyraNotificationMessage_Marker.h"
#include "Net/UnrealNetwork.h"


ALyraDSGameState::ALyraDSGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ReplicatedMessages.SetOwner(this);
}

void ALyraDSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALyraDSGameState, ReplicatedMessages);
}

void ALyraDSGameState::Broadcast_SessionParticipantEvent(const FOnSessionParticipantEventParameters& Parameters)
{
	FLyraNotificationMessage Message;
	Message.TargetChannel = LyraGameplayTags::ToastMessage_Session_MemberEvent;
	Message.PayloadData = FInstancedStruct::Make<FOnSessionParticipantEventParameters>(Parameters);
	//Message.PayloadData.InitializeAs(FOnSessionParticipantEventParameters::StaticStruct(), reinterpret_cast<const uint8*>(&Parameters));

	Multicast_MessageToClients(Message);
}

void ALyraDSGameState::Broadcast_PlaceMarkerEvent(const FOnPlaceMarkerParameters& Parameters)
{
	FLyraNotificationMessage Message;
	Message.TargetChannel = LyraGameplayTags::Gameplay_Message_Marker_Add;
	Message.PayloadData = FInstancedStruct::Make<FOnPlaceMarkerParameters>(Parameters);
	//Message.PayloadData.InitializeAs(FOnPlaceMarkerParameters::StaticStruct(), reinterpret_cast<const uint8*>(&Parameters));

	Multicast_MessageToClients(Message);
}

void ALyraDSGameState::Broadcast_RemoveMarkerEvent(const FOnRemoveMarkerParameters& Parameters)
{
	FLyraNotificationMessage Message;
	Message.TargetChannel = LyraGameplayTags::Gameplay_Message_Marker_Remove;
	Message.PayloadData = FInstancedStruct::Make<FOnRemoveMarkerParameters>(Parameters);
	//Message.PayloadData.InitializeAs(FOnRemoveMarkerParameters::StaticStruct(), reinterpret_cast<const uint8*>(&Parameters));

	Multicast_MessageToClients(Message);
}

void ALyraDSGameState::Multicast_MessageToClients_Implementation(const FLyraNotificationMessage& Message)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.TargetChannel, Message.PayloadData);
	}
}

void ALyraDSGameState::Multicast_ReliableMessageToClients_Implementation(const FLyraNotificationMessage& Message)
{
	Multicast_MessageToClients_Implementation(Message);
}

