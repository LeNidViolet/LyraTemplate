// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraUINaviSubsystem.generated.h"


struct FOnUIFocusNaviParameters;

UCLASS(MinimalAPI)
class ULyraUINaviSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	ULyraUINaviSubsystem() {}

	// 设置是否允许摇杆导航
	UFUNCTION(BlueprintCallable, Category=UINavigation)
	static void SetAllowAnalogNavigation(bool Allow);


protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;



	FGameplayMessageListenerHandle UINaviFocusEventListener;
	void HandleUIFocusNaviEvent(FGameplayTag Channel, const FOnUIFocusNaviParameters& Parameters);

	FString CurrentFocusedWidgetId;
};
