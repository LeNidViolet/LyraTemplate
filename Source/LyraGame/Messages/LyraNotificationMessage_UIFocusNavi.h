
#pragma once

#include "LyraNotificationMessage_UIFocusNavi.generated.h"

USTRUCT(BlueprintType)
struct FOnUIFocusNaviParameters
{
	GENERATED_BODY()

	/** The Widget to be Focus **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI Navi")
	TObjectPtr<UUserWidget> WidgetToFocus;

	/** Widget identify **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI Navi")
	FString WidgetId;
};
