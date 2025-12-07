// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventoryFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "InventoryFragment_Rarity.h"
#include "LyraGameplayTags.h"
#include "LyraInventoryItemDefinition.h"
#include "LyraInventoryItemInstance.h"
#include "LyraInventoryManagerComponent.h"
#include "System/LyraGameData.h"
#include "System/LyraSystemStatics.h"
#include "Weapons/InventoryFragment_Ammo.h"
#include "TargetData/LyraGameplayAbilityTargetData_Inventory.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryFunctionLibrary)

const ULyraInventoryItemFragment* ULyraInventoryFunctionLibrary::FindItemDefinitionFragment(
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef, TSubclassOf<ULyraInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<ULyraInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}

FLinearColor ULyraInventoryFunctionLibrary::GetRarityColor(EInventoryItemRarity Rarity)
{
	return ULyraGameData::Get().GetRarityInfo(Rarity).Color;
}

FText ULyraInventoryFunctionLibrary::GetRarityName(EInventoryItemRarity Rarity)
{
	return ULyraGameData::Get().GetRarityInfo(Rarity).DisplayName;
}

int32 ULyraInventoryFunctionLibrary::GetAmmoCount(ULyraInventoryManagerComponent* InventoryComponent, EWeaponAmmoType AmmoType)
{
	if (!InventoryComponent)
	{
		return 0;
	}

	// TArray<ULyraInventoryItemInstance*> Instances = InventoryComponent->GetAllItems();
	// for (ULyraInventoryItemInstance* Instance : Instances)
	// {
	// 	if (IsValid(Instance))
	// 	{
	// 		TSubclassOf<ULyraInventoryItemDefinition> ItemDef = Instance->GetItemDef();
	// 		const ULyraInventoryItemDefinition* ItemCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDef);
	// 		if (ItemCDO->ItemCategory == EInventoryItemCategory::EIC_Ammo)
	// 		{
	// 			const UInventoryFragment_Ammo* AmmoFragment = Cast<UInventoryFragment_Ammo>(FindItemDefinitionFragment(ItemDef, UInventoryFragment_Ammo::StaticClass()));
	// 			if (AmmoFragment)
	// 			{
	// 				if (AmmoFragment->AmmoType == AmmoType)
	// 				{
	// 					return Instance->GetStackCount();
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	return 0;
}

FString ULyraInventoryFunctionLibrary::GetInventoryAddItemResultString(EInventoryCanAddItemResult Result)
{
	// const UEnum* EnumPtr = StaticEnum<EInventoryCanAddItemResult>();
	// if (!EnumPtr) return TEXT("Invalid");
	// // 获取 DisplayName
	// return EnumPtr->GetDisplayNameTextByValue(static_cast<int64>(Result)).ToString();

	switch (Result)
	{
	case EInventoryCanAddItemResult::EICAR_Success:
		return TEXT("Success");

	case EInventoryCanAddItemResult::EICAR_InventoryFull:
		return TEXT("Inventory Full");
	case EInventoryCanAddItemResult::EICAR_ExceedsMaxItemCount:
	case EInventoryCanAddItemResult::EICAR_ExceedsMaxStackCount:
		return TEXT("Carry Limit");

	case EInventoryCanAddItemResult::EICAR_InvalidRequest:
	case EInventoryCanAddItemResult::EICAR_InvalidItemDefinition:
	case EInventoryCanAddItemResult::EICAR_InvalidItemInstance:
	case EInventoryCanAddItemResult::EICAR_ItemInstanceAlreadyInInventory:
	default:
		return TEXT("Invalid Data");
	}
}

EInventorySlotOperationType ULyraInventoryFunctionLibrary::GetInventorySlotOperationType(ULyraInventoryManagerComponent* InventoryComponent, int32 SourceSlotIndex, int32 TargetSlotIndex)
{
	EInventorySlotOperationType OperationType = EInventorySlotOperationType::EISO_None;
	if (SourceSlotIndex == TargetSlotIndex) return OperationType;
	if (!InventoryComponent) return OperationType;

	ULyraInventoryItemInstance* ItemInstance = InventoryComponent->GetItemInstance(SourceSlotIndex);
	if (!ItemInstance) return OperationType;
	ULyraInventoryItemInstance* TargetSlotItemInstance = InventoryComponent->GetItemInstance(TargetSlotIndex);

	OperationType = EInventorySlotOperationType::EISO_Swap;

	// 目标槽位没有物品, 只能交换
	if (!TargetSlotItemInstance) return OperationType;
	// 物品类型不同, 只能交换
	if (TargetSlotItemInstance->GetItemDef() != ItemInstance->GetItemDef()) return OperationType;

	// 物品类型相同, 检测是否可以堆叠
	const ULyraInventoryItemDefinition* ItemDef = GetDefault<ULyraInventoryItemDefinition>(ItemInstance->GetItemDef());
	// 不允许堆叠, 只能交换
	if (!ItemDef->bAllowStacking) return OperationType;

	// 允许堆叠, 检测当前slot的堆叠数量是否已满
	int32 CurrentStackCount = InventoryComponent->GetItemStackCount(TargetSlotIndex);
	if (CurrentStackCount >= ItemDef->MaxStackCount) return OperationType;

	OperationType = EInventorySlotOperationType::EISO_Stack;
	return OperationType;
}

