// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventoryDragVisualWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryDragVisualWidget)

ULyraInventoryDragVisualWidget::ULyraInventoryDragVisualWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraInventoryDragVisualWidget::SetDraggedItemInstance(class ULyraInventoryItemInstance* InItemInstance)
{
	DraggedItemInstance = InItemInstance;
	K2_UpdateDragVisual();
}

void ULyraInventoryDragVisualWidget::SetSlotSize(const FVector2D& InSlotSize)
{
	SlotSize = InSlotSize;
	K2_UpdateSlotSize();
}

void ULyraInventoryDragVisualWidget::SetDragOperationType(UUserWidget* TriggerWidget, EInventorySlotOperationType OperationType)
{
	if (!TriggerWidget) return;

	// 记录最后更新的Widget, 如果当前状态已经由另外的Widget做出了变动, 之前的Widget再来设置None时就忽略它
	if (OperationType != EInventorySlotOperationType::EISO_None)
	{
		LastWidget = TriggerWidget;
	}
	else
	{
		if (LastWidget != TriggerWidget)
		{
			return ;
		}
	}

	DragOperationType = OperationType;
	K2_UpdateActionState();
}
