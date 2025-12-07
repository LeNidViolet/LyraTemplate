// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventorySlot.h"

#include "LyraGameplayTags.h"
#include "LyraInventoryDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Messages/LyranotificationMessage_Inventory.h"
#include "Inventory/LyraInventoryFunctionLibrary.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventorySlot)

ULyraInventorySlot::ULyraInventorySlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraInventorySlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void ULyraInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();
	RegisterMessageHandlers();
	InitializeSlot();
}

void ULyraInventorySlot::NativeDestruct()
{
	UnregisterMessageHandlers();
	Super::NativeDestruct();
}

void ULyraInventorySlot::RegisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (!InventoryStackChangedEventListener.IsValid())
			{
				// 如果道具/数量发生变化, 则更新 UI
				InventoryStackChangedEventListener = MessageSubsystem->RegisterListener<FOnInventoryStackChangeParameters>(
					LyraGameplayTags::Gameplay_Message_Inventory_StackChanged,
					this,
					&ThisClass::HandleInventoryStackChangeEvent);
			}
		}
	}
}

void ULyraInventorySlot::UnregisterMessageHandlers()
{
	if (InventoryStackChangedEventListener.IsValid())
	{
		InventoryStackChangedEventListener.Unregister();
	}
}

void ULyraInventorySlot::HandleInventoryStackChangeEvent(FGameplayTag Channel, const FOnInventoryStackChangeParameters& Parameters)
{
	if (Parameters.SlotIndex != SlotIndex)
	{
		return ;
	}

	K2_UpdateSlot();
}

ULyraInventoryItemInstance* ULyraInventorySlot::GetInventoryItemInstance() const
{
	// 获取当前slot存储的物品实例
	return InventoryManager ? InventoryManager->GetItemInstance(SlotIndex) : nullptr;
}

int32 ULyraInventorySlot::GetInventoryItemStackCount() const
{
	// 获取当前slot存储的物品堆叠数量
	return InventoryManager ? InventoryManager->GetItemStackCount(SlotIndex) : 0;
}

void ULyraInventorySlot::InitializeSlot()
{
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller) return;
	InventoryManager = Controller->FindComponentByClass<ULyraInventoryManagerComponent>();
	if (!InventoryManager) return;

	K2_UpdateSlot();
}

FReply ULyraInventorySlot::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 发起拖拽
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void ULyraInventorySlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// 拖拽开始

	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!InventoryManager) return ;

	// 当前slot没有物品, 不允许拖拽
	ULyraInventoryItemInstance* ItemInstance = InventoryManager->GetItemInstance(SlotIndex);
	if (!ItemInstance) return ;

	ULyraInventoryDragVisualWidget* DragVisualWidget = CreateWidget<ULyraInventoryDragVisualWidget>(GetOwningPlayer(), DragVisualClass);
	DragVisualWidget->SetSlotSize(GetCachedGeometry().GetLocalSize());
	DragVisualWidget->SetDraggedItemInstance(ItemInstance);

	ULyraInventoryDragDropOperation* DragOperation = NewObject<ULyraInventoryDragDropOperation>();
	DragOperation->SourceSlotIndex = SlotIndex;
	DragOperation->ItemInstance = ItemInstance;
	DragOperation->DefaultDragVisual = DragVisualWidget;
	OutOperation = DragOperation;
}