bool ULyraInventoryFunctionLibrary::DropInventoryItem(
	APawn* Pawn,
	ULyraInventoryItemInstance* ItemInstance,
	int32 DropCount)
{
	if (!Pawn || !ItemInstance || DropCount <= 0)
	{
		return false;
	}

	FVector DropLocation;
	bool bOk = ULyraSystemStatics::FindValidSpawnLocationInCone(
		DropLocation,
		Pawn);
	if (!bOk)
	{
		DropLocation = Pawn->GetActorLocation();
	}


	// 这里使用TargetData传递消息给技能系统, 这个结构同时也是客户端技能与服务器通讯时使用的数据结构
	// 这个数据结构由TargetDataHandle接管负责释放
	FLyraGameplayAbilityTargetData_Inventory_Drop* DropTargetData = new FLyraGameplayAbilityTargetData_Inventory_Drop();
	DropTargetData->ItemInstance = ItemInstance;
	DropTargetData->DropCount = DropCount;
	DropTargetData->DropLocation = DropLocation;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(DropTargetData);

	FGameplayEventData EventData;
	EventData.EventTag = LyraGameplayTags::GameplayEvent_Inventory_DropItem;
	EventData.Instigator = Pawn;
	EventData.Target = Pawn;
	EventData.TargetData = TargetDataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Pawn,
		LyraGameplayTags::GameplayEvent_Inventory_DropItem,
		EventData
	);

	return true;
}

bool ULyraInventoryFunctionLibrary::SwapInventoryItem(
	APawn* Pawn,
	ULyraInventoryItemInstance* SourceSlotItemInstance,
	int32 SourceSlotIndex,
	int32 TargetSlotIndex)
{
	if (!Pawn || !SourceSlotItemInstance || SourceSlotIndex == TargetSlotIndex)
	{
		return false;
	}

	// 这里使用TargetData传递消息给技能系统, 这个结构同时也是客户端技能与服务器通讯时使用的数据结构
	// 这个数据结构由TargetDataHandle接管负责释放
	FLyraGameplayAbilityTargetData_Inventory_Swap* SwapTargetData = new FLyraGameplayAbilityTargetData_Inventory_Swap();
	SwapTargetData->ItemInstance = SourceSlotItemInstance;
	SwapTargetData->SourceSlotIndex = SourceSlotIndex;
	SwapTargetData->TargetSlotIndex = TargetSlotIndex;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(SwapTargetData);

	FGameplayEventData EventData;
	EventData.EventTag = LyraGameplayTags::GameplayEvent_Inventory_SwapItem;
	EventData.Instigator = Pawn;
	EventData.Target = Pawn;
	EventData.TargetData = TargetDataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Pawn,
		LyraGameplayTags::GameplayEvent_Inventory_SwapItem,
		EventData
	);

	return true;
}

bool ULyraInventoryFunctionLibrary::StackInventoryItem(
	APawn* Pawn,
	ULyraInventoryItemInstance* SourceSlotItemInstance,
	int32 SourceSlotIndex,
	int32 TargetSlotIndex)
{
	if (!Pawn || !SourceSlotItemInstance || SourceSlotIndex == TargetSlotIndex)
	{
		return false;
	}

	// 这里使用TargetData传递消息给技能系统, 这个结构同时也是客户端技能与服务器通讯时使用的数据结构
	// 这个数据结构由TargetDataHandle接管负责释放
	FLyraGameplayAbilityTargetData_Inventory_Stack* StackTargetData = new FLyraGameplayAbilityTargetData_Inventory_Stack();
	StackTargetData->ItemInstance = SourceSlotItemInstance;
	StackTargetData->SourceSlotIndex = SourceSlotIndex;
	StackTargetData->TargetSlotIndex = TargetSlotIndex;

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	TargetDataHandle.Add(StackTargetData);

	FGameplayEventData EventData;
	EventData.EventTag = LyraGameplayTags::GameplayEvent_Inventory_StackItem;
	EventData.Instigator = Pawn;
	EventData.Target = Pawn;
	EventData.TargetData = TargetDataHandle;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Pawn,
		LyraGameplayTags::GameplayEvent_Inventory_StackItem,
		EventData
	);

	return true;
}
