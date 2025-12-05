// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/LyraActivatableWidget.h"
#include "LyraInventoryScreen.generated.h"


#define UE_API LYRAGAME_API


class ULyraInventorySlot;
struct FOnInventoryStackChangeParameters;
class ULyraEquipmentManagerComponent;
class ULyraQuickBarComponent;
class ULyraInventoryManagerComponent;
enum class ECommonInputType : uint8;
/**
 *
 */
UCLASS()
class ULyraInventoryScreen : public ULyraActivatableWidget
{
	GENERATED_BODY()

public:

protected:
	// ~ UCommonUserWidget
	UE_API virtual void NativeOnInitialized() override;
	UE_API virtual void NativeConstruct() override;
	UE_API virtual void NativeDestruct() override;
	// ~ UCommonUserWidget

	// ~ ULyraActivatableWidget
	UE_API virtual void NativeOnActivated() override;
	UE_API virtual void NativeOnDeactivated() override;
	// ~ ULyraActivatableWidget


	// 关闭界面
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FDataTableRowHandle BackAction;

	// 丢弃物品
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FDataTableRowHandle DropAction;
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FDataTableRowHandle DropAllAction;

	// 选中物品
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FDataTableRowHandle SelectAction;

	UE_API void HandleBackAction();
	UE_API void HandleDropAction();
	UE_API void HandleDropAllAction();
	UE_API void HandleSelectAction();

	// 通知蓝图库存变化事件
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventoryStackChanged(const FOnInventoryStackChangeParameters& Parameters);

	// Inventory Slot相关事件
	UE_API void HandleInventorySlotFocused(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotFocused(ULyraInventorySlot* SlotWidget);

	UE_API void HandleInventorySlotUnfocused(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotUnfocused(ULyraInventorySlot* SlotWidget);


	UE_API void HandleInventorySlotHovered(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotHovered(ULyraInventorySlot* SlotWidget);

	UE_API void HandleInventorySlotUnhovered(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotUnhovered(ULyraInventorySlot* SlotWidget);


	UE_API void HandleInventorySlotClicked(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotClicked(ULyraInventorySlot* SlotWidget);





	// Quick Slot相关事件
	UE_API void HandleQuickBarSlotFocused(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotFocused(ULyraInventorySlot* SlotWidget);

	UE_API void HandleQuickBarSlotUnfocused(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotUnfocused(ULyraInventorySlot* SlotWidget);



	UE_API void HandleQuickBarSlotHovered(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotHovered(ULyraInventorySlot* SlotWidget);

	UE_API void HandleQuickBarSlotUnhovered(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotUnfovered(ULyraInventorySlot* SlotWidget);



	UE_API void HandleQuickBarSlotClicked(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotClicked(ULyraInventorySlot* SlotWidget);


	UE_API void HandleInventorySlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);

	UE_API void HandleQuickBarSlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);


	UE_API ULyraInventorySlot* GetCurrentFocusedInventorySlot() const;
	UE_API ULyraInventorySlot* GetCurrentFocusedQuickBarSlot() const;


	// Slot Widget Class
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<ULyraInventorySlot> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<ULyraInventorySlot> QuickBarSlotWidgetClass;

	// Inventory Slots Grid Panel
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UUniformGridPanel> InventorySlotsGridPanel;

	// QuickBar Slots Grid Panel
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UUniformGridPanel> QuickBarSlotsGridPanel;

private:
	FUIActionBindingHandle BackHandle;
	FUIActionBindingHandle DropHandle;
	FUIActionBindingHandle DropAllHandle;
	FUIActionBindingHandle SelectHandle;

	FDelegateHandle InputMethodChangedHandle;
	void HandleInputMethodChanged(ECommonInputType NewInputMethod);

	void DropActionDo(bool DropAll);

	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();

	FGameplayMessageListenerHandle InventoryStackChangedEventListener;
	void HandleInventoryStackChangeEvent(FGameplayTag Channel, const FOnInventoryStackChangeParameters& Parameters);

	void InitializeComponents();
	UPROPERTY()
	TObjectPtr<ULyraInventoryManagerComponent> InventoryManagerComponent;
	UPROPERTY()
	TObjectPtr<ULyraQuickBarComponent> QuickBarComponent;
	UPROPERTY()
	TObjectPtr<ULyraEquipmentManagerComponent> EquipmentManagerComponent;

	void InitializeUIWidgets();
	void DeinitializeUIWidgets();

	// 记录当前选中 / 焦点槽位
	int32 FocusedInventorySlotIndex = INDEX_NONE;
	int32 FocusedQuickBarSlotIndex = INDEX_NONE;

	int32 SelectedInventorySlotIndex = INDEX_NONE;
	int32 SelectedQuickBarSlotIndex = INDEX_NONE;

	UPROPERTY()
	TArray<TObjectPtr<ULyraInventorySlot>> InventorySlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<ULyraInventorySlot>> QuickBarSlotWidgets;
};



#undef UE_API
