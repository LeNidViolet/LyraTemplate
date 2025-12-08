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
#include "UI/Foundation/LyraActionWidget.h"
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

	DropActionName = DropHandle.GetDisplayName();
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

	// 停止可能存在的拖拽操作
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().CancelDragDrop();
	}
}

void ULyraInventoryScreen::HandleBackAction()
{
	if (bIsInSelectMode)
	{
		ensure(bIsUsingGamepad);
		// 如果是手柄操作且处于选择模式, 则先退出选择模式
		EnterOrExitSelectMode();
		return;
	}
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
	if (!bIsUsingGamepad) return;

	ULyraInventoryItemInstance* ItemInstance = InventoryManagerComponent->GetItemInstance(CurrentFocusedInventorySlotIndex);
	if (!ItemInstance && !bIsInSelectMode)
	{
		// 当前slot没有物品, 且不处于选择模式, 则不进行任何操作
		return;
	}

	// 如果选择的是自身, 则退出选择模式, 如果不是, 则进行相应的操作
	if (CurrentFocusedInventorySlotIndex != CurrentSelectedInventorySlotIndex && CurrentSelectedInventorySlotIndex != INDEX_NONE)
	{
		// 需要针对目标slot进行操作
		EInventorySlotOperationType OperationType = ULyraInventoryFunctionLibrary::GetInventorySlotOperationType(
			InventoryManagerComponent.Get(),
			CurrentSelectedInventorySlotIndex,
			CurrentFocusedInventorySlotIndex);
		switch (OperationType)
		{
		case EInventorySlotOperationType::EISO_Stack:
			ULyraInventoryFunctionLibrary::StackInventoryItem(
				GetOwningPlayerPawn(),
				InventoryManagerComponent->GetItemInstance(CurrentSelectedInventorySlotIndex),
				CurrentSelectedInventorySlotIndex,
				CurrentFocusedInventorySlotIndex);
			break;
		case EInventorySlotOperationType::EISO_Swap:
			ULyraInventoryFunctionLibrary::SwapInventoryItem(
				GetOwningPlayerPawn(),
				InventoryManagerComponent->GetItemInstance(CurrentSelectedInventorySlotIndex),
				CurrentSelectedInventorySlotIndex,
				CurrentFocusedInventorySlotIndex);
			break;
		default:
			// 无法进行操作, 直接退出选择模式
			break;
		}
	}


	// 进入/退出选择模式
	EnterOrExitSelectMode();
}


// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotFocused(ULyraInventorySlot* SlotWidget)
{
	int32 SlotIndex = SlotWidget->GetSlotIndex();
	if (SlotIndex != CurrentFocusedInventorySlotIndex)
	{
		PreviousFocusedInventorySlotIndex = CurrentFocusedInventorySlotIndex;
		CurrentFocusedInventorySlotIndex = SlotIndex;
		HandleInventorySlotFocusChanged(CurrentFocusedInventorySlotIndex, PreviousFocusedInventorySlotIndex);
	}

	UpdateDropActionName();
}

// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotUnfocused(ULyraInventorySlot* SlotWidget)
{
	if (CurrentFocusedInventorySlotIndex == SlotWidget->GetSlotIndex())
	{
		PreviousFocusedInventorySlotIndex = CurrentFocusedInventorySlotIndex;
		CurrentFocusedInventorySlotIndex = INDEX_NONE;
		HandleInventorySlotFocusChanged(CurrentFocusedInventorySlotIndex, PreviousFocusedInventorySlotIndex);
	}

	UpdateDropActionName();
}

// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotHovered(ULyraInventorySlot* SlotWidget)
{
	HandleInventorySlotFocused(SlotWidget);
}

// Inventory 追踪焦点槽位
void ULyraInventoryScreen::HandleInventorySlotUnhovered(ULyraInventorySlot* SlotWidget)
{
	HandleInventorySlotUnfocused(SlotWidget);
}

// Inventory Slot 点击事件
void ULyraInventoryScreen::HandleInventorySlotClicked(ULyraInventorySlot* SlotWidget)
{
	K2_OnInventorySlotClicked(SlotWidget);
	CurrentSelectedInventorySlotIndex = SlotWidget->GetSlotIndex();
}




// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotFocused(ULyraInventorySlot* SlotWidget)
{
	int32 SlotIndex = SlotWidget->GetSlotIndex();
	if (SlotIndex != CurrentFocusedQuickBarSlotIndex)
	{
		PreviousFocusedQuickBarSlotIndex = CurrentFocusedQuickBarSlotIndex;
		CurrentFocusedQuickBarSlotIndex = SlotIndex;
		HandleQuickBarSlotFocusChanged(PreviousFocusedQuickBarSlotIndex, CurrentFocusedQuickBarSlotIndex);
	}
}

// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotUnfocused(ULyraInventorySlot* SlotWidget)
{
	if (CurrentFocusedQuickBarSlotIndex == SlotWidget->GetSlotIndex())
	{
		PreviousFocusedQuickBarSlotIndex = CurrentFocusedQuickBarSlotIndex;
		CurrentFocusedQuickBarSlotIndex = INDEX_NONE;
		HandleQuickBarSlotFocusChanged(CurrentFocusedQuickBarSlotIndex, PreviousFocusedQuickBarSlotIndex);
	}
}

// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotHovered(ULyraInventorySlot* SlotWidget)
{
	HandleQuickBarSlotFocused(SlotWidget);
}

// QuickBar 追踪焦点槽位
void ULyraInventoryScreen::HandleQuickBarSlotUnhovered(ULyraInventorySlot* SlotWidget)
{
	HandleQuickBarSlotUnfocused(SlotWidget);
}

// QuickBar Slot 点击事件
void ULyraInventoryScreen::HandleQuickBarSlotClicked(ULyraInventorySlot* SlotWidget)
{
	K2_OnQuickBarSlotClicked(SlotWidget);
	CurrentSelectedQuickBarSlotIndex = SlotWidget->GetSlotIndex();
}


void ULyraInventoryScreen::UpdateDropActionName()
{
	int32 DropCount = 0;
	int32 StackCount = 0;
	ULyraInventoryItemInstance* ItemInstance = InventoryManagerComponent->GetItemInstance(CurrentFocusedInventorySlotIndex);
	if (ItemInstance)
	{
		StackCount = InventoryManagerComponent->GetItemStackCount(CurrentFocusedInventorySlotIndex);
		if (StackCount > 1)
		{
			const TObjectPtr<UCurveFloat> &Curve = ULyraGameData::Get().InventoryDropCurve;
			if (Curve)
			{
				DropCount = ULyraSystemStatics::CalculateDropCount(StackCount, Curve.Get());
			}
		}
	}

	// 更新丢弃操作显示名称 (如果数量大于1)
	if (DropCount > 1 || StackCount > 1)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Count"), DropCount);
		FText NewDropActionName = FText::Format(NSLOCTEXT("LyraInventory", "DropActionWithCount", "Drop ({Count})"), Args);
		DropHandle.SetDisplayName(NewDropActionName);
	}
	else
	{
		DropHandle.SetDisplayName(DropActionName);
	}
}

void ULyraInventoryScreen::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
	// 仅在使用手柄时启用选择操作
	bIsUsingGamepad = (NewInputMethod == ECommonInputType::Gamepad);
	if (bIsUsingGamepad)
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

	if (bIsInSelectMode)
		ExitSelectMode();
}

void ULyraInventoryScreen::DropActionDo(bool DropAll)
{
	ULyraInventoryItemInstance* ItemInstance = InventoryManagerComponent->GetItemInstance(CurrentFocusedInventorySlotIndex);
	int32 ItemStackCount = InventoryManagerComponent->GetItemStackCount(CurrentFocusedInventorySlotIndex);
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

	UpdateDropActionName();
}


ULyraInventorySlot* ULyraInventoryScreen::GetCurrentFocusedInventorySlot() const
{
	if (InventorySlotWidgets.IsValidIndex(CurrentFocusedInventorySlotIndex))
	{
		return InventorySlotWidgets[CurrentFocusedInventorySlotIndex];
	}
	return nullptr;
}

ULyraInventorySlot* ULyraInventoryScreen::GetCurrentFocusedQuickBarSlot() const
{
	if (QuickBarSlotWidgets.IsValidIndex(CurrentFocusedQuickBarSlotIndex))
	{
		return QuickBarSlotWidgets[CurrentFocusedQuickBarSlotIndex];
	}
	return nullptr;
}

