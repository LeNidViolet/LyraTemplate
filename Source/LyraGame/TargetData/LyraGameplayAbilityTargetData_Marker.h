
#pragma once

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "LyraGameplayAbilityTargetData_Marker.generated.h"

#define UE_API LYRAGAME_API

// 自定义 TargetData 结构体用来传递消息
USTRUCT(BlueprintType)
struct FLyraGameplayAbilityTargetData_Marker : public FGameplayAbilityTargetData
{
	GENERATED_BODY()

	//------------------------------------------------------
	// 自定义数据 (想传递的任何数据)
	//------------------------------------------------------

	UPROPERTY()
	bool bHasAimingMarker = false;  // 是否正在瞄准标记 (将要移除标记)

	UPROPERTY()
	int32 AimingMarkerId = -1; // 当前瞄准的标记 ID (将要移除标记)

	UPROPERTY()
	TObjectPtr<AActor> TargetActor; // 标记位置所属于 Actor (将要添加标记)

	UPROPERTY()
	FVector TargetLocation = FVector::ZeroVector;  // 标记位置 (将要添加标记)



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
		Ar << bHasAimingMarker;
		Ar << AimingMarkerId;
		Ar << TargetActor;
		Ar << TargetLocation;

		bOutSuccess = true;
		return true;
	}
};
// 告诉 UE 启用这个 Struct 的网络序列化
template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_Marker> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_Marker>
{
	enum { WithNetSerializer = true };
};

#undef UE_API