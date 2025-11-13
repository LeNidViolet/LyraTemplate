// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraInventoryItemDefinition.h"
#include "InventoryFragment_Rarity.generated.h"

#define UE_API LYRAGAME_API


/**
 * @brief 物品稀有度枚举 用于区分库存物品的稀有程度
 * Item rarity enumeration for distinguishing the rarity levels of inventory items.
 */
UENUM(BlueprintType)
enum class EInventoryItemRarity : uint8
{
	// 未知 (Unknown)
	EIR_Unknown     UMETA(DisplayName="Unknown"),
	// 普通 (Common)
	EIR_Common      UMETA(DisplayName="Common"),
	// 非凡 (Uncommon)
	EIR_Uncommon    UMETA(DisplayName="Uncommon"),
	// 稀有 (Rare)
	EIR_Rare        UMETA(DisplayName="Rare"),
	// 史诗 (Epic)
	EIR_Epic        UMETA(DisplayName="Epic"),
	// 传说 (Legendary)
	EIR_Legendary   UMETA(DisplayName="Legendary"),
	// 神话 (Mythic)
	EIR_Mythic      UMETA(DisplayName="Mythic"),
};

/**
 * @brief 物品稀有度库存碎片类 用于表示库存物品的稀有属性
 * Inventory fragment class for representing the rarity properties of inventory items.
 */
UCLASS()
class UInventoryFragment_Rarity : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	UInventoryFragment_Rarity();

	// 物品稀有度 (Rarity level of the item)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	EInventoryItemRarity Rarity = EInventoryItemRarity::EIR_Common;
};

#undef UE_API