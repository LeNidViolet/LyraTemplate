// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSGameMode.h"

#include "LyraDSGameState.h"
#include "LyraDSPlayerController.h"
#include "LyraDSPlayerState.h"
#include "LyraLogChannels.h"
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
		check(GameState);

		ALyraDSPlayerState* PlayerState = NewPlayer->GetPlayerState<ALyraDSPlayerState>();
		check(PlayerState);

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
		check(GameState);

		ALyraDSPlayerState* PlayerState = Exiting->GetPlayerState<ALyraDSPlayerState>();
		check(PlayerState);

		FUniqueNetIdRepl UniqueIdRepl = PlayerState->GetUniqueId();
		if (UniqueIdRepl.IsValid())
		{
			const FUniqueNetId& UniqueId = *UniqueIdRepl.GetUniqueNetId();
			UE_LOG(LogLyra, Log, TEXT("Player %s Logout with ID: %s"),
				*PlayerState->GetPlayerName(),
				*UniqueId.ToString())
		}

		// 移除该玩家的所有标记点
		PlayerState->RemoveAllWorldMarkers();


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

#undef LOCTEXT_NAMESPACE
