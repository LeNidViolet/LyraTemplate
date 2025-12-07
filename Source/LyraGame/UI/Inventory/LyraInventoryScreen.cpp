// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventoryScreen.h"


#include "CommonInputSubsystem.h"
#include "LyraGameplayTags.h"
#include "LyraInventoryDragVisualWidget.h"
#include "LyraInventorySlot.h"
#include "LyraLogChannels.h"
#include "Components/UniformGridPanel.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Equipment/LyraQuickBarComponent.h"
#include "Input/CommonUIInputTypes.h"
#include "Inventory/LyraInventoryFunctionLibrary.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "System/LyraGameData.h"
#include "System/LyraSystemStatics.h"
#include "UI/Misc/LyraUINaviSubsystem.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryScreen)


void ULyraInventoryScreen::NativeOnInitialized()
{
	bIsBackHandler = false;
	bIsBackActionDisplayedInActionBar = false;

	Super::NativeOnInitialized();

	// 注册 ActionBar 可用操作
	DropHandle = RegisterUIActionBinding(FBindUIActionArgs(DropAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleDropAction)));
	DropAllHandle = RegisterUIActionBinding(FBindUIActionArgs(DropAllAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleDropAllAction)));
	SelectHandle = RegisterUIActionBinding(FBindUIActionArgs(SelectAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleSelectAction)));
	BackHandle = RegisterUIActionBinding(FBindUIActionArgs(BackAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleBackAction)));
}

void ULyraInventoryScreen::NativeConstruct()
{
	RegisterMessageHandlers();

	// 输入方式变更的时候做一些额外的操作方式处理
	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		if (!InputMethodChangedHandle.IsValid())
		{
			InputMethodChangedHandle = InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
			HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
		}
	}

	InitializeComponents();
	InitializeUIWidgets();

	// TODO
	QuickBarSlotsGridPanel->SetIsEnabled(false);

	Super::NativeConstruct();
}

void ULyraInventoryScreen::NativeDestruct()
{
	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		if (InputMethodChangedHandle.IsValid())
		{
			InputSubsystem->OnInputMethodChangedNative.Remove(InputMethodChangedHandle);
			InputMethodChangedHandle.Reset();
		}
	}
	UnregisterMessageHandlers();
	DeinitializeUIWidgets();

	Super::NativeDestruct();
}

void ULyraInventoryScreen::NativeOnActivated()
{
	// 当 Inventory Screen 激活时, 禁用模拟摇杆导航
	Super::NativeOnActivated();
	ULyraUINaviSubsystem::SetAllowAnalogNavigation(false);
}

void ULyraInventoryScreen::NativeOnDeactivated()
{
	// 当 Inventory Screen 停用时, 恢复模拟摇杆导航
	ULyraUINaviSubsystem::SetAllowAnalogNavigation(true);
	Super::NativeOnDeactivated();
}

void ULyraInventoryScreen::HandleBackAction()
{
	// 准备关闭 Inventory Screen
	DeactivateWidget();
}

void ULyraInventoryScreen::HandleDropAction()
{
	// 丢弃(部分)物品
	DropActionDo(false);
}

void ULyraInventoryScreen::HandleDropAllAction()
{
	// 丢弃全部物品
	DropActionDo(true);
}

void ULyraInventoryScreen::HandleSelectAction()
{
	// 选中物品(只在手柄操作时使用)
}


// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotFocused(ULyraInventorySlot* SlotWidget)
{
	K2_OnInventorySlotFocused(SlotWidget);
	int32 SlotIndex = SlotWidget->GetSlotIndex();
	if (SlotIndex != FocusedInventorySlotIndex)
	{
		int32 OldSlotIndex = FocusedInventorySlotIndex;
		FocusedInventorySlotIndex = SlotIndex;
		HandleInventorySlotFocusChanged(SlotIndex, OldSlotIndex);
	}
}

// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotUnfocused(ULyraInventorySlot* SlotWidget)
{
	K2_OnInventorySlotUnfocused(SlotWidget);
	if (FocusedInventorySlotIndex == SlotWidget->GetSlotIndex())
	{
		int32 OldSlotIndex = FocusedInventorySlotIndex;
		FocusedInventorySlotIndex = INDEX_NONE;
		HandleInventorySlotFocusChanged(FocusedInventorySlotIndex, OldSlotIndex);
	}
}

// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotHovered(ULyraInventorySlot* SlotWidget)
{
	K2_OnInventorySlotHovered(SlotWidget);
	HandleInventorySlotFocused(SlotWidget);
}

// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotUnhovered(ULyraInventorySlot* SlotWidget)
{
	K2_OnInventorySlotUnhovered(SlotWidget);
	HandleInventorySlotUnfocused(SlotWidget);
}

// Inventory Slot 点击事件
void ULyraInventoryScreen::HandleInventorySlotClicked(ULyraInventorySlot* SlotWidget)
{
	K2_OnInventorySlotClicked(SlotWidget);
	SelectedInventorySlotIndex = SlotWidget->GetSlotIndex();
}




// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotFocused(ULyraInventorySlot* SlotWidget)
{
	K2_OnQuickBarSlotFocused(SlotWidget);
	int32 SlotIndex = SlotWidget->GetSlotIndex();
	if (SlotIndex != FocusedQuickBarSlotIndex)
	{
		int32 OldSlotIndex = FocusedQuickBarSlotIndex;
		FocusedQuickBarSlotIndex = SlotIndex;
		HandleQuickBarSlotFocusChanged(SlotIndex, OldSlotIndex);
	}
}

// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotUnfocused(ULyraInventorySlot* SlotWidget)
{
	K2_OnQuickBarSlotUnfocused(SlotWidget);
	if (FocusedQuickBarSlotIndex == SlotWidget->GetSlotIndex())
	{
		int32 OldSlotIndex = FocusedQuickBarSlotIndex;
		FocusedQuickBarSlotIndex = INDEX_NONE;
		HandleQuickBarSlotFocusChanged(FocusedQuickBarSlotIndex, OldSlotIndex);
	}
}

// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotHovered(ULyraInventorySlot* SlotWidget)
{
	K2_OnQuickBarSlotHovered(SlotWidget);
	HandleQuickBarSlotFocused(SlotWidget);
}

// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotUnhovered(ULyraInventorySlot* SlotWidget)
{
	K2_OnQuickBarSlotUnfovered(SlotWidget);
	HandleQuickBarSlotUnfocused(SlotWidget);
}

// QuickBar Slot 点击事件
void ULyraInventoryScreen::HandleQuickBarSlotClicked(ULyraInventorySlot* SlotWidget)
{
	K2_OnQuickBarSlotClicked(SlotWidget);
	SelectedQuickBarSlotIndex = SlotWidget->GetSlotIndex();
}



void ULyraInventoryScreen::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
	// 仅在使用手柄时启用选择操作
	if (NewInputMethod == ECommonInputType::Gamepad)
	{
		if (!GetActionBindings().Contains(SelectHandle))
		{
			AddActionBinding(SelectHandle);
		}
	}
	else
	{
		RemoveActionBinding(SelectHandle);
	}
}

void ULyraInventoryScreen::DropActionDo(bool DropAll)
{
	ULyraInventoryItemInstance* ItemInstance = InventoryManagerComponent->GetItemInstance(FocusedInventorySlotIndex);
	int32 ItemStackCount = InventoryManagerComponent->GetItemStackCount(FocusedInventorySlotIndex);
	if (!ItemInstance) return;
	if (ItemStackCount <= 0) return;

	int32 DropCount = ItemStackCount;

	if (DropAll)
	{
	}
	else
	{
		const TObjectPtr<UCurveFloat> &Curve = ULyraGameData::Get().InventoryDropCurve;
		if (!Curve)
		{
			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryScreen::DropAction failed: DropCurve is null"));
			return;
		}

		DropCount = ULyraSystemStatics::CalculateDropCount(ItemStackCount, Curve.Get());
		if (DropCount <= 0)
		{
			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryScreen::DropAction: Calculated DropCount is 0"));
			return;
		}
	}


	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn)
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryScreen::DropAction failed: OwningPlayerPawn is null"));
		return;
	}

	ULyraInventoryFunctionLibrary::DropInventoryItem(Pawn, ItemInstance, DropCount);
}


