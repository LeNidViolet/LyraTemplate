// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/LyraWorldCollectable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LyraInventoryFunctionLibrary.generated.h"

#define UE_API LYRAGAME_API


enum class EInventoryCanAddItemResult : uint8;
class ULyraInventoryManagerComponent;
enum class EWeaponAmmoType : uint8;
enum class EInventoryItemRarity : uint8;
class ULyraInventoryItemDefinition;
class ULyraInventoryItemFragment;

/**
 *
 */
UCLASS()
class ULyraInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// 查找物品定义碎片
	// Find item definition fragment
	UFUNCTION(BlueprintPure, Category="Inventory", meta=(DeterminesOutputType=FragmentClass))
	static const ULyraInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, TSubclassOf<ULyraInventoryItemFragment> FragmentClass);


	// 获取稀有度颜色
	// Get rarity color
	UFUNCTION(BlueprintPure, Category="Inventory")
	static FLinearColor GetRarityColor(EInventoryItemRarity Rarity);

	// 获取稀有度名称
	// Get rarity name
	UFUNCTION(BlueprintPure, Category="Inventory")
	static FText GetRarityName(EInventoryItemRarity Rarity);

	// 获取弹药数 这里是主动遍历获取弹药数 推荐只作为首次查询使用
	// Get ammo count by actively iterating through the inventory. Recommended for initial queries only.
	UFUNCTION(BlueprintPure, Category="Inventory")
	static int32 GetAmmoCount(ULyraInventoryManagerComponent* InventoryComponent, EWeaponAmmoType AmmoType);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	static FString GetInventoryAddItemResultString(EInventoryCanAddItemResult Result);
};


#undef UE_API