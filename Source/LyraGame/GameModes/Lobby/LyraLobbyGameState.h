// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameState.h"
#include "LyraLobbyGameState.generated.h"

#define UE_API LYRAGAME_API

/**
 *
 */
UCLASS(MinimalAPI)
class ALyraLobbyGameState : public ALyraGameState
{
	GENERATED_BODY()

public:

	UFUNCTION(NetMulticast, Reliable)
	UE_API void Multicast_BroadcastMessage(const FString& Message);

	UFUNCTION(NetMulticast, Reliable)
	UE_API void Multicast_BroadcastCountdown(int32 CountdownValue);
};

#undef UE_API