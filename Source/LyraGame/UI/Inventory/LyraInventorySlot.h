// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/Foundation/LyraButtonBase.h"
#include "LyraInventorySlot.generated.h"

#define UE_API LYRAGAME_API

class ULyraInventoryItemInstance;
class ULyraInventoryManagerComponent;
struct FOnInventoryStackChangeParameters;

/**
 *
 */
UCLASS()
class ULyraInventorySlot : public ULyraButtonBase
{
	GENERATED_BODY()

public:
	UE_API ULyraInventorySlot(const FObjectInitializer& ObjectInitializer);

	UE_API void SetSlotIndex(int32 SlotIndex) { this->SlotIndex = SlotIndex; }

	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API int32 GetSlotIndex() const { return SlotIndex; }

protected:
	// ~ UCommonUserWidget
	UE_API virtual void NativeOnInitialized() override;
	UE_API virtual void NativeConstruct() override;
	UE_API virtual void NativeDestruct() override;
	// ~ UCommonUserWidget

	UE_API void RegisterMessageHandlers();
	UE_API void UnregisterMessageHandlers();

	FGameplayMessageListenerHandle InventoryStackChangedEventListener;
	UE_API void HandleInventoryStackChangeEvent(FGameplayTag Channel, const FOnInventoryStackChangeParameters& Parameters);

	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API ULyraInventoryItemInstance* GetInventoryItemInstance() const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API int32 GetInventoryItemStackCount() const;

	// 更新slot ui, 在蓝图实现
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void K2_UpdateSlot();

private:

	void InitializeSlot();

	// UI 槽位索引
	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY()
	TObjectPtr<ULyraInventoryManagerComponent> InventoryManager;
};

#undef UE_API
