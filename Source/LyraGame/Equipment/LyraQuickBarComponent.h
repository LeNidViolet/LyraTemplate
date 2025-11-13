// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ControllerComponent.h"
#include "Inventory/LyraInventoryItemInstance.h"

#include "LyraQuickBarComponent.generated.h"

#define UE_API LYRAGAME_API

class AActor;
class ULyraEquipmentInstance;
class ULyraEquipmentManagerComponent;
class UObject;
struct FFrame;

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class ULyraQuickBarComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	ULyraQuickBarComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="QuickBar")
	UE_API void CycleActiveSlotForward();

	UFUNCTION(BlueprintCallable, Category="QuickBar")
	UE_API void CycleActiveSlotBackward();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category="QuickBar")
	UE_API void SetActiveSlotIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="QuickBar")
	UE_API TArray<ULyraInventoryItemInstance*> GetSlots() const
	{
		return Slots;
	}

	UFUNCTION(BlueprintPure, Category="QuickBar")
	UE_API int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="QuickBar")
	UE_API ULyraInventoryItemInstance* GetActiveSlotItem() const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category="QuickBar")
	UE_API int32 GetNextFreeItemSlot() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="QuickBar")
	UE_API void AddItemToSlot(int32 SlotIndex, ULyraInventoryItemInstance* Item);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="QuickBar")
	UE_API ULyraInventoryItemInstance* RemoveItemFromSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category="QuickBar")
	UE_API int32 GetQuickBarCapacity() const { return NumSlots; }

	virtual void BeginPlay() override;

private:
	void UnequipItemInSlot();
	void EquipItemInSlot();

	ULyraEquipmentManagerComponent* FindEquipmentManager() const;

protected:
	// Number of quickbar slots 具体数值从GameData中获取
	int32 NumSlots = 0;

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_ActiveSlotIndex();

private:
	UPROPERTY(ReplicatedUsing=OnRep_Slots)
	TArray<TObjectPtr<ULyraInventoryItemInstance>> Slots;

	UPROPERTY(ReplicatedUsing=OnRep_ActiveSlotIndex)
	int32 ActiveSlotIndex = -1;

	UPROPERTY()
	TObjectPtr<ULyraEquipmentInstance> EquippedItem;
};


USTRUCT(BlueprintType)
struct FLyraQuickBarSlotsChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TArray<TObjectPtr<ULyraInventoryItemInstance>> Slots;
};


USTRUCT(BlueprintType)
struct FLyraQuickBarActiveIndexChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<AActor> Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 ActiveIndex = 0;
};

#undef UE_API