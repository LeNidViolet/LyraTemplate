// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "LobbyGameState.h"
#include "LobbyPlayerState.h"



void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALobbyPlayerState* PlayerState = NewPlayer->GetPlayerState<ALobbyPlayerState>();
	check(PlayerState != nullptr);

	ALobbyGameState* GameState = GetGameState<ALobbyGameState>();
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

void ALobbyGameMode::Logout(AController* Exiting)
{
	ALobbyPlayerState* PlayerState = Exiting->GetPlayerState<ALobbyPlayerState>();
	check(PlayerState != nullptr);

	ALobbyGameState* GameState = GetGameState<ALobbyGameState>();
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

void ALobbyGameMode::OnPlayerReadyStateChanged(ALobbyPlayerState* PlayerState)
{
	FString Message = FString::Printf(
		TEXT("%s %s"),
		*PlayerState->GetName(),
		PlayerState->IsReady() ? *FString(TEXT("Ready")) : *FString(TEXT("Not Ready")));
	ALobbyGameState* GameState = GetGameState<ALobbyGameState>();
	check(GameState != nullptr);
	GameState->Multicast_BroadcastMessage(Message);

	RefreshLobbyCountdown();
}

void ALobbyGameMode::RefreshLobbyCountdown()
{
	ALobbyGameState* GameState = GetGameState<ALobbyGameState>();
	if (!GameState) return;

	UE_LOG(LogTemp, Display, TEXT("Refreshing Lobby countdown, PlayerState[%d] DelegateMap[%d]"),
		GameState->PlayerArray.Num(),
		DelegateMap.Num()
		);

	int32 PlayerCountInReadyState = 0;
	for (TPair<ALobbyPlayerState*, FDelegateHandle>& Pair : DelegateMap)
	{
		ALobbyPlayerState* LobbyPS = Pair.Key;
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

void ALobbyGameMode::StartCountdownTimer()
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
		TimerDelegate.BindUObject(this, &ALobbyGameMode::TimerExpiredFunction);

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

void ALobbyGameMode::StopCountdownTimer()
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

void ALobbyGameMode::TimerExpiredFunction()
{
	ALobbyGameState* GameState = GetGameState<ALobbyGameState>();
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
			TravelExperience(
				FacingExperience.Get(),
				TMap<FString, FString>(),
				true,
				ECommonSessionOnlineMode::Online
				);
		}
		else if (!FacingExperienceName.IsNone())
		{
			TravelExperienceWithName(
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
