// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "LyraInventoryItemDefinition.generated.h"

class ALyraWorldCollectable;
template <typename T> class TSubclassOf;

class ULyraInventoryItemInstance;
struct FFrame;

//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class ULyraInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:

	// 从 Definition 拷贝模版数据给 Instance
	virtual void OnInstanceCreated(ULyraInventoryItemInstance* Instance) const {}
};




//////////////////////////////////////////////////////////////////////

// 物品类型
UENUM(BlueprintType)
enum class EInventoryItemCategory : uint8
{
	EIC_None            UMETA(DisplayName = "None"),

	// 武器
	EIC_Weapon          UMETA(DisplayName = "Weapon"),
	// 弹药
	EIC_Ammo            UMETA(DisplayName = "Ammo"),

	// 防具类
	// EIC_Armor           UMETA(DisplayName = "Armor"),
	// EIC_Shield          UMETA(DisplayName = "Shield"),

	// 消耗品
	EIC_Consumable      UMETA(DisplayName = "Consumable"),
	// EIC_Medical         UMETA(DisplayName = "Medical"),

	// 金币
	EIC_Currency        UMETA(DisplayName = "Currency"),

	// 制作材料
	EIC_Resource        UMETA(DisplayName = "Resource"),
	// EIC_Component       UMETA(DisplayName = "Component"),

	// 任务物品
	// EIC_QuestItem       UMETA(DisplayName = "Quest Item"),

	// 杂项
	// EIC_Accessory       UMETA(DisplayName = "Accessory"),
	// EIC_KeyItem         UMETA(DisplayName = "Key Item"),
	// EIC_Misc            UMETA(DisplayName = "Misc"),
};




//////////////////////////////////////////////////////////////////////

/**
 * ULyraInventoryItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class ULyraInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	ULyraInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 物品显示名称
	// Item display name
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	FText DisplayName;

	// 物品实例个数限制(它可以出现在几个slot中), 注意这并不是堆叠数量限制 0 表示无限制
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	int32 MaxInstanceSlotCount = 0;


	// 是否允许堆叠
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	bool bAllowStacking = true;

	// 物品最大堆叠数量(单个slot)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory", meta=(EditCondition="bAllowStacking"))
	int32 MaxStackCount = 1;

	// 物品类别
	// Item category
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	EInventoryItemCategory ItemCategory = EInventoryItemCategory::EIC_None;

	// 物品是否支持自动拾取
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	bool bAutoPickup = false;

	// 物品掉落时使用的 WorldCollectable 类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TSubclassOf<ALyraWorldCollectable> WorldCollectableClass;

	// 物品碎片列表
	// Item fragment list
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory", Instanced)
	TArray<TObjectPtr<ULyraInventoryItemFragment>> Fragments;

	UFUNCTION(BlueprintCallable, Category=Inventory, meta=(DeterminesOutputType=FragmentClass))
	const ULyraInventoryItemFragment* FindFragmentByClass(TSubclassOf<ULyraInventoryItemFragment> FragmentClass) const;

protected:
	virtual void PostInitProperties() override;
};
