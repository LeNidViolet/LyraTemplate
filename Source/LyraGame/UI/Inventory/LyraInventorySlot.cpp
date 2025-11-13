// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventorySlot.h"

#include "LyraGameplayTags.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "Messages/LyranotificationMessage_Inventory.h"


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
					LyraGameplayTags::Inventory_Stack_Changed,
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
	return InventoryManager ? InventoryManager->GetItemInstance(SlotIndex) : nullptr;
}

int32 ULyraInventorySlot::GetInventoryItemStackCount() const
{
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
