// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "InventoryFragment_Ammo.generated.h"

#define UE_API LYRAGAME_API

/**
 * @brief 常见弹药类型枚举 用于管理武器和玩家库存
 * Common ammunition types enumeration for managing weapons and player inventory.
 */
UENUM(BlueprintType)
enum class EWeaponAmmoType : uint8
{
	// It's always best practice to include a 'None' or 'Undefined' as the first value (0) for initialization.
	EAT_None UMETA(DisplayName = "None"),

	// 步枪/突击步枪弹药 (Rifle/Assault Rifle Ammo, e.g., 5.56mm)
	EAT_Rifle UMETA(DisplayName = "Rifle Ammo"),

	// 手枪/冲锋枪弹药 (Pistol/SMG Ammo, e.g., 9mm)
	EAT_Pistol UMETA(DisplayName = "Pistol Ammo"),

	// 霰弹枪弹药 (Shotgun Shells/Buckshot)
	EAT_Shotgun UMETA(DisplayName = "Shotgun Shells"),

	// 狙击步枪弹药 (Sniper Rifle Ammo, e.g., .308)
	EAT_Sniper UMETA(DisplayName = "Sniper Ammo"),

	// 重型机枪/特殊武器弹药 (Heavy Machine Gun/Special Weapon Ammo)
	EAT_Heavy UMETA(DisplayName = "Heavy Ammo"),

	// 能量/激光武器弹药 (Energy/Laser Weapon Ammunition)
	EAT_Energy UMETA(DisplayName = "Energy Cell"),

	// 爆炸物 (Explosives, e.g., Grenade Launcher rounds)
	EAT_Explosive UMETA(DisplayName = "Explosive Round"),

	// 特殊/稀有弹药 (Special/Rare Ammo)
	EAT_Special UMETA(DisplayName = "Special Ammo")
};


/**
 * @brief 弹药库存碎片类 用于表示库存物品的弹药属性
 * Inventory fragment class for representing ammunition properties of inventory items.
 */
UCLASS()
class UInventoryFragment_Ammo : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:

	UInventoryFragment_Ammo();

	// 弹药类型 (Type of ammunition)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ammo)
	EWeaponAmmoType AmmoType = EWeaponAmmoType::EAT_None;

	// 弹药图标 (Icon representing the ammunition)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ammo)
	FSlateBrush AmmoBrush;
};

#undef UE_API