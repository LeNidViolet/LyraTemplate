// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonActionWidget.h"
#include "LyraActionWidget.generated.h"

#define UE_API LYRAGAME_API

class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;

/** An action widget that will get the icon of key that is currently assigned to the common input action on this widget */
UCLASS(BlueprintType, Blueprintable)
class ULyraActionWidget : public UCommonActionWidget
{
	GENERATED_BODY()

public:

	//~ Begin UCommonActionWidget interface
	virtual FSlateBrush GetIcon() const override;
	//~ End of UCommonActionWidget interface

	/** The Enhanced Input Action that is associated with this Common Input action. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	const TObjectPtr<UInputAction> AssociatedInputAction;

protected:
	UE_API virtual void OnActionProgress(float HeldPercent) override;
	UE_API virtual void OnActionComplete() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=ActionWidget)
	bool bOverwriteHoldTintColor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=ActionWidget, meta=(EditCondition="bOverwriteHoldTintColor"))
	FSlateColor NewTintColor = FSlateColor(FLinearColor::White);

private:
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
};

#undef UE_API
