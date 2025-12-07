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

void ULyraInventorySlot::SetVisualState(EInventorySlotVisualState NewState)
{
	if (NewState != CurrentVisualState)
	{
		CurrentVisualState = NewState;
		K2_UpdateSlot();
	}
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

	LastDragActionState = EInventorySlotOperationType::EISO_None;

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

	EInventorySlotOperationType Operation = ULyraInventoryFunctionLibrary::GetInventorySlotOperationType(
		InventoryManager.Get(),
		DragOperation->SourceSlotIndex,
		SlotIndex);

	LastDragActionState = Operation;
	DragVisual->SetDragOperationType(this, Operation);
}

void ULyraInventorySlot::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	LastDragActionState = EInventorySlotOperationType::EISO_None;

	// 拖拽操作离开当前slot
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	if (!InventoryManager) return ;

	// 获取 Operation 和 Visual
	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return ;

	if (DragOperation->SourceSlotIndex == SlotIndex) return;

	ULyraInventoryDragVisualWidget* DragVisual = Cast<ULyraInventoryDragVisualWidget>(DragOperation->DefaultDragVisual);
	if (!DragVisual) return ;

	DragVisual->SetDragOperationType(this, LastDragActionState);
}

bool ULyraInventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	// 拖拽操作放下到当前slot
	if (LastDragActionState == EInventorySlotOperationType::EISO_None) return false;

	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return false;

	// 根据最后测定的操作类型来执行对应的逻辑
	if (LastDragActionState == EInventorySlotOperationType::EISO_Stack)
	{
		// 堆叠物品
		return HandleDropStackOperation(DragOperation);
	}
	if (LastDragActionState == EInventorySlotOperationType::EISO_Swap)
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
