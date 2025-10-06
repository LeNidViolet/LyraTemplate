// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Foundation/LyraLoadingScreenSubsystem.h"

#include "LoadingScreenManager.h"
#include "Blueprint/UserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraLoadingScreenSubsystem)

class UUserWidget;

//////////////////////////////////////////////////////////////////////
// ULyraLoadingScreenSubsystem

ULyraLoadingScreenSubsystem::ULyraLoadingScreenSubsystem()
{
}

void ULyraLoadingScreenSubsystem::SetLoadingScreenContentWidget(TSoftClassPtr<UUserWidget> NewWidgetClass)
{
	// Update to the LoadingScreenManager for next use
	if (ULoadingScreenManager* LoadingScreenManager = UGameInstance::GetSubsystem<ULoadingScreenManager>(GetGameInstance()))
	{
		LoadingScreenManager->SetOneTimeLoadScreenWidget(NewWidgetClass);
	}

	if (LoadingScreenWidgetClass != NewWidgetClass)
	{
		LoadingScreenWidgetClass = NewWidgetClass;

		OnLoadingScreenWidgetChanged.Broadcast(LoadingScreenWidgetClass);
	}
}

TSoftClassPtr<UUserWidget> ULyraLoadingScreenSubsystem::GetLoadingScreenContentWidget() const
{
	return LoadingScreenWidgetClass;
}

