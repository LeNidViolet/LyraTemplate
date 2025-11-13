// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/LyraInventoryItemDefinition.h"
#include "Styling/SlateBrush.h"

#include "InventoryFragment_QuickBarIcon.generated.h"

class UObject;

UCLASS()
class UInventoryFragment_QuickBarIcon : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;

	// 使用 InventoryFragment_Ammo 代替
//	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
//	FSlateBrush AmmoBrush;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayNameWhenEquipped;
};
