// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSPlayerState.h"

#include "Net/UnrealNetwork.h"


ALyraDSPlayerState::ALyraDSPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}



void ALyraDSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALyraDSPlayerState, PlayerColor);
}

void ALyraDSPlayerState::OnRep_PlayerColor()
{
	OnPlayerColorChanged.Broadcast(PlayerColor);
}
