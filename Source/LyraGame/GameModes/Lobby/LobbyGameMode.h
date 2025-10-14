// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LobbyPlayerState.h"
#include "GameModes/LyraGameMode.h"
#include "GameModes/LyraUserFacingExperienceDefinition.h"
#include "LobbyGameMode.generated.h"


#define UE_API LYRAGAME_API


UCLASS(MinimalAPI)
class ALobbyGameMode : public ALyraGameMode
{
	GENERATED_BODY()

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	int32 CountdownToGameStartSeconds = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	int32 MinimumPlayersRequired = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	TSoftObjectPtr<ULyraUserFacingExperienceDefinition> FacingExperience;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Lobby Game Mode")
	FName FacingExperienceName;

	TMap<ALobbyPlayerState*, FDelegateHandle> DelegateMap;

	void OnPlayerReadyStateChanged(ALobbyPlayerState* PlayerState);

	int32 CurrentCountdownValue = 0;
	bool bCountdownStarted = false;
	void RefreshLobbyCountdown();

	FTimerHandle TimerHandle;
	void StartCountdownTimer();
	void StopCountdownTimer();
	void TimerExpiredFunction();
};

#undef UE_API