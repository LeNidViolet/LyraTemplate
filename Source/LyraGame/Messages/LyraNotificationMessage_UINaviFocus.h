
#pragma once

#include "LyraNotificationMessage_UINaviFocus.generated.h"

USTRUCT(BlueprintType)
struct FOnUINaviFocusParameters
{
	GENERATED_BODY()

	/** The Widget to be Focus **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi")
	TObjectPtr<UUserWidget> WidgetToFocus;

	/** Widget identify **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi")
	FString WidgetId;
};
