// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSPlayerController.h"

#include "LyraDSGameMode.h"

ALyraDSPlayerController::ALyraDSPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ALyraDSPlayerController::Server_Request_RemoveMarker_Implementation(
	const FServerRequestRemoveMarkerParameters& Parameters)
{
	if (HasAuthority())
	{
		AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
		ALyraDSGameMode* DSGameMode = Cast<ALyraDSGameMode>(GameModeBase);
		if (DSGameMode)
		{
			DSGameMode->ProcessServerRequestRemoveMarker(this, Parameters);
		}
	}
}

void ALyraDSPlayerController::Server_Request_PlaceMarker_Implementation(
	const FServerRequestPlaceMarkerParameters& Parameters)
{
	if (HasAuthority())
	{
		AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
		ALyraDSGameMode* DSGameMode = Cast<ALyraDSGameMode>(GameModeBase);
		if (DSGameMode)
		{
			DSGameMode->ProcessServerRequestPlaceMarker(this, Parameters);
		}
	}
}
