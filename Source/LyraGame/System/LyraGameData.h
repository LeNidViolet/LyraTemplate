// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Interaction/LyraWorldMarker.h"

#include "LyraGameData.generated.h"

#define UE_API LYRAGAME_API

enum class ELyraWorldMarkerType : uint8;
class UIndicatorDescriptor;

class UGameplayEffect;
class UObject;




USTRUCT(BlueprintType)
struct FLyraWorldMarkerTypeIndicatorMapping
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::None;

	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	TSubclassOf<UIndicatorDescriptor> IndicatorDescriptorClass;
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

	// 记录标记点类型与使用的 UIndicatorDescriptor 子类对应的关系
	UPROPERTY(EditDefaultsOnly, Category = "WorldMarker")
	TArray<FLyraWorldMarkerTypeIndicatorMapping> TypeIndicatorMappings;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "WorldMarker")
	TSubclassOf<UIndicatorDescriptor> GetIndicatorDescriptorClassForMarkerType(ELyraWorldMarkerType MarkerType) const;
};

#undef UE_API
