// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IInteractableCallout.generated.h"


/**  */
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableCallout : public UInterface
{
	GENERATED_BODY()
};

/**  */
class IInteractableCallout
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent, Category="Callout")
	void K2_OnShowCalloutInteractablePrompt(AActor* Actor);

	UFUNCTION(BlueprintImplementableEvent, Category="Callout")
	void K2_OnHideCalloutInteractablePrompt(AActor* Actor);
};
