// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLobbyGameMode.h"

#include "LyraLobbyGameState.h"
#include "LyraLobbyPlayerState.h"
#include "System/LyraSystemStatics.h"


ALyraLobbyGameMode::ALyraLobbyGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ALyraLobbyGameState::StaticClass();
	PlayerStateClass = ALyraLobbyPlayerState::StaticClass();
}

void ALyraLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALyraLobbyPlayerState* PlayerState = NewPlayer->GetPlayerState<ALyraLobbyPlayerState>();
	check(PlayerState != nullptr);

	ALyraLobbyGameState* GameState = GetGameState<ALyraLobbyGameState>();
	check(GameState != nullptr);

	FString Message = FString::Printf(TEXT("%s joined the lobby"), *PlayerState->GetName());
	GameState->Multicast_BroadcastMessage(Message);


	FDelegateHandle Delegate = PlayerState->OnPlayerReadyStateChangedEvent.AddUObject(this, &ThisClass::OnPlayerReadyStateChanged);
	DelegateMap.Add(PlayerState, Delegate);

	if (DelegateMap.Num() >= MinimumPlayersRequired)
	{
		RefreshLobbyCountdown();
	}
}

void ALyraLobbyGameMode::Logout(AController* Exiting)
{
	ALyraLobbyPlayerState* PlayerState = Exiting->GetPlayerState<ALyraLobbyPlayerState>();
	check(PlayerState != nullptr);

	ALyraLobbyGameState* GameState = GetGameState<ALyraLobbyGameState>();
	check(GameState != nullptr);

	FString Message = FString::Printf(TEXT("%s left the lobby"), *PlayerState->GetName());
	GameState->Multicast_BroadcastMessage(Message);


	if (DelegateMap.Contains(PlayerState))
	{
		FDelegateHandle Delegate = DelegateMap.FindRef(PlayerState);
		if (Delegate.IsValid())
		{
			PlayerState->OnPlayerReadyStateChangedEvent.Remove(Delegate);
		}
		DelegateMap.Remove(PlayerState);
	}

	Super::Logout(Exiting);

	if (DelegateMap.Num() < MinimumPlayersRequired)
	{
		RefreshLobbyCountdown();
	}
}

void ALyraLobbyGameMode::OnPlayerReadyStateChanged(ALyraLobbyPlayerState* PlayerState)
{
	FString Message = FString::Printf(
		TEXT("%s %s"),
		*PlayerState->GetName(),
		PlayerState->IsReady() ? *FString(TEXT("Ready")) : *FString(TEXT("Not Ready")));
	ALyraLobbyGameState* GameState = GetGameState<ALyraLobbyGameState>();
	check(GameState != nullptr);
	GameState->Multicast_BroadcastMessage(Message);

	RefreshLobbyCountdown();
}

void ALyraLobbyGameMode::RefreshLobbyCountdown()
{
	ALyraLobbyGameState* GameState = GetGameState<ALyraLobbyGameState>();
	if (!GameState) return;

	int32 PlayerCountInReadyState = 0;
	for (TPair<ALyraLobbyPlayerState*, FDelegateHandle>& Pair : DelegateMap)
	{
		ALyraLobbyPlayerState* LobbyPS = Pair.Key;
		if (LobbyPS && LobbyPS->IsReady())
		{
			PlayerCountInReadyState++;
		}
	}
	bool bShouldStartCountdown = PlayerCountInReadyState >= MinimumPlayersRequired;

	if (bShouldStartCountdown)
	{
		bShouldStartCountdown = PlayerCountInReadyState == DelegateMap.Num();
	}


	if (bShouldStartCountdown)
	{
		// Should Begin
		if (bCountdownStarted)
		{
			// nothing to do
		}
		else
		{
			FString Message = TEXT("Start Countdown");
			GameState->Multicast_BroadcastMessage(Message);

			CurrentCountdownValue = CountdownToGameStartSeconds;

			StartCountdownTimer();
			bCountdownStarted = true;
		}
	}
	else
	{
		// Should Stop
		if (bCountdownStarted)
		{
			FString Message = TEXT("Stop Countdown");
			GameState->Multicast_BroadcastMessage(Message);

			StopCountdownTimer();
			bCountdownStarted = false;

			CurrentCountdownValue = CountdownToGameStartSeconds;
		}
		else
		{
			// nothing to do
		}
	}
}

void ALyraLobbyGameMode::StartCountdownTimer()
{
	if (!TimerHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		FTimerManager& TimerManager = World->GetTimerManager();

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &ALyraLobbyGameMode::TimerExpiredFunction);

		float DelayTime = 1.0f;
		bool bLooping = true;

		TimerManager.SetTimer(
			TimerHandle,
			TimerDelegate,
			DelayTime,
			bLooping
		);
	}
}

void ALyraLobbyGameMode::StopCountdownTimer()
{
	if (TimerHandle.IsValid())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(TimerHandle);
		TimerHandle = FTimerHandle();
	}
}

void ALyraLobbyGameMode::TimerExpiredFunction()
{
	ALyraLobbyGameState* GameState = GetGameState<ALyraLobbyGameState>();
	if (!GameState)
	{
		StopCountdownTimer();
		return;
	}

	CurrentCountdownValue--;
	if (CurrentCountdownValue <= 0)
	{
		StopCountdownTimer();
		bCountdownStarted = false;

		if (FacingExperience.IsValid())
		{
			// Travel Server
			ULyraSystemStatics::TravelExperience(
				this,
				FacingExperience.Get(),
				TMap<FString, FString>(),
				true,
				ECommonSessionOnlineMode::Online
				);
		}
		else if (!FacingExperienceName.IsNone())
		{
			ULyraSystemStatics::TravelExperienceWithName(
				this,
				FacingExperienceName,
				TMap<FString, FString>(),
				true,
				ECommonSessionOnlineMode::Online
				);
		}
		else
		{
			FString Message = TEXT("Invalid Facing Experience Argument");
			GameState->Multicast_BroadcastMessage(Message);
		}
	}
	else
	{
		GameState->Multicast_BroadcastCountdown(CurrentCountdownValue);
	}
}
