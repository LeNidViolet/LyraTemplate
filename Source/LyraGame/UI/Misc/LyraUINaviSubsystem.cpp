// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraUINaviSubsystem.h"

#include "NativeGameplayTags.h"
#include "Blueprint/UserWidget.h"


UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UI_Navi_Focus, TEXT("UI.Navi.Focus"))


void ULyraUINaviSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameplayMessageSubsystem & MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	UINaviFocusEventListener = MessageSubsystem.RegisterListener<FLyraUINaviFocus>(
		TAG_UI_Navi_Focus,
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

void ULyraUINaviSubsystem::HandleUINaviFocusEvent(FGameplayTag Channel, const FLyraUINaviFocus& Payload)
{
	FString NewFocusedWidgetId = Payload.WidgetId;
	if (NewFocusedWidgetId != CurrentFocusedWidgetId)
	{
		CurrentFocusedWidgetId = NewFocusedWidgetId;
		Payload.WidgetToFocus.Get()->SetFocus();
	}
}
