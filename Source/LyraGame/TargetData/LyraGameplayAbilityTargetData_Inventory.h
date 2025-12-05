
#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "LyraGameplayAbilityTargetData_Inventory.generated.h"

#define UE_API LYRAGAME_API

class ULyraInventoryItemInstance;
// 定义操作类型 丢弃 ? 交换 ?
UENUM(BlueprintType)
enum class EInventoryTargetDataOperationType : uint8
{
	EITDOT_Drop         UMETA(DisplayName = "Drop"),        // 丢弃道具
	EITDOT_Swap         UMETA(DisplayName = "Swap"),        // 交换道具
};



// 自定义 TargetData 结构体用来传递消息 目前用来容纳 丢弃道具 交换道具的操作数据
USTRUCT(BlueprintType)
struct FLyraGameplayAbilityTargetData_Inventory : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	//------------------------------------------------------
	// 自定义数据 (想传递的任何数据)
	//------------------------------------------------------

	UPROPERTY()
	EInventoryTargetDataOperationType OperationType = EInventoryTargetDataOperationType::EITDOT_Drop; // 操作类型

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
		Ar << OperationType;
		Ar << ItemInstance;
		Ar << DropCount;
		Ar << DropLocation;

		bOutSuccess = true;
		return true;
	}
};
// 告诉 UE 启用这个 Struct 的网络序列化
template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_Inventory> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_Inventory>
{
	enum { WithNetSerializer = true };
};

#undef UE_API