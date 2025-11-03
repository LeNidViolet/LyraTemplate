// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerController.h"
#include "LyraDSPlayerController.generated.h"


#define UE_API LYRAGAME_API

/**
 *
 */
UCLASS(MinimalAPI)
class ALyraDSPlayerController : public ALyraPlayerController
{
	GENERATED_BODY()

public:
	UE_API ALyraDSPlayerController(const FObjectInitializer& ObjectInitializer);
};

#undef UE_API