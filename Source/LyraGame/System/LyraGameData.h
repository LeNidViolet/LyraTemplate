// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "Interaction/LyraWorldCollectable.h"
#include "Interaction/LyraWorldMarker.h"
#include "Inventory/InventoryFragment_Rarity.h"

#include "LyraGameData.generated.h"

#define UE_API LYRAGAME_API

class ALyraWorldCollectable;
enum class ELyraWorldMarkerType : uint8;
class UIndicatorDescriptor;

class UGameplayEffect;
class UObject;




USTRUCT(BlueprintType)
struct FLyraWorldMarkerDescriptor
{
	GENERATED_BODY()

	// 标记点类型
	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::None;

	// 使用的 UIndicatorDescriptor 类
	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	TSubclassOf<UIndicatorDescriptor> IndicatorDescriptorClass;
};

USTRUCT(BlueprintType)
struct FLyraWorldMarkerCallout
{
	GENERATED_BODY()

	// 标记点类型
	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::None;

	// 使用的 Callout Widget 类
	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	TSubclassOf<UUserWidget> CalloutClass;
};

USTRUCT(BlueprintType)
struct FLyraRarityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	EInventoryItemRarity Rarity = EInventoryItemRarity::EIR_Unknown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FText DisplayName = FText::FromString("Unknown");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rarity")
	FGameplayTag RarityTag = FGameplayTag();
};



/**
 * ULyraGameData
 *
 *	Non-mutable data asset that contains global game data.
 */
UCLASS(MinimalAPI, BlueprintType, Const, Meta = (DisplayName = "Lyra Game Data", ShortTooltip = "Data asset containing global game data."))
class ULyraGameData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UE_API ULyraGameData();

	// Returns the loaded game data.
	static UE_API const ULyraGameData& Get();

public:

	// Gameplay effect used to apply damage.  Uses SetByCaller for the damage magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> DamageGameplayEffect_SetByCaller;

	// Gameplay effect used to apply healing.  Uses SetByCaller for the healing magnitude.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects", meta = (DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<UGameplayEffect> HealGameplayEffect_SetByCaller;

	// Gameplay effect used to add and remove dynamic tags.
	UPROPERTY(EditDefaultsOnly, Category = "Default Gameplay Effects")
	TSoftClassPtr<UGameplayEffect> DynamicTagGameplayEffect;



	// 目标 Actor 在屏幕空间中的偏移(默认端点位置在 Actor 中心)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Callout")
	FVector2D CalloutTargetActorScreenOffset = FVector2D::ZeroVector;

	// CalloutWidget 在屏幕空间中的偏移(默认端点位置在左下角)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Callout")
	FVector2D CalloutWidgetScreenOffset = FVector2D::ZeroVector;

	// 连接 CalloutWidget 的线条颜色
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Callout")
	FLinearColor CalloutWidgetLineColor = FLinearColor::White;

	// 连接 CalloutWidget 的线条粗细
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Callout")
	float CalloutWidgetLineThickness = 2.0f;






	// 射线检测通道
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Indicator|Marker")
	TEnumAsByte<ECollisionChannel> MarkerTraceChannel = ECC_WorldStatic;

	// 射线检测距离
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Indicator|Marker")
	float MarkerTraceDistance = 50000.f;

	// 要使用的Marker类类型
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Indicator|Marker")
	TSubclassOf<ALyraWorldMarker> MarkerClass = ALyraWorldMarker::StaticClass();

	// 目标标记类型 (先固定为 Waypoint, 应该根据目标Actor类型自动选择)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Indicator|Marker")
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::Waypoint;



	// 标记点类型与使用的 UIndicatorDescriptor 对应的关系
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Marker")
	TArray<FLyraWorldMarkerDescriptor> MarkerIndicators;

	// 标记点类型与使用的 Callout Widget 对应的关系
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Marker")
	TArray<FLyraWorldMarkerCallout> MarkerCallouts;

	// 标记点交互提示扫描频率(UI)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Marker")
	float MarkerScanRate = 0.2;

	// 标记点交互提示扫描半径(UI)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator|Marker")
	float MarkerScanRadius = 40;

	// MarkerIndicators 查询
	UFUNCTION(BlueprintCallable, Category = "Indicator|Marker")
	TSubclassOf<UIndicatorDescriptor> GetIndicatorClassForMarkerType(ELyraWorldMarkerType WorldMarkerType) const;

	// MarkerCallouts 查询
	UFUNCTION(BlueprintCallable, Category = "Indicator|Marker")
	TSubclassOf<UUserWidget> GetCalloutClassForMarkerType(ELyraWorldMarkerType WorldMarkerType) const;

	// Nameplate 使用的 UIndicatorDescriptor 类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator")
	TSubclassOf<UIndicatorDescriptor> NameplateIndicatorClass;

	// Collectable 使用的 UIndicatorDescriptor 类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Indicator")
	TSubclassOf<UIndicatorDescriptor> CollectableIndicatorClass;



	// 激活Interact技能时指定的TriggerTag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interact")
	FGameplayTag InteractTriggerTag = FGameplayTag::EmptyTag;

	// 扫描可交互对象时使用的CollisionProfile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interact")
	FName InteractTraceProfileName = FName("Interactable_BlockDynamic");

	// 可交互对象扫描频率
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interact")
	float InteractScanRate = 0.1f;

	// 可交互对象扫描范围
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interact")
	float InteractScanRange = 300.f;




	// 库存最大容量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 InventoryMaxCapacity = 30;

	// Inventory Screen 每一行排列几个槽位
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	int32 InventorySlotsPerRow = 8;

	// 快捷栏槽位数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	int32 QuickBarMaxCapacity = 5;

	// QuickBar Screen 每一行排列几个槽位
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	int32 QuickBarSlotsPerRow = 8;

	// 丢弃物品的数量曲线 横轴是当前持有量, 纵轴是 丢弃比例 (0.0 - 1.0) 或者 丢弃数量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TObjectPtr<UCurveFloat> InventoryDropCurve;

	// 物品稀有度信息列表
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	TArray<FLyraRarityInfo> RarityInfoList;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FLyraRarityInfo GetRarityInfo(EInventoryItemRarity Rarity) const;
};

#undef UE_API
