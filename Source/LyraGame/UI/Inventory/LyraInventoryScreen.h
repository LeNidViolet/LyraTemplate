// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/LyraActivatableWidget.h"
#include "LyraInventoryScreen.generated.h"


#define UE_API LYRAGAME_API


class ULyraInventoryItemInstance;
class ULyraInventorySlot;
struct FOnInventoryStackChangeParameters;
class ULyraEquipmentManagerComponent;
class ULyraQuickBarComponent;
class ULyraInventoryManagerComponent;
enum class ECommonInputType : uint8;


/**
 * 当使用手柄操作时, 需要提供一个选择模式, 以支持 Swap, Stack 的操作
 * 当按下返回按钮时, 如果使用的手柄操作, 则先退出选择模式, 然后才能关闭界面
 */


UCLASS(MinimalAPI)
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
	UE_API void HandleInventorySlotUnfocused(ULyraInventorySlot* SlotWidget);
	UE_API void HandleInventorySlotHovered(ULyraInventorySlot* SlotWidget);
	UE_API void HandleInventorySlotUnhovered(ULyraInventorySlot* SlotWidget);


	UE_API void HandleInventorySlotClicked(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotClicked(ULyraInventorySlot* SlotWidget);





	// Quick Slot相关事件
	UE_API void HandleQuickBarSlotFocused(ULyraInventorySlot* SlotWidget);
	UE_API void HandleQuickBarSlotUnfocused(ULyraInventorySlot* SlotWidget);
	UE_API void HandleQuickBarSlotHovered(ULyraInventorySlot* SlotWidget);
	UE_API void HandleQuickBarSlotUnhovered(ULyraInventorySlot* SlotWidget);



	UE_API void HandleQuickBarSlotClicked(ULyraInventorySlot* SlotWidget);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotClicked(ULyraInventorySlot* SlotWidget);


	UE_API void HandleInventorySlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventorySlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);

	UE_API void HandleQuickBarSlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnQuickBarSlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex);


	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API ULyraInventorySlot* GetCurrentFocusedInventorySlot() const;
	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API ULyraInventorySlot* GetCurrentFocusedQuickBarSlot() const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API ULyraInventoryItemInstance* GetCurrentFocusedInventoryItemInstance() const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API bool IsUsingGamepad() const { return bIsUsingGamepad; }
	UFUNCTION(BlueprintPure, Category="Inventory")
	UE_API bool IsInSelectMode() const { return bIsInSelectMode; }


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

	FText DropActionName;
	void UpdateDropActionName();

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

	// 记录当前焦点 slot, 上一个焦点 slot
	int32 PreviousFocusedInventorySlotIndex = INDEX_NONE;
	int32 PreviousFocusedQuickBarSlotIndex = INDEX_NONE;
	int32 CurrentFocusedInventorySlotIndex = INDEX_NONE;
	int32 CurrentFocusedQuickBarSlotIndex = INDEX_NONE;

	// 记录当前选中 slot
	int32 CurrentSelectedInventorySlotIndex = INDEX_NONE;
	int32 CurrentSelectedQuickBarSlotIndex = INDEX_NONE;

	void EnterOrExitSelectMode();
	void EnterSelectMode();
	void ExitSelectMode();

	// 当前是否是手柄操作
	bool bIsUsingGamepad = false;

	// 当前是否处于选择模式
	bool bIsInSelectMode = false;

	UPROPERTY()
	TArray<TObjectPtr<ULyraInventorySlot>> InventorySlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<ULyraInventorySlot>> QuickBarSlotWidgets;
};



#undef UE_API
