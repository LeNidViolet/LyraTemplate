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


UENUM(BlueprintType)
enum class EInventorySlotOperationType : uint8
{
	EISO_None,			// 无操作 对自己
	EISO_Swap,			// 交换
	EISO_Stack,			// 堆叠
	EISO_Drop			// 丢弃
};




UCLASS()
class ULyraInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

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

	// 一个 slot 可以对另一个 slot 进行的操作类型
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	static EInventorySlotOperationType GetInventorySlotOperationType(ULyraInventoryManagerComponent* InventoryComponent, int32 SourceSlotIndex, int32 TargetSlotIndex);

	// 丢掉物品到世界
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static bool DropInventoryItem(APawn* Pawn, ULyraInventoryItemInstance* ItemInstance, int32 DropCount);

	// 交换物品槽位
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static bool SwapInventoryItem(APawn* Pawn, ULyraInventoryItemInstance* SourceSlotItemInstance, int32 SourceSlotIndex, int32 TargetSlotIndex);

	// 合并两个槽位道具数量
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static bool StackInventoryItem(APawn* Pawn, ULyraInventoryItemInstance* SourceSlotItemInstance, int32 SourceSlotIndex, int32 TargetSlotIndex);
};


#undef UE_API