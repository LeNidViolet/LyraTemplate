// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IInteractableMarker.generated.h"


/**  */
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableMarker : public UInterface
{
	GENERATED_BODY()
};

/**  */
class IInteractableMarker
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category="WorldMarker")
	void K2_OnShowMarkerInteractablePrompt(UIndicatorDescriptor* Descriptor);

	UFUNCTION(BlueprintImplementableEvent, Category="WorldMarker")
	void K2_OnHideMarkerInteractablePrompt(UIndicatorDescriptor* Descriptor);

	UFUNCTION(BlueprintImplementableEvent, Category="WorldMarker")
	FVector2D K2_OnGetWidgetPixelSize();
};
