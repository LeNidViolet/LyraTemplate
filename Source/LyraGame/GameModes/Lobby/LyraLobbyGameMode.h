// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraLobbyPlayerState.h"
#include "GameModes/LyraGameMode.h"
#include "GameModes/LyraUserFacingExperienceDefinition.h"
#include "LyraLobbyGameMode.generated.h"


#define UE_API LYRAGAME_API


UCLASS(MinimalAPI)
class ALyraLobbyGameMode : public ALyraGameMode
{
	GENERATED_BODY()

public:
	UE_API ALyraLobbyGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	UE_API virtual void PostLogin(APlayerController* NewPlayer) override;
	UE_API virtual void Logout(AController* Exiting) override;


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	int32 CountdownToGameStartSeconds = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	int32 MinimumPlayersRequired = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	TSoftObjectPtr<ULyraUserFacingExperienceDefinition> FacingExperience;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	FName FacingExperienceName;

	TMap<ALyraLobbyPlayerState*, FDelegateHandle> DelegateMap;

	void OnPlayerReadyStateChanged(ALyraLobbyPlayerState* PlayerState);

	int32 CurrentCountdownValue = 0;
	bool bCountdownStarted = false;
	void RefreshLobbyCountdown();

	FTimerHandle TimerHandle;
	void StartCountdownTimer();
	void StopCountdownTimer();
	void TimerExpiredFunction();
};

#undef UE_API