// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraUINaviSubsystem.h"

#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraNotificationMessage_UIFocusNavi.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Application/NavigationConfig.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraUINaviSubsystem)


void ULyraUINaviSubsystem::SetAllowAnalogNavigation(bool Allow)
{
	FSlateApplication::Get().GetNavigationConfig()->bAnalogNavigation = Allow;
}

void ULyraUINaviSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 不允许Tab进行导航, 我们可能会使用它来打开背包等UI
	FSlateApplication::Get().GetNavigationConfig()->bTabNavigation = false;

	if (ULocalPlayer* Player = GetLocalPlayer())
	{
		UGameInstance* GameInstance = Player->GetGameInstance();
		if (GameInstance)
		{
			UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
			if (MessageSubsystem)
			{
				if (!UINaviFocusEventListener.IsValid())
				{
					UINaviFocusEventListener = MessageSubsystem->RegisterListener<FOnUIFocusNaviParameters>(
						LyraGameplayTags::Gameplay_Message_UI_FocusNavi,
						this,
						&ThisClass::HandleUIFocusNaviEvent
					);
				}
			}
		}
	}
}

void ULyraUINaviSubsystem::Deinitialize()
{
	if (UINaviFocusEventListener.IsValid())
	{
		UINaviFocusEventListener.Unregister();
	}

	Super::Deinitialize();
}

void ULyraUINaviSubsystem::HandleUIFocusNaviEvent(FGameplayTag Channel, const FOnUIFocusNaviParameters& Parameters)
{
	FString NewFocusedWidgetId = Parameters.WidgetId;
	if (NewFocusedWidgetId != CurrentFocusedWidgetId)
	{
		CurrentFocusedWidgetId = NewFocusedWidgetId;
		Parameters.WidgetToFocus.Get()->SetFocus();
	}
}
