
#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "LyraGameplayAbilityTargetData_Inventory.generated.h"

#define UE_API LYRAGAME_API

class ULyraInventoryItemInstance;




USTRUCT(BlueprintType)
struct FLyraGameplayAbilityTargetData_Inventory_Drop : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	//------------------------------------------------------
	// 自定义数据 (想传递的任何数据)
	//------------------------------------------------------

	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> ItemInstance = nullptr; // 道具实例 (将要丢弃/交换的道具)

	UPROPERTY()
	int32 DropCount = 0; // 数量 (将要丢弃的数量)

	UPROPERTY()
	FVector DropLocation = FVector::ZeroVector; // 丢弃位置 (将要丢弃道具时使用)

	//------------------------------------------------------
	// 必需的虚函数重写
	//------------------------------------------------------

	/** 返回结构体类型 */
	UE_API virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** 网络序列化（必需实现） */
	UE_API bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// 序列化所有自定义字段
		Ar << ItemInstance;
		Ar << DropCount;
		Ar << DropLocation;

		bOutSuccess = true;
		return true;
	}
};
// 告诉 UE 启用这个 Struct 的网络序列化
template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_Inventory_Drop> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_Inventory_Drop>
{
	enum { WithNetSerializer = true };
};



USTRUCT(BlueprintType)
struct FLyraGameplayAbilityTargetData_Inventory_Swap : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	//------------------------------------------------------
	// 自定义数据 (想传递的任何数据)
	//------------------------------------------------------

	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> ItemInstance = nullptr; // 道具实例 (将要丢弃/交换的道具)

	UPROPERTY()
	int32 SourceSlotIndex = INDEX_NONE; // 源槽位索引

	UPROPERTY()
	int32 TargetSlotIndex = INDEX_NONE; // 目标槽位索引


	//------------------------------------------------------
	// 必需的虚函数重写
	//------------------------------------------------------

	/** 返回结构体类型 */
	UE_API virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** 网络序列化（必需实现） */
	UE_API bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// 序列化所有自定义字段
		Ar << ItemInstance;
		Ar << SourceSlotIndex;
		Ar << TargetSlotIndex;

		bOutSuccess = true;
		return true;
	}
};
// 告诉 UE 启用这个 Struct 的网络序列化
template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_Inventory_Swap> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_Inventory_Swap>
{
	enum { WithNetSerializer = true };
};



USTRUCT(BlueprintType)
struct FLyraGameplayAbilityTargetData_Inventory_Stack : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	//------------------------------------------------------
	// 自定义数据 (想传递的任何数据)
	//------------------------------------------------------

	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> ItemInstance = nullptr; // 道具实例 (将要丢弃/交换的道具)

	UPROPERTY()
	int32 SourceSlotIndex = INDEX_NONE; // 源槽位索引

	UPROPERTY()
	int32 TargetSlotIndex = INDEX_NONE; // 目标槽位索引


	//------------------------------------------------------
	// 必需的虚函数重写
	//------------------------------------------------------

	/** 返回结构体类型 */
	UE_API virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	/** 网络序列化（必需实现） */
	UE_API bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		// 序列化所有自定义字段
		Ar << ItemInstance;
		Ar << SourceSlotIndex;
		Ar << TargetSlotIndex;

		bOutSuccess = true;
		return true;
	}
};
// 告诉 UE 启用这个 Struct 的网络序列化
template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_Inventory_Stack> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_Inventory_Stack>
{
	enum { WithNetSerializer = true };
};

#undef UE_API