// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerState.h"
#include "LyraDSPlayerState.generated.h"

#define UE_API LYRAGAME_API


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerColorChanged_Dynamic, FColor, NewColor);

UCLASS(MinimalAPI)
class ALyraDSPlayerState : public ALyraPlayerState
{
	GENERATED_BODY()

public:
	UE_API ALyraDSPlayerState(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category="PlayerState")
	FOnPlayerColorChanged_Dynamic OnPlayerColorChanged;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerColor, BlueprintReadOnly, VisibleAnywhere, Category="PlayerState")
	FColor PlayerColor = FColor::White;

protected:
	UFUNCTION()
	void OnRep_PlayerColor();
};

#undef UE_API