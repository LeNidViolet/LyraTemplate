// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/LyraGameMode.h"
#include "LyraDSGameMode.generated.h"

#define UE_API LYRAGAME_API

class ALyraDSPlayerState;
class ALyraDSPlayerController;
struct FServerRequestPlaceMarkerParameters;
struct FServerRequestRemoveMarkerParameters;

UCLASS(MinimalAPI)
class ALyraDSGameMode : public ALyraGameMode
{
	GENERATED_BODY()

public:
	UE_API ALyraDSGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UE_API virtual void PostLogin(APlayerController* NewPlayer) override;
	UE_API virtual void Logout(AController* Exiting) override;

	UE_API virtual void BeginPlay() override;

private:

	TArray<FColor> AvailablePlayerColors;
	int32 NextColorIndex = 0;
};

#undef UE_API