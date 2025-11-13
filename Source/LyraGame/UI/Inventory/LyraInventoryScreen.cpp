// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventoryScreen.h"

#include "CommonInputSubsystem.h"
#include "LyraGameplayTags.h"
#include "LyraInventorySlot.h"
#include "LyraLogChannels.h"
#include "Components/UniformGridPanel.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Equipment/LyraQuickBarComponent.h"
#include "Input/CommonUIInputTypes.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Messages/LyraNotificationMessage_Inventory.h"
#include "UI/Misc/LyraUINaviSubsystem.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryScreen)


void ULyraInventoryScreen::NativeOnInitialized()
{
	bIsBackHandler = false;
	bIsBackActionDisplayedInActionBar = false;

	Super::NativeOnInitialized();

	// 注册 ActionBar 可用操作
	DropHandle = RegisterUIActionBinding(FBindUIActionArgs(DropAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleDropAction)));
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
	Super::NativeOnActivated();
	ULyraUINaviSubsystem::SetAllowAnalogNavigation(false);
}

void ULyraInventoryScreen::NativeOnDeactivated()
{
	ULyraUINaviSubsystem::SetAllowAnalogNavigation(true);
	Super::NativeOnDeactivated();
}

void ULyraInventoryScreen::HandleBackAction()
{
	DeactivateWidget();
}

void ULyraInventoryScreen::HandleDropAction()
{

}

void ULyraInventoryScreen::HandleSelectAction()
{

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
					LyraGameplayTags::Inventory_Stack_Changed,
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
			UE_LOG(LogLyraInventory, Log,
				   TEXT("LyraInventory ====> "
					   "ULyraInventoryScreen::HandleInventoryStackChangeEvent: "
					   "Item Added: %s, New Count: %d"),
				   *GetNameSafe(Parameters.Instance),
				   Parameters.NewCount);
			break;
		}
	case EInventoryStackChangeMessageType::EISCM_Remove:
		{
			// 处理移除物品的逻辑
			UE_LOG(LogLyraInventory, Log,
				   TEXT("LyraInventory ====> "
					   "ULyraInventoryScreen::HandleInventoryStackChangeEvent: "
					   "Item Removed: %s"),
				   *GetNameSafe(Parameters.Instance));
			break;
		}
	case EInventoryStackChangeMessageType::EISCM_Update:
		{
			// 处理更新物品数量的逻辑
			UE_LOG(LogLyraInventory, Log,
				   TEXT("LyraInventory ====> "
					   "ULyraInventoryScreen::HandleInventoryStackChangeEvent: "
					   "Item Updated: %s, New Count: %d, Delta: %d"),
				   *GetNameSafe(Parameters.Instance),
				   Parameters.NewCount,
				   Parameters.Delta);
			break;
		}
	case EInventoryStackChangeMessageType::EISCM_Swap:
		{
			// 处理交换物品位置的逻辑
			UE_LOG(LogLyraInventory, Log,
				   TEXT("LyraInventory ====> "
					   "ULyraInventoryScreen::HandleInventoryStackChangeEvent: "
					   "Item Swapped: %s"),
				   *GetNameSafe(Parameters.Instance));
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
			// 初始化库存槽位UI
			for (int32 i = 0; i < InventoryCapacity; i++)
			{
				ULyraInventorySlot* InventorySlot = CreateWidget<ULyraInventorySlot>(this, InventorySlotWidgetClass);
				// 初始化 slot
				InventorySlot->SetSlotIndex(i);

				// 添加到网格面板
				InventorySlotsGridPanel->AddChildToUniformGrid(InventorySlot, i / InventorySlotsPerRow, i % InventorySlotsPerRow);

				// 存储引用
				InventorySlotWidgets.Add(InventorySlot);
			}
		}
	}

	if (EquipmentManagerComponent)
	{
		// int32 EquipmentCapacity = EquipmentManagerComponent->GetEquipmentCapacity();
	}
}

void ULyraInventoryScreen::DeinitializeUIWidgets()
{
	InventorySlotsGridPanel->ClearChildren();
	InventorySlotWidgets.Empty();
}

