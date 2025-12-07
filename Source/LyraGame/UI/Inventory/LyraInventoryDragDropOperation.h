// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "LyraInventoryDragDropOperation.generated.h"

#define UE_API LYRAGAME_API

class ULyraInventoryItemInstance;
/**
 *
 */
UCLASS(MinimalAPI)
class ULyraInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 拖拽时携带的数据
	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	int32 SourceSlotIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Category="Inventory")
	TObjectPtr<ULyraInventoryItemInstance> ItemInstance;
};

#undef UE_API
