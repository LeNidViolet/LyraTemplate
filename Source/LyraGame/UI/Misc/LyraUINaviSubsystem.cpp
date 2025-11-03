// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraUINaviSubsystem.h"

#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraNotificationMessage_UINaviFocus.h"
#include "Blueprint/UserWidget.h"



void ULyraUINaviSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);



	if (ULocalPlayer* Player = GetLocalPlayer())
	{
		UGameInstance* GameInstance = Player->GetGameInstance();
		if (GameInstance)
		{
			UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
			if (MessageSubsystem)
			{
				UINaviFocusEventListener = MessageSubsystem->RegisterListener<FOnUINaviFocusParameters>(
					LyraGameplayTags::UI_Navi_Focus,
					this,
					&ThisClass::HandleUINaviFocusEvent
				);
			}
		}
	}
}

void ULyraUINaviSubsystem::Deinitialize()
{
	if (UINaviFocusEventListener.IsValid())
	{
		if (ULocalPlayer* Player = GetLocalPlayer())
		{
			UGameInstance* GameInstance = Player->GetGameInstance();
			if (GameInstance)
			{
				UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
				if (MessageSubsystem)
				{
					MessageSubsystem->UnregisterListener(UINaviFocusEventListener);
				}
			}
		}
	}

	Super::Deinitialize();
}

void ULyraUINaviSubsystem::HandleUINaviFocusEvent(FGameplayTag Channel, const FOnUINaviFocusParameters& Parameters)
{
	FString NewFocusedWidgetId = Parameters.WidgetId;
	if (NewFocusedWidgetId != CurrentFocusedWidgetId)
	{
		CurrentFocusedWidgetId = NewFocusedWidgetId;
		Parameters.WidgetToFocus.Get()->SetFocus();
	}
}
