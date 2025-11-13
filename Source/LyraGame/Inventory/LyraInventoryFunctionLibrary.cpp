// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraInventoryFunctionLibrary.h"

#include "InventoryFragment_Rarity.h"
#include "LyraInventoryItemDefinition.h"
#include "LyraInventoryItemInstance.h"
#include "LyraInventoryManagerComponent.h"
#include "Weapons/InventoryFragment_Ammo.h"

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
	switch (Rarity)
	{
	case EInventoryItemRarity::EIR_Uncommon:
		return FLinearColor(0.0f, 0.66f, 0.0f); // 绿色
	case EInventoryItemRarity::EIR_Rare:
		return FLinearColor(0.0f, 0.33f, 0.66f); // 蓝色
	case EInventoryItemRarity::EIR_Epic:
		return FLinearColor(0.66f, 0.0f, 0.66f); // 紫色
	case EInventoryItemRarity::EIR_Legendary:
		return FLinearColor(1.0f, 0.66f, 0.0f); // 橙色
	case EInventoryItemRarity::EIR_Mythic:
		return FLinearColor(1.0f, 0.22f, 0.22f); // 红色
	case EInventoryItemRarity::EIR_Common:
	case EInventoryItemRarity::EIR_Unknown:
	default:
		return FLinearColor(0.66f, 0.66f, 0.66f); // 灰色
	}
}

FText ULyraInventoryFunctionLibrary::GetRarityName(EInventoryItemRarity Rarity)
{
	switch (Rarity)
	{
	case EInventoryItemRarity::EIR_Common:
		return FText::FromString(TEXT("Common"));
	case EInventoryItemRarity::EIR_Uncommon:
		return FText::FromString(TEXT("Uncommon"));
	case EInventoryItemRarity::EIR_Rare:
		return FText::FromString(TEXT("Rare"));
	case EInventoryItemRarity::EIR_Epic:
		return FText::FromString(TEXT("Epic"));
	case EInventoryItemRarity::EIR_Legendary:
		return FText::FromString(TEXT("Legendary"));
	case EInventoryItemRarity::EIR_Mythic:
		return FText::FromString(TEXT("Mythic"));
	case EInventoryItemRarity::EIR_Unknown:
	default:
		return FText::FromString(TEXT("Unknown"));
	}
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
