// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "Templates/SubclassOf.h"

#include "UObject/WeakObjectPtr.h"
#include "LyraLoadingScreenSubsystem.generated.h"

#define UE_API LYRAGAME_API

class UObject;
class UUserWidget;
struct FFrame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadingScreenWidgetChangedDelegate, TSoftClassPtr<UUserWidget>, NewWidgetClass);

/**
 * Tracks/stores the current loading screen configuration in a place
 * that persists across map transitions
 */
UCLASS(MinimalAPI)
class ULyraLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UE_API ULyraLoadingScreenSubsystem();

	// Sets the loading screen widget class to display inside of the loading screen widget host
	UFUNCTION(BlueprintCallable)
	UE_API void SetLoadingScreenContentWidget(TSoftClassPtr<UUserWidget> NewWidgetClass);

	// Returns the last set loading screen widget class to display inside of the loading screen widget host
	UFUNCTION(BlueprintPure)
	UE_API TSoftClassPtr<UUserWidget> GetLoadingScreenContentWidget() const;

private:
	UPROPERTY(BlueprintAssignable, meta=(AllowPrivateAccess))
	FLoadingScreenWidgetChangedDelegate OnLoadingScreenWidgetChanged;

	UPROPERTY()
	TSoftClassPtr<UUserWidget> LoadingScreenWidgetClass;
};

#undef UE_API
