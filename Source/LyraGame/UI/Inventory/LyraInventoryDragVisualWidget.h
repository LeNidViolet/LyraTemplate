// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Inventory/LyraInventoryFunctionLibrary.h"
#include "LyraInventoryDragVisualWidget.generated.h"

#define UE_API LYRAGAME_API


UCLASS(MinimalAPI)
class ULyraInventoryDragVisualWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UE_API ULyraInventoryDragVisualWidget(const FObjectInitializer& ObjectInitializer);

	// 设置正在拖拽的 ItemInstance
	UFUNCTION(BlueprintCallable, Category="Inventory")
	UE_API void SetDraggedItemInstance(class ULyraInventoryItemInstance* InItemInstance);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void K2_UpdateDragVisual();

	// 设置参考槽位大小
	UFUNCTION(BlueprintCallable, Category="Inventory")
	UE_API void SetSlotSize(const FVector2D& InSlotSize);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void K2_UpdateSlotSize();

	// 设置拖拽操作状态
	UFUNCTION(BlueprintCallable, Category="Inventory")
	UE_API void SetDragOperationType(UUserWidget* TriggerWidget, EInventorySlotOperationType OperationType);
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void K2_UpdateActionState();


protected:
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	TObjectPtr<ULyraInventoryItemInstance> DraggedItemInstance;

	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	FVector2D SlotSize = FVector2D(64.f, 64.f);

	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	EInventorySlotOperationType DragOperationType = EInventorySlotOperationType::EISO_None;

	UPROPERTY()
	TObjectPtr<UUserWidget> LastWidget;
};

#undef UE_API
