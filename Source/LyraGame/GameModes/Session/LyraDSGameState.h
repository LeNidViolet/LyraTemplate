// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameState.h"
#include "LyraDSGameState.generated.h"

#define UE_API LYRAGAME_API

struct FLyraNotificationMessage;
struct FOnSessionParticipantEventParameters;


UCLASS(MinimalAPI)
class ALyraDSGameState : public ALyraGameState
{
	GENERATED_BODY()

public:

	UE_API ALyraDSGameState(const FObjectInitializer& ObjectInitializer);

	UE_API void Broadcast_SessionParticipantEvent(const FOnSessionParticipantEventParameters& Parameters);


	// Send a notify message that all clients will (probably) get
	// (use only for client notifications like eliminations, server join messages, etc... that can handle being lost)
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Lyra|GameState")
	UE_API void Multicast_MessageToClients(const FLyraNotificationMessage& Message);

	// Send a notify message that all clients will be guaranteed to get
	// (use only for client notifications that cannot handle being lost)
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Lyra|GameState")
	UE_API void Multicast_ReliableMessageToClients(const FLyraNotificationMessage& Message);

protected:
	UE_API virtual void BeginPlay() override;
	UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

#undef UE_API