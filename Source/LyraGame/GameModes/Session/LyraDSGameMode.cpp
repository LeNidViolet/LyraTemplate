// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSGameMode.h"

#include "LyraDSGameState.h"
#include "LyraDSPlayerController.h"
#include "LyraDSPlayerState.h"
#include "LyraLogChannels.h"
#include "Messages/LyraNotificationMessage_Marker.h"
#include "Messages/LyraNotificationMessage_Participant.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/LyraExperienceManagerComponent.h"

#define LOCTEXT_NAMESPACE "Lyra"

ALyraDSGameMode::ALyraDSGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ALyraDSGameState::StaticClass();
	PlayerStateClass = ALyraDSPlayerState::StaticClass();
	PlayerControllerClass = ALyraDSPlayerController::StaticClass();

	AvailablePlayerColors.Add(FColorList::Scarlet);
	AvailablePlayerColors.Add(FColorList::SlateBlue);
	AvailablePlayerColors.Add(FColorList::SpringGreen);
	AvailablePlayerColors.Add(FColorList::Orange);
}

void ALyraDSGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (HasAuthority())
	{
		ALyraDSGameState* GameState = GetGameState<ALyraDSGameState>();
		check(GameState != nullptr);

		ALyraDSPlayerState* PlayerState = NewPlayer->GetPlayerState<ALyraDSPlayerState>();
		check(PlayerState != nullptr);

		int32 ColorIndex = NextColorIndex++ % AvailablePlayerColors.Num();
		FColor AssignedColor = AvailablePlayerColors[ColorIndex];

		PlayerState->PlayerColor = AssignedColor;

		FUniqueNetIdRepl UniqueIdRepl = PlayerState->GetUniqueId();
		if (!UniqueIdRepl.IsValid())
		{
			UE_LOG(LogLyra, Warning, TEXT("Player %s has invalid UniqueId"), *PlayerState->GetPlayerName());

			NewPlayer->ClientReturnToMainMenuWithTextReason(FText::FromString("Invalid UniqueId"));
			return;
		}

		const FUniqueNetId& UniqueId = *UniqueIdRepl.GetUniqueNetId();
		UE_LOG(LogLyra, Log, TEXT("Player %s logged in with ID: %s"),
			*PlayerState->GetPlayerName(),
			*UniqueId.ToString())

		FOnSessionParticipantEventParameters Parameters;
		Parameters.EventType = EOnSessionParticipantEventType::Join;
		Parameters.ParticipantName = *PlayerState->GetPlayerName();
		GameState->Broadcast_SessionParticipantEvent(Parameters);
	}
}

void ALyraDSGameMode::Logout(AController* Exiting)
{
	if (HasAuthority())
	{
		ALyraDSGameState* GameState = GetGameState<ALyraDSGameState>();
		check(GameState != nullptr);

		APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
		check(PlayerState != nullptr);

		FUniqueNetIdRepl UniqueIdRepl = PlayerState->GetUniqueId();
		if (UniqueIdRepl.IsValid())
		{
			const FUniqueNetId& UniqueId = *UniqueIdRepl.GetUniqueNetId();
			UE_LOG(LogLyra, Log, TEXT("Player %s Logout with ID: %s"),
				*PlayerState->GetPlayerName(),
				*UniqueId.ToString())
		}

		FOnSessionParticipantEventParameters Parameters;
		Parameters.EventType = EOnSessionParticipantEventType::Left;
		Parameters.ParticipantName = *PlayerState->GetPlayerName();
		GameState->Broadcast_SessionParticipantEvent(Parameters);
	}

	Super::Logout(Exiting);
}

void ALyraDSGameMode::BeginPlay()
{
	Super::BeginPlay();
}






void ALyraDSGameMode::ProcessServerRequestPlaceMarker(
	ALyraDSPlayerController* PlayerController,
	const FServerRequestPlaceMarkerParameters& Parameters)
{
	ALyraDSGameState* GameState = GetGameState<ALyraDSGameState>();
	check(GameState != nullptr);

	ALyraDSPlayerState* PlayerState = PlayerController->GetPlayerState<ALyraDSPlayerState>();
	check(PlayerState != nullptr);

	// Remove Existing Marker(s) Belong this Player
	int32 FoundIndex = PlayerMarkers.IndexOfByPredicate([PlayerState](const TSharedPtr<FPlayerMarkerData>& Marker)
		{
			return Marker.IsValid() && Marker->MarkerOwner == PlayerState;
		});

	if (FoundIndex != INDEX_NONE)
	{
		TSharedPtr<FPlayerMarkerData> MarkerToRemove = PlayerMarkers[FoundIndex];

		FOnRemoveMarkerParameters RemoveMarkerParameters;
		RemoveMarkerParameters.MarkerId = MarkerToRemove->MarkerId;
		RemoveMarkerParameters.PlayerState = PlayerState;
		GameState->Broadcast_RemoveMarkerEvent(RemoveMarkerParameters);

		PlayerMarkers.RemoveAt(FoundIndex);
	}


	TSharedPtr<FPlayerMarkerData> Marker = MakeShared<FPlayerMarkerData>();
	Marker->MarkerOwner = PlayerState;
	Marker->MarkerId = FGuid::NewGuid();
	Marker->MarkerLocation = Parameters.Location;
	PlayerMarkers.Add(Marker);

	FOnPlaceMarkerParameters PlaceMarkerParameters;
	PlaceMarkerParameters.Location = Marker->MarkerLocation;
	PlaceMarkerParameters.MarkerId = Marker->MarkerId;
	PlaceMarkerParameters.PlayerState = PlayerState;

	GameState->Broadcast_PlaceMarkerEvent(PlaceMarkerParameters);
}

void ALyraDSGameMode::ProcessServerRequestRemoveMarker(
	ALyraDSPlayerController* PlayerController,
	const FServerRequestRemoveMarkerParameters& Parameters)
{
	ALyraDSGameState* GameState = GetGameState<ALyraDSGameState>();
	check(GameState != nullptr);

	ALyraDSPlayerState* PlayerState = PlayerController->GetPlayerState<ALyraDSPlayerState>();
	check(PlayerState != nullptr);

	int32 FoundIndex = PlayerMarkers.IndexOfByPredicate([Parameters, PlayerState](const TSharedPtr<FPlayerMarkerData>& Marker)
		{
			return Marker.IsValid() && Marker->MarkerId == Parameters.MarkerId && Marker->MarkerOwner == PlayerState;
		});

	if (FoundIndex != INDEX_NONE)
	{
		TSharedPtr<FPlayerMarkerData> MarkerToRemove = PlayerMarkers[FoundIndex];

		FOnRemoveMarkerParameters RemoveMarkerParameters;
		RemoveMarkerParameters.MarkerId = MarkerToRemove->MarkerId;
		RemoveMarkerParameters.PlayerState = PlayerState;
		GameState->Broadcast_RemoveMarkerEvent(RemoveMarkerParameters);

		PlayerMarkers.RemoveAt(FoundIndex);
	}
}


#undef LOCTEXT_NAMESPACE
