// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraUINaviSubsystem.h"

#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraNotificationMessage_UINaviFocus.h"
#include "Blueprint/UserWidget.h"



void ULyraUINaviSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameplayMessageSubsystem & MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	UINaviFocusEventListener = MessageSubsystem.RegisterListener<FOnUINaviFocusParameters>(
		LyraGameplayTags::UI_Navi_Focus,
		this,
		&ThisClass::HandleUINaviFocusEvent
	);
}

void ULyraUINaviSubsystem::Deinitialize()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	if (UINaviFocusEventListener.IsValid())
	{
		MessageSubsystem.UnregisterListener(UINaviFocusEventListener);
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