void ULyraInventoryScreen::RegisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (!InventoryStackChangedEventListener.IsValid())
			{
				// 如果道具发生变化, 则更新 UI
				InventoryStackChangedEventListener = MessageSubsystem->RegisterListener<FOnInventoryStackChangeParameters>(
					LyraGameplayTags::Gameplay_Message_Inventory_StackChanged,
					this,
					&ThisClass::HandleInventoryStackChangeEvent);
			}
		}
	}
}

void ULyraInventoryScreen::UnregisterMessageHandlers()
{
	if (InventoryStackChangedEventListener.IsValid())
	{
		InventoryStackChangedEventListener.Unregister();
	}
}

void ULyraInventoryScreen::HandleInventoryStackChangeEvent(FGameplayTag Channel, const FOnInventoryStackChangeParameters& Parameters)
{
	// 处理库存堆栈变化事件

	if (!InventoryManagerComponent)
	{
		return ;
	}


	// 根据不同的消息类型执行相应的操作
	switch (Parameters.MessageType)
	{
	case EInventoryStackChangeMessageType::EISCM_Add:
		{
			// 处理添加物品的逻辑
			break;
		}
	case EInventoryStackChangeMessageType::EISCM_Remove:
		{
			// 处理移除物品的逻辑
			break;
		}
	case EInventoryStackChangeMessageType::EISCM_Update:
		{
			// 处理更新物品数量的逻辑
			break;
		}
	case EInventoryStackChangeMessageType::EISCM_Swap:
		{
			// 处理交换物品位置的逻辑
			break;
		}
	default:
		{
			break;;
		}
	}

	// 调用蓝图事件以通知变化
	K2_OnInventoryStackChanged(Parameters);

	// 1. 添加物品
	// 2. 移除物品
	// 3. 更新物品数量
	// 4. 交换物品位置
}


ULyraInventorySlot* ULyraInventoryScreen::GetCurrentFocusedInventorySlot() const
{
	if (InventorySlotWidgets.IsValidIndex(FocusedInventorySlotIndex))
	{
		return InventorySlotWidgets[FocusedInventorySlotIndex];
	}
	return nullptr;
}

ULyraInventorySlot* ULyraInventoryScreen::GetCurrentFocusedQuickBarSlot() const
{
	if (QuickBarSlotWidgets.IsValidIndex(FocusedQuickBarSlotIndex))
	{
		return QuickBarSlotWidgets[FocusedQuickBarSlotIndex];
	}
	return nullptr;
}

void ULyraInventoryScreen::HandleInventorySlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex)
{
	K2_OnInventorySlotFocusChanged(NewFocusedSlotIndex, OldFocusedSlotIndex);
}

void ULyraInventoryScreen::HandleQuickBarSlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex)
{
	K2_OnQuickBarSlotFocusChanged(NewFocusedSlotIndex, OldFocusedSlotIndex);
}



void ULyraInventoryScreen::InitializeComponents()
{
	InventoryManagerComponent = GetOwningPlayer()->FindComponentByClass<ULyraInventoryManagerComponent>();
	ensure(InventoryManagerComponent);
	QuickBarComponent = GetOwningPlayer()->FindComponentByClass<ULyraQuickBarComponent>();
	ensure(QuickBarComponent);
	EquipmentManagerComponent = GetOwningPlayer()->FindComponentByClass<ULyraEquipmentManagerComponent>();
	ensure(EquipmentManagerComponent);
}

