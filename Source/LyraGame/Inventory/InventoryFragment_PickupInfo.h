// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Inventory/LyraInventoryItemDefinition.h"
#include "UObject/ObjectPtr.h"

#include "InventoryFragment_PickupInfo.generated.h"

class UNiagaraSystem;
class UObject;

UCLASS()
class UInventoryFragment_PickupInfo : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	UInventoryFragment_PickupInfo();

	// 物品显示用的静态网格
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMesh> DisplayMesh;

	// 拾取音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> PickedUpSound;

	// 生成音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USoundBase> SpawnedSound;

	// 拾取时的粒子特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> PickedUpEffect;

	// 生成时的粒子特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UNiagaraSystem> SpawnedEffect;
};