ULyraInventoryItemInstance* ULyraInventoryScreen::GetCurrentFocusedInventoryItemInstance() const
{
	ULyraInventorySlot* SlotWidget = GetCurrentFocusedInventorySlot();
	if (SlotWidget)
	{
		return InventoryManagerComponent->GetItemInstance(SlotWidget->GetSlotIndex());
	}
	return nullptr;
}

void ULyraInventoryScreen::HandleInventorySlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex)
{
	K2_OnInventorySlotFocusChanged(NewFocusedSlotIndex, OldFocusedSlotIndex);

	// 如果没有处于选择模式, 则旧slot去焦, 新slot获焦
	if (!bIsInSelectMode)
	{
		if (InventorySlotWidgets.IsValidIndex(OldFocusedSlotIndex))
		{
			InventorySlotWidgets[OldFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_Default);
		}
		if (InventorySlotWidgets.IsValidIndex(NewFocusedSlotIndex))
		{
			InventorySlotWidgets[NewFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_Focused);
		}
	}
	else
	{
		// 处于选择模式, 选中的slot保持焦点状态, 当前slot根据可进行的操作显示不同状态
		if (CurrentSelectedInventorySlotIndex != OldFocusedSlotIndex)
		{
			if (InventorySlotWidgets.IsValidIndex(OldFocusedSlotIndex))
			{
				InventorySlotWidgets[OldFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_Default);
			}
		}

		if (InventorySlotWidgets.IsValidIndex(NewFocusedSlotIndex))
		{
			if (CurrentSelectedInventorySlotIndex != NewFocusedSlotIndex)
			{
				// 根据可进行的操作显示不同状态
				EInventorySlotOperationType OperationType = ULyraInventoryFunctionLibrary::GetInventorySlotOperationType(
					InventoryManagerComponent.Get(),
					CurrentSelectedInventorySlotIndex,
					NewFocusedSlotIndex);
				switch (OperationType)
				{
				case EInventorySlotOperationType::EISO_Stack:
					InventorySlotWidgets[NewFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_AbleToStack);
					break;
				case EInventorySlotOperationType::EISO_Swap:
					InventorySlotWidgets[NewFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_AbleToSwap);
					break;
				default:
					InventorySlotWidgets[NewFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_Default);
					break;
				}
			}
			else
			{
				InventorySlotWidgets[NewFocusedSlotIndex]->SetVisualState(EInventorySlotVisualState::EISVS_SelectFocused);
			}
		}
	}
}

void ULyraInventoryScreen::HandleQuickBarSlotFocusChanged(int32 NewFocusedSlotIndex, int32 OldFocusedSlotIndex)
{
	// TODO
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

void ULyraInventoryScreen::EnterOrExitSelectMode()
{
	if (bIsInSelectMode)
	{
		ExitSelectMode();
	}
	else
	{
		EnterSelectMode();
	}
}

void ULyraInventoryScreen::EnterSelectMode()
{
	ensure(bIsInSelectMode == false);
	bIsInSelectMode = true;
	CurrentSelectedInventorySlotIndex = CurrentFocusedInventorySlotIndex;
	HandleInventorySlotFocusChanged(CurrentFocusedInventorySlotIndex, PreviousFocusedInventorySlotIndex);


	RemoveActionBinding(DropAllHandle);
	RemoveActionBinding(DropHandle);

	// TODO QuickBar
}

void ULyraInventoryScreen::ExitSelectMode()
{
	ensure(bIsInSelectMode == true);
	bIsInSelectMode = false;

	// 退出选择模式后, 选中slot恢复默认状态, 当前slot恢复焦点状态
	if (InventorySlotWidgets.IsValidIndex(CurrentSelectedInventorySlotIndex))
	{
		InventorySlotWidgets[CurrentSelectedInventorySlotIndex]->SetVisualState(
			CurrentSelectedInventorySlotIndex != CurrentFocusedInventorySlotIndex ?
			EInventorySlotVisualState::EISVS_Default : EInventorySlotVisualState::EISVS_Focused);
	}

	CurrentSelectedInventorySlotIndex = INDEX_NONE;
	HandleInventorySlotFocusChanged(CurrentFocusedInventorySlotIndex, PreviousFocusedInventorySlotIndex);


	if (!GetActionBindings().Contains(DropAllHandle))
	{
		AddActionBinding(DropAllHandle);
	}
	if (!GetActionBindings().Contains(DropHandle))
	{
		AddActionBinding(DropHandle);
	}

	// TODO QuickBar
}