void ULyraInventorySlot::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 拖拽操作进入当前slot

	LastDragActionState = EDragActionState::EDAS_None;

	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	if (!InventoryManager) return ;

	// 获取 Operation 和 Visual
	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return ;
	ULyraInventoryDragVisualWidget* DragVisual = Cast<ULyraInventoryDragVisualWidget>(DragOperation->DefaultDragVisual);
	if (!DragVisual) return ;

	// 忽略自己拖拽到自己
	if (DragOperation->SourceSlotIndex == SlotIndex) return;

	if (!DragOperation->ItemInstance) return ;

	// 这里的逻辑是: 检测正在拖拽的物品类型与当前slot的物品类型是否相同
	// 如果相同, 且可以堆叠, 则显示堆叠提示; 否则显示交换提示
	EDragActionState DragActionState = EDragActionState::EDAS_None;
	ULyraInventoryItemInstance* CurrentSlotItemInstance = InventoryManager->GetItemInstance(SlotIndex);
	if (!CurrentSlotItemInstance)
	{
		// 当前Slot没有物品, 只能交换
		DragActionState = EDragActionState::EDAS_Swap;
	}
	else
	{
		// 如果物品类型不同, 只能交换
		if (CurrentSlotItemInstance->GetItemDef() != DragOperation->ItemInstance->GetItemDef())
		{
			DragActionState = EDragActionState::EDAS_Swap;
		}
		else
		{
			// 物品类型相同, 检测是否可以堆叠
			const ULyraInventoryItemDefinition* ItemDef = GetDefault<ULyraInventoryItemDefinition>(DragOperation->ItemInstance->GetItemDef());
			if (!ItemDef->bAllowStacking)
			{
				// 不允许堆叠
				DragActionState = EDragActionState::EDAS_Swap;
			}
			else
			{
				// 允许堆叠, 检测当前slot的堆叠数量是否已满
				int32 CurrentStackCount = InventoryManager->GetItemStackCount(SlotIndex);
				if (CurrentStackCount >= ItemDef->MaxStackCount)
				{
					// 已满, 只能交换
					DragActionState = EDragActionState::EDAS_Swap;
				}
				else
				{
					// 可以堆叠
					DragActionState = EDragActionState::EDAS_Stack;
				}
			}
		}
	}

	LastDragActionState = DragActionState;
	DragVisual->SetDragActionState(this, DragActionState);
}

void ULyraInventorySlot::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	LastDragActionState = EDragActionState::EDAS_None;

	// 拖拽操作离开当前slot
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	if (!InventoryManager) return ;

	// 获取 Operation 和 Visual
	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return ;

	if (DragOperation->SourceSlotIndex == SlotIndex) return;

	ULyraInventoryDragVisualWidget* DragVisual = Cast<ULyraInventoryDragVisualWidget>(DragOperation->DefaultDragVisual);
	if (!DragVisual) return ;

	DragVisual->SetDragActionState(this, EDragActionState::EDAS_None);
}

bool ULyraInventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 拖拽操作放下到当前slot
	if (LastDragActionState == EDragActionState::EDAS_None) return false;

	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return false;

	// 根据最后测定的操作类型来执行对应的逻辑
	if (LastDragActionState == EDragActionState::EDAS_Stack)
	{
		// 堆叠物品
		return HandleDropStackOperation(DragOperation);
	}
	if (LastDragActionState == EDragActionState::EDAS_Swap)
	{
		// 交换物品
		return HandleDropSwapOperation(DragOperation);
	}
	return false;
}

bool ULyraInventorySlot::HandleDropSwapOperation(ULyraInventoryDragDropOperation* DragOperation)
{
	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return false;

	int32 SourceSlotIndex = DragOperation->SourceSlotIndex;
	int32 TargetSlotIndex = SlotIndex;
	return ULyraInventoryFunctionLibrary::SwapInventoryItem(
		Pawn,
		DragOperation->ItemInstance,
		SourceSlotIndex,
		TargetSlotIndex);
}

bool ULyraInventorySlot::HandleDropStackOperation(ULyraInventoryDragDropOperation* DragOperation)
{
	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return false;

	int32 SourceSlotIndex = DragOperation->SourceSlotIndex;
	int32 TargetSlotIndex = SlotIndex;
	return ULyraInventoryFunctionLibrary::StackInventoryItem(
		Pawn,
		DragOperation->ItemInstance,
		SourceSlotIndex,
		TargetSlotIndex);
}