void ULyraInventoryScreen::InitializeUIWidgets()
{
	if (InventoryManagerComponent)
	{
		int32 InventoryCapacity = InventoryManagerComponent->GetInventoryCapacity();
		if (InventoryCapacity > 0)
		{
			int32 SlotsPerRow = ULyraGameData::Get().InventorySlotsPerRow;

			// 初始化库存槽位UI
			for (int32 i = 0; i < InventoryCapacity; i++)
			{
				ULyraInventorySlot* InventorySlot = CreateWidget<ULyraInventorySlot>(this, InventorySlotWidgetClass);
				// 初始化 slot
				InventorySlot->SetSlotIndex(i);

				// 添加到网格面板
				InventorySlotsGridPanel->AddChildToUniformGrid(InventorySlot, i / SlotsPerRow, i % SlotsPerRow);

				// 存储引用
				InventorySlotWidgets.Add(InventorySlot);

				InventorySlot->OnFocusReceived().AddUObject(this, &ThisClass::HandleInventorySlotFocused, InventorySlot);
				InventorySlot->OnFocusLost().AddUObject(this, &ThisClass::HandleInventorySlotUnfocused, InventorySlot);
				InventorySlot->OnHovered().AddUObject(this, &ThisClass::HandleInventorySlotHovered, InventorySlot);
				InventorySlot->OnUnhovered().AddUObject(this, &ThisClass::HandleInventorySlotUnhovered, InventorySlot);
				InventorySlot->OnClicked().AddUObject(this, &ThisClass::HandleInventorySlotClicked, InventorySlot);
			}
		}
	}

	if (EquipmentManagerComponent)
	{
		// int32 EquipmentCapacity = EquipmentManagerComponent->GetEquipmentCapacity();

		// TODO 暂时从GameData读取, EquipmentManagerComponent 还未整理结构
		int32 EquipmentCapacity = ULyraGameData::Get().QuickBarMaxCapacity;
		if (EquipmentCapacity > 0)
		{
			int32 SlotsPerRow = ULyraGameData::Get().QuickBarSlotsPerRow;

			// 初始化快捷栏槽位UI
			for (int32 i = 0; i < EquipmentCapacity; i++)
			{
				ULyraInventorySlot* QuickBarSlot = CreateWidget<ULyraInventorySlot>(this, QuickBarSlotWidgetClass);
				// 初始化 slot
				QuickBarSlot->SetSlotIndex(i);

				// 添加到网格面板
				QuickBarSlotsGridPanel->AddChildToUniformGrid(QuickBarSlot, i / SlotsPerRow, i % SlotsPerRow);

				// 存储引用
				QuickBarSlotWidgets.Add(QuickBarSlot);

				QuickBarSlot->OnFocusReceived().AddUObject(this, &ThisClass::HandleQuickBarSlotFocused, QuickBarSlot);
				QuickBarSlot->OnFocusLost().AddUObject(this, &ThisClass::HandleQuickBarSlotUnfocused, QuickBarSlot);
				QuickBarSlot->OnHovered().AddUObject(this, &ThisClass::HandleQuickBarSlotHovered, QuickBarSlot);
				QuickBarSlot->OnUnhovered().AddUObject(this, &ThisClass::HandleQuickBarSlotUnhovered, QuickBarSlot);
				QuickBarSlot->OnClicked().AddUObject(this, &ThisClass::HandleQuickBarSlotClicked, QuickBarSlot);
			}
		}
	}
}

void ULyraInventoryScreen::DeinitializeUIWidgets()
{
	// 移除所有注册的回调
	for (TObjectPtr<ULyraInventorySlot>& SlotWidget : InventorySlotWidgets)
	{
		SlotWidget->OnFocusReceived().RemoveAll(this);
		SlotWidget->OnFocusLost().RemoveAll(this);
		SlotWidget->OnHovered().RemoveAll(this);
		SlotWidget->OnUnhovered().RemoveAll(this);
		SlotWidget->OnClicked().RemoveAll(this);
	}

	for (TObjectPtr<ULyraInventorySlot>& SlotWidget : QuickBarSlotWidgets)
	{
		SlotWidget->OnFocusReceived().RemoveAll(this);
		SlotWidget->OnFocusLost().RemoveAll(this);
		SlotWidget->OnHovered().RemoveAll(this);
		SlotWidget->OnUnhovered().RemoveAll(this);
		SlotWidget->OnClicked().RemoveAll(this);
	}

	InventorySlotsGridPanel->ClearChildren();
	InventorySlotWidgets.Empty();

	QuickBarSlotsGridPanel->ClearChildren();
	QuickBarSlotWidgets.Empty();
}

