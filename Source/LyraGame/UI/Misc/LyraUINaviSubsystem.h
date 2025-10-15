// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraUINaviSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FLyraUINaviFocus
{
	GENERATED_BODY()

public:
	/** The Widget to be Focus **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi")
	TObjectPtr<UUserWidget> WidgetToFocus;

	/** Widget identify **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi")
	FString WidgetId;
};


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
	void HandleUINaviFocusEvent(FGameplayTag Channel, const FLyraUINaviFocus& Payload);

	FString CurrentFocusedWidgetId;
};
