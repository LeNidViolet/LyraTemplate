// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerController.h"
#include "LyraDSPlayerController.generated.h"


#define UE_API LYRAGAME_API

struct FServerRequestPlaceMarkerParameters;
struct FServerRequestRemoveMarkerParameters;

/**
 *
 */
UCLASS(MinimalAPI)
class ALyraDSPlayerController : public ALyraPlayerController
{
	GENERATED_BODY()

public:
	UE_API ALyraDSPlayerController(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	UE_API void Server_Request_PlaceMarker(const FServerRequestPlaceMarkerParameters& Parameters);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	UE_API void Server_Request_RemoveMarker(const FServerRequestRemoveMarkerParameters& Parameters);

};

#undef UE_API