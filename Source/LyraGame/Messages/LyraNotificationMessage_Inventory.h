#pragma once

#include "LyraNotificationMessage_Inventory.generated.h"

class UActorComponent;
class ULyraInventoryItemInstance;

// 定义库存消息类型
UENUM(BlueprintType)
enum class EInventoryStackChangeMessageType : uint8
{
	// 添加新的库存堆栈
	EISCM_Add UMETA(DisplayName = "Add Inventory Stack"),
	// 移除库存堆栈
	EISCM_Remove UMETA(DisplayName = "Remove Inventory Stack"),
	// 更新库存堆栈数量
	EISCM_Update UMETA(DisplayName = "Update Inventory Stack"),
	// 交换库存堆栈位置
	EISCM_Swap UMETA(DisplayName = "Swap Inventory Stack"),
};


// 背包数据变更消息通知
USTRUCT(BlueprintType)
struct FOnInventoryStackChangeParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	EInventoryStackChangeMessageType MessageType = EInventoryStackChangeMessageType::EISCM_Update;

	//@TODO: Tag based names+owning actors for inventories instead of directly
	// exposing the component?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	int32 NewCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory)
	int32 Delta = 0;
};

