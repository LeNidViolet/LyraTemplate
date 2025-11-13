// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraInventoryItemDefinition.h"
#include "InventoryFragment_InventoryIcon.generated.h"

/**
 *
 */
UCLASS()
class UInventoryFragment_InventoryIcon : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	// 显示在背包界面的图标
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;
};
