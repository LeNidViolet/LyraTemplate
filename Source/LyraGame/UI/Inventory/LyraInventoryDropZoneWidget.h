// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "LyraInventoryDropZoneWidget.generated.h"

#define UE_API LYRAGAME_API

/**
 *
 */
UCLASS(MinimalAPI)
class ULyraInventoryDropZoneWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UE_API ULyraInventoryDropZoneWidget(const FObjectInitializer& ObjectInitializer);

protected:

	UE_API virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UE_API virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	UE_API virtual bool NativeOnDrop( const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};

#undef UE_API
