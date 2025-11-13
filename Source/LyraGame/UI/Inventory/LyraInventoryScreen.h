// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	// 选中物品
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	FDataTableRowHandle SelectAction;

	UE_API void HandleBackAction();
	UE_API void HandleDropAction();
	UE_API void HandleSelectAction();

	// 通知蓝图库存变化事件
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	UE_API void K2_OnInventoryStackChanged(const FOnInventoryStackChangeParameters& Parameters);


	// Slot Widget Class
	UPROPERTY(EditDefaultsOnly, Category="Inventory")
	TSubclassOf<ULyraInventorySlot> InventorySlotWidgetClass;

	// Inventory 每一行排列几个槽位
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory")
	int32 InventorySlotsPerRow = 8;

	// Inventory Slots Grid Panel
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<class UUniformGridPanel> InventorySlotsGridPanel;

private:
	FUIActionBindingHandle BackHandle;
	FUIActionBindingHandle DropHandle;
	FUIActionBindingHandle SelectHandle;

	FDelegateHandle InputMethodChangedHandle;
	void HandleInputMethodChanged(ECommonInputType NewInputMethod);


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

	UPROPERTY()
	TArray<TObjectPtr<ULyraInventorySlot>> InventorySlotWidgets;
};



#undef UE_API
