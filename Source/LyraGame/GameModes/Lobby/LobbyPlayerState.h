// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerState.h"
#include "CommonUserSubsystem.h"
#include "LobbyPlayerState.generated.h"

#define UE_API LYRAGAME_API

class ALobbyPlayerState;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerReadyStateChanged, ALobbyPlayerState* /*PlayerState*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReadyStateChanged_Dynamic, ALobbyPlayerState*, PlayerState);


UCLASS(MinimalAPI)
class ALobbyPlayerState : public ALyraPlayerState
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	UE_API void RPC_SetReady(bool bNewReadyState);

	UFUNCTION(Server, Reliable)
	UE_API void RPC_SetDisplayName(FName DisplayName);

	UFUNCTION(BlueprintPure, Category="Player State")
	UE_API bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintPure, Category="Player State")
	UE_API FName GetDisplayName() const { return PlayerDisplayName; }

	FOnPlayerReadyStateChanged OnPlayerReadyStateChangedEvent;
	UPROPERTY(BlueprintAssignable, Category="Player State")
	FOnPlayerReadyStateChanged_Dynamic K2_OnPlayerReadyStateChangedEvent;

private:
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_IsReady)
	bool bIsReady = false;

	UPROPERTY(Replicated)
	FName PlayerDisplayName = FName(TEXT("UNKNOW"));

	UFUNCTION()
	void OnRep_IsReady();
};

#undef UE_API
