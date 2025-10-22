// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraUINaviSubsystem.generated.h"


struct FOnUINaviFocusParameters;

UCLASS(MinimalAPI)
class ULyraUINaviSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	ULyraUINaviSubsystem() {}

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FGameplayMessageListenerHandle UINaviFocusEventListener;
	void HandleUINaviFocusEvent(FGameplayTag Channel, const FOnUINaviFocusParameters& Parameters);

	FString CurrentFocusedWidgetId;
};
