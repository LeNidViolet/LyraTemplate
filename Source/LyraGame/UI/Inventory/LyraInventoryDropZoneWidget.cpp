// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventoryDropZoneWidget.h"

#include "LyraInventoryDragDropOperation.h"
#include "LyraInventoryDragVisualWidget.h"
#include "Inventory/LyraInventoryFunctionLibrary.h"
#include "Inventory/LyraInventoryManagerComponent.h"

ULyraInventoryDropZoneWidget::ULyraInventoryDropZoneWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetVisibility(ESlateVisibility::Visible);
}

void ULyraInventoryDropZoneWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return ;
	ULyraInventoryDragVisualWidget* DragVisual = Cast<ULyraInventoryDragVisualWidget>(DragOperation->DefaultDragVisual);
	if (!DragVisual) return ;

	DragVisual->SetDragOperationType(this, EInventorySlotOperationType::EISO_Drop);
}

void ULyraInventoryDropZoneWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return ;
	ULyraInventoryDragVisualWidget* DragVisual = Cast<ULyraInventoryDragVisualWidget>(DragOperation->DefaultDragVisual);
	if (!DragVisual) return ;

	DragVisual->SetDragOperationType(this, EInventorySlotOperationType::EISO_None);
}

bool ULyraInventoryDropZoneWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ULyraInventoryDragDropOperation* DragOperation = Cast<ULyraInventoryDragDropOperation>(InOperation);
	if (!DragOperation) return false;

	if (!DragOperation->ItemInstance) return false;
	APlayerController* Controller = GetOwningPlayer();
	if (!Controller) return false;
	ULyraInventoryManagerComponent* InventoryManager = Controller->FindComponentByClass<ULyraInventoryManagerComponent>();
	if (!InventoryManager) return false;

	ULyraInventoryItemInstance* ExistsItemInstance = InventoryManager->GetItemInstance(DragOperation->SourceSlotIndex);
	if (!ExistsItemInstance) return false;
	if (ExistsItemInstance != DragOperation->ItemInstance) return false;

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return false;

	ULyraInventoryFunctionLibrary::DropInventoryItem(Pawn, DragOperation->ItemInstance, InventoryManager->GetItemStackCount(DragOperation->SourceSlotIndex));

	return true;
}