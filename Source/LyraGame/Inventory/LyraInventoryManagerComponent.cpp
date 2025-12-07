// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraInventoryManagerComponent.h"

#include "IPickupable.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraInventoryItemDefinition.h"
#include "LyraInventoryItemInstance.h"
#include "LyraLogChannels.h"
#include "LyraGameplayTags.h"
#include "Interaction/LyraWorldCollectable.h"
#include "Net/UnrealNetwork.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;


//////////////////////////////////////////////////////////////////////
// FLyraInventoryItem

FString FLyraInventoryItem::GetDebugString() const
{
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef;
	if (ItemInstance != nullptr)
	{
		ItemDef = ItemInstance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(ItemInstance), StackCount, *GetNameSafe(ItemDef));
}

bool FLyraInventoryItem::UpdateItem(ULyraInventoryItemInstance* InInstance, int32 InStackCount)
{
	// 更新库存条目(服务端)
	ItemInstance = InInstance;
	StackCount = InStackCount;

	// 这里的 LastObservedInstance LastObservedStackCount 属于服务端
	// 它们与 Replicated 回调中的不同, 那些是客户端的版本
	// 在这里修改了 Observed 数据再直接调用 Replicated 回调的话会检测不到变化

	bool Result = false;
	if (ItemInstance != LastObservedInstance || StackCount != LastObservedStackCount)
	{
		// 是否有变动
		Result = true;

		LastObservedStackCount = StackCount;
		LastObservedInstance = ItemInstance;
	}
	return Result;
}

//////////////////////////////////////////////////////////////////////
// FLyraInventoryList

void FLyraInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// 这个方法只会在 client 端触发
	// 目前来说不会从数组中删除某个节点, 只是清空节点数据

	for (int32 Index : RemovedIndices)
	{
		FLyraInventoryItem& Item = Items[Index];
		BroadcastChangeMessage(
			Item,
			/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Remove,
			/*OldCount=*/Item.LastObservedStackCount,
			/*NewCount=*/0);
		Item.LastObservedInstance = nullptr;
		Item.LastObservedStackCount = 0;
	}
}

void FLyraInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	// 这个方法只会在 client 端触发
	// 在初始化的时候会调用到添加接口, 之后不会再添加新的节点, 只是更新已有节点的数据

	for (int32 Index : AddedIndices)
	{
		FLyraInventoryItem& Item = Items[Index];
		BroadcastChangeMessage(
			Item,
			/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Add,
			/*OldCount=*/0,
			/*NewCount=*/Item.StackCount);

		// 开始添加时, StackCount 会指定为 0
		Item.LastObservedInstance = Item.ItemInstance;
		Item.LastObservedStackCount = Item.StackCount;
	}
}

void FLyraInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	// 这个方法只会在 client 端触发
	for (int32 Index : ChangedIndices)
	{
		FLyraInventoryItem& Item = Items[Index];

		if (Item.LastObservedInstance == nullptr && Item.ItemInstance == nullptr)
		{
			continue;
		}

		// 这里可能是 覆盖新的 清空旧的 交换位置 变更数量

		if (Item.LastObservedInstance == nullptr && Item.ItemInstance != nullptr)
		{
			// 旧实例为空, 新实例不为空, 说明是新增
			BroadcastChangeMessage(
				Item,
				/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Add,
				/*OldCount=*/0,
				/*NewCount=*/Item.StackCount);

			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> PostReplicatedChange: Added %s"), *Item.GetDebugString());

			Item.LastObservedInstance = Item.ItemInstance;
			Item.LastObservedStackCount = Item.StackCount;
			continue;
		}

		if (Item.LastObservedInstance != nullptr && Item.ItemInstance == nullptr)
		{
			// 旧实例不为空, 新实例为空, 说明是移除
			BroadcastChangeMessage(
				Item,
				/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Remove,
				/*OldCount=*/Item.LastObservedStackCount,
				/*NewCount=*/0);

			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> PostReplicatedChange: Removed %s"), *Item.GetDebugString());

			Item.LastObservedInstance = nullptr;
			Item.LastObservedStackCount = 0;
			continue;
		}

		if (Item.LastObservedInstance == Item.ItemInstance && Item.LastObservedStackCount != Item.StackCount)
		{
			// 实例相同, 但是数量变更
			BroadcastChangeMessage(
				Item,
				/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Update,
				/*OldCount=*/Item.LastObservedStackCount,
				/*NewCount=*/Item.StackCount);

			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> PostReplicatedChange: Updated %s"), *Item.GetDebugString());

			Item.LastObservedInstance = Item.ItemInstance;
			Item.LastObservedStackCount = Item.StackCount;
			continue;
		}

		if (Item.LastObservedInstance != Item.ItemInstance)
		{
			// 实例变更, 说明是交换位置
			BroadcastChangeMessage(
				Item,
				/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Swap,
				/*OldCount=*/0,
				/*NewCount=*/Item.StackCount);

			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> PostReplicatedChange: Swapped %s"), *Item.GetDebugString());

			Item.LastObservedInstance = Item.ItemInstance;
			Item.LastObservedStackCount = Item.StackCount;
			continue;
		}

		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> PostReplicatedChange: Unhandled change for %s"), *Item.GetDebugString());
	}
}

void FLyraInventoryList::BroadcastChangeMessage(
	FLyraInventoryItem& Item,
	EInventoryStackChangeMessageType MessageType,
	int32 OldCount,
	int32 NewCount)
{
	FOnInventoryStackChangeParameters Message;
	Message.SlotIndex = Item.SlotIndex;
	Message.MessageType = MessageType;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Item.ItemInstance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(LyraGameplayTags::Gameplay_Message_Inventory_StackChanged, Message);
}

bool FLyraInventoryList::IsItemInstanceInInventory(ULyraInventoryItemInstance* ItemInstance) const
{
	for (const FLyraInventoryItem& Item : Items)
	{
		if (Item.ItemInstance == ItemInstance)
		{
			return true;
		}
	}
	return false;
}

bool FLyraInventoryList::AllocItem(ULyraInventoryItemInstance* ItemInstance, int32 StackCount)
{
	if (IsItemInstanceInInventory(ItemInstance))
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> FLyraInventoryList::AllocItem failed: Instance %s is already in inventory"), *GetNameSafe(ItemInstance));
		return false;
	}

	// 查找第一个空闲位置
	int32 FreeIndex = FindFreeSlotIndex();
	if (!IsValidSlotIndex(FreeIndex))
	{
		return false;
	}

	// 使用空闲位置, 注意必要的初始化
	FLyraInventoryItem &Item = Items[FreeIndex];
	bool bOk = Item.UpdateItem(ItemInstance, StackCount);
	check(bOk);

	MarkItemDirty(Item);
	// Broadcast on Server/Standalone
	BroadcastChangeMessage(
	Item,
	/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Add,
	/*OldCount=*/0,
	/*NewCount=*/Item.StackCount);

	return true;
}


void FLyraInventoryList::EraseItem(ULyraInventoryItemInstance* ItemInstance)
{
	for (FLyraInventoryItem& Item : Items)
	{
		if (Item.ItemInstance == ItemInstance)
		{
			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> FLyraInventoryList::EraseItem: %s"), *Item.GetDebugString());

			int32 OldStackCount = Item.StackCount;

			bool bOk = Item.UpdateItem(nullptr, 0);
			check(bOk);
			MarkItemDirty(Item);

			// Broadcast on Server/Standalone
			BroadcastChangeMessage(
				Item,
				/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Remove,
				/*OldCount=*/OldStackCount,
				/*NewCount=*/0);
			break;
		}
	}
}

bool FLyraInventoryList::StackItem(ULyraInventoryItemInstance* ItemInstance, int32 Delta)
{
	bool bResult = false;

	for (FLyraInventoryItem& Item : Items)
	{
		if (Item.ItemInstance == ItemInstance)
		{
			int32 OldStackCount = Item.StackCount;
			int32 NewStackCount = OldStackCount + Delta;

			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> FLyraInventoryList::StackItem: Changed %s from %d to %d"), *Item.GetDebugString(), OldStackCount, NewStackCount);

			if (NewStackCount <= 0)
			{
				ensure(NewStackCount == 0);

				// 移除该物品实例 注意调用方应当负责将Instance移除复制队列
				bool bOk = Item.UpdateItem(nullptr, 0);
				check(bOk);
				MarkItemDirty(Item);

				// Broadcast on Server/Standalone
				BroadcastChangeMessage(
					Item,
					/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Remove,
					/*OldCount=*/OldStackCount,
					/*NewCount=*/0);
			}
			else
			{
				// 更新堆叠数量 我们这里不检查是否超过最大堆叠数量, 由调用方负责
				bool bOk = Item.UpdateItem(ItemInstance, NewStackCount);
				check(bOk);
				MarkItemDirty(Item);

				// 实例相同, 但是数量变更
				BroadcastChangeMessage(
					Item,
					/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Update,
					/*OldCount=*/OldStackCount,
					/*NewCount=*/NewStackCount);
			}

			bResult = true;
			break;
		}
	}

	return bResult;
}

bool FLyraInventoryList::SwapItem(int32 SourceSlotIndex, int32 TargetSlotIndex)
{
	if (!IsValidSlotIndex(SourceSlotIndex) || !IsValidSlotIndex(TargetSlotIndex))
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> FLyraInventoryList::SwapItem failed: Invalid slot indices %d, %d"), SourceSlotIndex, TargetSlotIndex);
		return false;
	}
	if (SourceSlotIndex == TargetSlotIndex) return false;

	FLyraInventoryItem SourceItem = Items[SourceSlotIndex];
	FLyraInventoryItem TargetItem = Items[TargetSlotIndex];

	FLyraInventoryItem& SourceItemRef = Items[SourceSlotIndex];
	FLyraInventoryItem& TargetItemRef = Items[TargetSlotIndex];

	// 交换数据
	SourceItemRef.UpdateItem(TargetItem.ItemInstance.Get(), TargetItem.StackCount);
	TargetItemRef.UpdateItem(SourceItem.ItemInstance.Get(), SourceItem.StackCount);

	MarkItemDirty(SourceItemRef);
	MarkItemDirty(TargetItemRef);

	BroadcastChangeMessage(
		SourceItemRef,
		/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Swap,
		/*OldCount=*/0,
		/*NewCount=*/SourceItemRef.StackCount);
	BroadcastChangeMessage(
		TargetItemRef,
		/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Swap,
		/*OldCount=*/0,
		/*NewCount=*/TargetItemRef.StackCount);

	return true;
}


void FLyraInventoryList::Initialize(int32 Capacity)
{
	if (Capacity <= 0)
	{
		return;
	}
	if (Items.Num() > 0)
	{
		return;
	}

	Items.SetNum(Capacity);
	for (int32 i = 0; i < Capacity; ++i)
	{
		Items[i].SlotIndex = i;
		Items[i].ItemInstance = nullptr;
		Items[i].StackCount = 0;
		Items[i].LastObservedStackCount = 0;
		MarkItemDirty(Items[i]); // 确保同步

		// Broadcast on Server/Standalone
		BroadcastChangeMessage(
		Items[i],
		/*MessageType=*/EInventoryStackChangeMessageType::EISCM_Add,
		/*OldCount=*/0,
		/*NewCount=*/0);
	}
}

bool FLyraInventoryList::IsValidSlotIndex(int32 Index) const
{
	return Index >= 0 && Index < Items.Num();
}

TArray<ULyraInventoryItemInstance*> FLyraInventoryList::GetAllItemInstances() const
{
	TArray<ULyraInventoryItemInstance*> Results;
	Results.Reserve(Items.Num());
	for (const FLyraInventoryItem& Item : Items)
	{
		if (IsValid(Item.ItemInstance.Get())) //@TODO: Would prefer to not deal with this
		{
			Results.Add(Item.ItemInstance.Get());
		}
	}
	return Results;
}

int32 FLyraInventoryList::FindFreeSlotIndex() const
{
	for (const FLyraInventoryItem& Item : Items)
	{
		if (!IsValid(Item.ItemInstance.Get())) //@TODO: Would prefer to not deal with this
		{
			return Item.SlotIndex;
		}
	}
	return INDEX_NONE;
}

int32 FLyraInventoryList::GetFreeSlotsCount() const
{
	int32 Count = 0;
	for (const FLyraInventoryItem& Item : Items)
	{
		if (Item.ItemInstance == nullptr)
		{
			++Count;
		}
	}
	return Count;
}

int32 FLyraInventoryList::FindSlotIndexByInstance(ULyraInventoryItemInstance* ItemInstance) const
{
	for (const FLyraInventoryItem& Item : Items)
	{
		ULyraInventoryItemInstance* OtherInstance = Item.ItemInstance.Get();
		if (IsValid(OtherInstance) && Item.ItemInstance == ItemInstance)
		{
			return Item.SlotIndex;
		}
	}
	return INDEX_NONE;
}

int32 FLyraInventoryList::FindSlotIndexByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition) const
{
	for (const FLyraInventoryItem& Item : Items)
	{
		ULyraInventoryItemInstance* ItemInstance = Item.ItemInstance.Get();
		if (IsValid(ItemInstance) && ItemInstance->GetItemDef() == ItemDefinition)
		{
			return Item.SlotIndex;
		}
	}
	return INDEX_NONE;
}

ULyraInventoryItemInstance* FLyraInventoryList::FindItemInstanceByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition) const
{
	for (const FLyraInventoryItem& Item : Items)
	{
		ULyraInventoryItemInstance* ItemInstance = Item.ItemInstance.Get();
		if (IsValid(ItemInstance) && ItemInstance->GetItemDef() == ItemDefinition)
		{
			return ItemInstance;
		}
	}
	return nullptr;
}

int32 FLyraInventoryList::GetSlotsCountByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition) const
{
	// 获取指定类型道具的slot数量
	int32 TotalCount = 0;
	for (const FLyraInventoryItem& Item : Items)
	{
		ULyraInventoryItemInstance* Instance = Item.ItemInstance.Get();

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDefinition)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}


//////////////////////////////////////////////////////////////////////
// ULyraInventoryManagerComponent

ULyraInventoryManagerComponent::ULyraInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	// 因为组件中含有需要复制的Property, 所以需要设置组件的Replicated
	SetIsReplicatedByDefault(true);
}

void ULyraInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryCapacity = ULyraGameData::Get().InventoryMaxCapacity;
	AActor* OwningActor = GetOwner();
	if (OwningActor && OwningActor->HasAuthority())
	{
		// 仅在服务器上初始化库存列表
		InventoryList.Initialize(InventoryCapacity);
	}
}

void ULyraInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

EInventoryCanAddItemResult ULyraInventoryManagerComponent::CanAddItem(ALyraWorldCollectable* Collectable, EInventoryStackBehavior StackBehavior) const
{
	// 这里不做权限判断, 客户端也可以调用这个函数来检查是否可以添加物品

	if (!Collectable)
	{
		return EInventoryCanAddItemResult::EICAR_InvalidRequest;
	}

	FInventoryPickup Pickup = Collectable->GetPickupInventory();

	// 没有物品需要添加
	if (Pickup.Instances.Num() <= 0 && Pickup.Definitions.Num() <= 0)
	{
		return EInventoryCanAddItemResult::EICAR_InvalidRequest;
	}

	// 如果有任意物品实例或者物品定义可以添加则返回 true , 不要求全部添加成功
	for (const FPickupInstance& PickupInstance : Pickup.Instances)
	{
		ULyraInventoryItemInstance* Instance = PickupInstance.Item.Get();
		if (IsValid(Instance) && PickupInstance.StackCount > 0)
		{
			EInventoryCanAddItemResult Result = CanAddItemForInstance(
				Instance,
				PickupInstance.StackCount,
				StackBehavior);

			if (Result == EInventoryCanAddItemResult::EICAR_Success)
			{
				return Result;
			}
		}
	}

	for (const FPickupDefinition& PickupDefinition : Pickup.Definitions)
	{
		if (PickupDefinition.ItemDef && PickupDefinition.StackCount > 0)
		{
			EInventoryCanAddItemResult Result = CanAddItemForDefinition(
				PickupDefinition.ItemDef,
				PickupDefinition.StackCount,
				StackBehavior);

			if (Result == EInventoryCanAddItemResult::EICAR_Success)
			{
				return Result;
			}
		}
	}

	return EInventoryCanAddItemResult::EICAR_InvalidRequest;
}

EInventoryCanAddItemResult ULyraInventoryManagerComponent::CanAddItemForDefinition(
	TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
	int32 StackCount,
	EInventoryStackBehavior StackBehavior) const
{
	// 这里不做权限判断, 客户端也可以调用这个函数来检查是否可以添加物品

	if (!ItemDefinition || StackCount <= 0)
	{
		return EInventoryCanAddItemResult::EICAR_InvalidRequest;
	}

	switch (StackBehavior)
	{
	case EInventoryStackBehavior::ESB_StackIntoExisting:
		{
			return CanAddItemToExisting(ItemDefinition, StackCount);
		}
	case EInventoryStackBehavior::ESB_CreateNewStack:
		{
			return CanAddItemToNew(ItemDefinition, StackCount);
		}
	case EInventoryStackBehavior::ESB_StackOrCreateNew:
		{
			EInventoryCanAddItemResult Result1 = CanAddItemToExisting(ItemDefinition, StackCount);
			if (Result1 == EInventoryCanAddItemResult::EICAR_Success)
			{
				return Result1;
			}
			EInventoryCanAddItemResult Result2 = CanAddItemToNew(ItemDefinition, StackCount);
			if (Result2 == EInventoryCanAddItemResult::EICAR_Success)
			{
				return Result2;
			}

			// 随意返回一个
			return Result2;
		}
	default:
		{
			return EInventoryCanAddItemResult::EICAR_InvalidRequest;
		}
	}
}

EInventoryCanAddItemResult ULyraInventoryManagerComponent::CanAddItemForInstance(
	ULyraInventoryItemInstance* ItemInstance,
	int32 StackCount,
	EInventoryStackBehavior StackBehavior) const
{
	// 这里不做权限判断, 客户端也可以调用这个函数来检查是否可以添加物品

	if (!ItemInstance || !ItemInstance->GetItemDef() || StackCount <= 0)
	{
		return EInventoryCanAddItemResult::EICAR_InvalidRequest;
	}

	// 物品实例已存在?
	if (IsItemInstanceInInventory(ItemInstance))
	{
		return EInventoryCanAddItemResult::EICAR_ItemInstanceAlreadyInInventory;
	}


	TSubclassOf<ULyraInventoryItemDefinition> ItemDef =	ItemInstance->GetItemDef();
	return CanAddItemForDefinition(ItemDef, StackCount, StackBehavior);
}


EInventoryCanAddItemResult ULyraInventoryManagerComponent::CanAddItemToExisting(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition, int32 StackCount) const
{
	// 检查是否有可以堆叠的 Slot

	const ULyraInventoryItemDefinition* ItemCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefinition);

	// 如果不允许堆叠则直接返回 false
	if (!ItemCDO->bAllowStacking)
		return EInventoryCanAddItemResult::EICAR_ExceedsMaxStackCount;

	// 检查每个Slot 的 MaxStackSize 限制
	// 如果有一个 Slot 没有达到限制, 就算不能容纳所有的 StackCount 也允许添加
	for (const FLyraInventoryItem& Item : InventoryList.Items)
	{
		ULyraInventoryItemInstance* Instance = Item.ItemInstance.Get();
		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDefinition)
			{
				if (Item.StackCount < ItemCDO->MaxStackCount)
				{
					return EInventoryCanAddItemResult::EICAR_Success;
				}
			}
		}
	}

	// 也有可能没找到目标Definition
	return EInventoryCanAddItemResult::EICAR_ExceedsMaxStackCount;
}

EInventoryCanAddItemResult ULyraInventoryManagerComponent::CanAddItemToNew(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition, int32 StackCount) const
{
	// 检查是否有空闲的 slot, 并且符合 MaxInstanceSlotCount 限制

	const ULyraInventoryItemDefinition* ItemCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefinition);

	// 此类型道具已经占据了几个 slot
	int32 InstanceSlotCount = InventoryList.GetSlotsCountByDefinition(ItemDefinition);

	// 剩余的背包空闲 slot 数量
	int32 FreeSlotCount = InventoryList.GetFreeSlotsCount();

	// 背包已满
	if (FreeSlotCount <= 0)
	{
		return EInventoryCanAddItemResult::EICAR_InventoryFull;
	}

	// 如果不限制 MaxInstanceSlots(也就是该道具背包里可以放几个) 则只要有空间就允许添加
	if (ItemCDO->MaxInstanceSlotCount == 0)
	{
		return EInventoryCanAddItemResult::EICAR_Success;
	}

	// 如果限制 MaxInstanceSlots 则检查是否到达了限制, 到达的话不添加
	if (InstanceSlotCount >= ItemCDO->MaxInstanceSlotCount)
	{
		return EInventoryCanAddItemResult::EICAR_ExceedsMaxItemCount;
	}

	return EInventoryCanAddItemResult::EICAR_Success;
}

bool ULyraInventoryManagerComponent::IsItemInstanceInInventory(ULyraInventoryItemInstance* ItemInstance) const
{
	if (!ItemInstance)
	{
		return false;
	}

	for (const FLyraInventoryItem& Item : InventoryList.Items)
	{
		ULyraInventoryItemInstance* Instance = Item.ItemInstance.Get();

		if (IsValid(Instance))
		{
			if (Instance == ItemInstance)
			{
				return true;
			}
		}
	}

	return false;
}


int32 ULyraInventoryManagerComponent::AddItem(ALyraWorldCollectable* Collectable, FInventoryPickup& RestPickup, EInventoryStackBehavior StackBehavior)
{
	int32 Result = 0;

	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::AddItem failed: OwnerComponent has no valid owning Actor or not Authority"));
		return Result;
	}

	FInventoryPickup ItemPickup = Collectable->GetPickupInventory();

	// 先处理 Definition
	for (const FPickupDefinition& PickupDefinition : ItemPickup.Definitions)
	{
		if (PickupDefinition.ItemDef && PickupDefinition.StackCount > 0)
		{
			EInventoryCanAddItemResult CanAddItemResult = CanAddItemForDefinition(
				PickupDefinition.ItemDef,
				PickupDefinition.StackCount,
				StackBehavior);
			if (CanAddItemResult == EInventoryCanAddItemResult::EICAR_Success)
			{
				int32 NumAdded = AddItemDefinition(
					PickupDefinition.ItemDef,
					PickupDefinition.StackCount,
					StackBehavior);
				Result += NumAdded;
				int32 NumRemaining = PickupDefinition.StackCount - NumAdded;
				if (NumRemaining > 0)
				{
					FPickupDefinition RemainingDef;
					RemainingDef.ItemDef = PickupDefinition.ItemDef;
					RemainingDef.StackCount = NumRemaining;
					RestPickup.Definitions.Add(RemainingDef);
				}
				else
				{
					ensure(NumRemaining == 0);
				}
			}
			else
			{
				// 无法添加, 全部放入剩余物品中
				RestPickup.Definitions.Add(PickupDefinition);
			}
		}
	}
	// 再处理 Instance
	for (const FPickupInstance& PickupInstance : ItemPickup.Instances)
	{
		if (IsValid(PickupInstance.Item.Get()) && PickupInstance.StackCount > 0)
		{
			EInventoryCanAddItemResult CanAddItemResult = CanAddItemForInstance(
				PickupInstance.Item.Get(),
				PickupInstance.StackCount,
				StackBehavior);
			if (CanAddItemResult == EInventoryCanAddItemResult::EICAR_Success)
			{
				int32 NumAdded = AddItemInstance(
					PickupInstance.Item.Get(),
					Collectable,
					PickupInstance.StackCount,
					StackBehavior);
				Result += NumAdded;
				int32 NumRemaining = PickupInstance.StackCount - NumAdded;
				if (NumRemaining > 0)
				{
					FPickupInstance RemainingInst;
					RemainingInst.Item = PickupInstance.Item;
					RemainingInst.StackCount = NumRemaining;
					RestPickup.Instances.Add(RemainingInst);
				}
				else
				{
					ensure(NumRemaining == 0);
				}
			}
			else
			{
				// 无法添加, 全部放入剩余物品中
				RestPickup.Instances.Add(PickupInstance);
			}
		}
	}

	return Result;
}



int32 ULyraInventoryManagerComponent::AddItemDefinition(
	TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
	int32 StackCount,
	EInventoryStackBehavior StackBehavior)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()){ return 0; }
	if (!ItemDefinition || StackCount <= 0){ return 0; }
	if (CanAddItemForDefinition(ItemDefinition, StackCount, StackBehavior) != EInventoryCanAddItemResult::EICAR_Success){ return 0; }

	return AddItemInstanceInternal(false, nullptr, ItemDefinition, nullptr, StackCount, StackBehavior);
}

int32 ULyraInventoryManagerComponent::AddItemInstance(
	ULyraInventoryItemInstance* ItemInstance,
	AActor* OwnerActor,
	int32 StackCount,
	EInventoryStackBehavior StackBehavior)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()){ return 0; }
	if (!ItemInstance || StackCount <= 0){ return 0; }
	if (CanAddItemForInstance(ItemInstance, StackCount, StackBehavior) != EInventoryCanAddItemResult::EICAR_Success){ return 0; }
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef = ItemInstance->GetItemDef();
	if (!ItemDef){ return 0; }

	return AddItemInstanceInternal(true, OwnerActor, ItemDef, ItemInstance, StackCount, StackBehavior);
}



int32 ULyraInventoryManagerComponent::AddItemInstanceInternal(
	bool FromInstance,
	AActor* OwnerActor,
	TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
	ULyraInventoryItemInstance* ItemInstance,
	int32 StackCount,
	EInventoryStackBehavior StackBehavior)
{
	// 向库存中添加物品实例

	int32 NumAdded = 0;
	const ULyraInventoryItemDefinition* ItemCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDefinition);

	// 首先遍历可以进行堆叠的 slot
	if (StackBehavior == EInventoryStackBehavior::ESB_StackIntoExisting ||
		StackBehavior == EInventoryStackBehavior::ESB_StackOrCreateNew)
	{
		// 如果不允许堆叠则跳过
		if (ItemCDO->bAllowStacking)
		{
			for (FLyraInventoryItem& Item : InventoryList.Items)
			{
				ULyraInventoryItemInstance* Instance = Item.ItemInstance.Get();
				if (IsValid(Instance))
				{
					if (Instance->GetItemDef() == ItemDefinition)
					{
						int32 AvailableSpace = ItemCDO->MaxStackCount - Item.StackCount;
						if (AvailableSpace > 0)
						{
							int32 ToAdd = FMath::Min(AvailableSpace, StackCount);

							InventoryList.StackItem(Instance, ToAdd);

							StackCount -= ToAdd;

							NumAdded += ToAdd;

							if (StackCount <= 0)
							{
								// 全部堆叠完成
								ensure(StackCount == 0);
								return NumAdded;
							}
						}
					}
				}
			}
		}
	}

	// 流程到这里要么是 StackBehavior 要求创建新堆叠,
	// 要么是已有slot没有足够空间堆叠

	// 如果只要求堆叠则直接返回
	if (StackBehavior == EInventoryStackBehavior::ESB_StackIntoExisting)
	{
		return NumAdded;
	}


	// 这里可能涉及到多次创建新堆叠
	while (StackCount > 0)
	{
		int32 ToAdd = FMath::Min(ItemCDO->MaxStackCount, StackCount);
		// 有可能没有足够的 slot 空间
		if (CanAddItemToNew(ItemDefinition, ToAdd) != EInventoryCanAddItemResult::EICAR_Success)
		{
			break;
		}

		// 一个 slot 就可以放下所有的堆叠数量
		if (FromInstance && ToAdd == StackCount)
		{
			// 哪些内容会被复制?
			// UPROPERTY 标注的所有属性
			// UOBject* 执行浅拷贝, 与原始对象指向同一个地址
			// Instanced 属性 (递归复制)
			// 普通的 C++ 成员变量不会被复制 它们会保持默认构造的值
			// 容器 TArray, TMap 容器内的元素也会按照上述规则进行复制

			// TODO FGameplayTagStackContainer 中有TMap, 可能需要手动重建它
			ULyraInventoryItemInstance* NewInstance = DuplicateObject<ULyraInventoryItemInstance>(ItemInstance, this);

			// 注册复制, 否则客户端可能看不到这个物品的更新
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(NewInstance);
			}

			bool bOk = InventoryList.AllocItem(NewInstance, ToAdd);
			check(bOk);

			NumAdded += ToAdd;
			break;
		}

		if (FromInstance)
		{
			// 一般不会走到这个分支, 除非Instance内道具数量超过了单个堆叠上限
			ensure(false);
			break;
		}

		// Owner == Controller
		ULyraInventoryItemInstance* NewInstance = NewObject<ULyraInventoryItemInstance>(this);
		NewInstance->SetItemDef(ItemDefinition);

		// 初始化 Fragment 使用 ItemDef 创建新实例时需要调用
		for (ULyraInventoryItemFragment* Fragment : GetDefault<ULyraInventoryItemDefinition>(ItemDefinition)->Fragments)
		{
			if (Fragment != nullptr)
			{
				Fragment->OnInstanceCreated(NewInstance);
			}
		}

		bool bOk = InventoryList.AllocItem(NewInstance, ToAdd);
		check(bOk);

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::AddItem: Adding replicated subobject %s"), *GetNameSafe(NewInstance));
			AddReplicatedSubObject(NewInstance);
		}

		StackCount -= ToAdd;
		NumAdded += ToAdd;
		check(StackCount >= 0);
	}

	return NumAdded;
}


bool ULyraInventoryManagerComponent::DropItem(
	ULyraInventoryItemInstance* ItemInstance,
	int32 StackCount,
	ULyraInventoryItemInstance*& OutDroppedInstance,
	int32& OutDroppedStackCount)
{
	if (!ItemInstance || StackCount <= 0) return false;
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()) {return false;}

	int32 SlotIndex = InventoryList.FindSlotIndexByInstance(ItemInstance);
	if (!InventoryList.IsValidSlotIndex(SlotIndex))
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::DropItem failed: ItemInstance %s is not in inventory"), *GetNameSafe(ItemInstance));
		return false;
	}

	FLyraInventoryItem &Item = InventoryList.Items[SlotIndex];
	if (StackCount > Item.StackCount)
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::DropItem failed: ItemInstance %s has only %d items, requested %d"), *GetNameSafe(ItemInstance), Item.StackCount, StackCount);
		return false;
	}

	// 判断一下是全部丢弃 还是部分丢弃
	if (StackCount < Item.StackCount)
	{
		// 部分丢弃, 减少库存数量, 使用 ItemDefinition 创建新的 WorldCollectable
		InventoryList.StackItem(ItemInstance, -StackCount);
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::DropItem: Dropped %d from ItemInstance %s, Remaining StackCount %d"), StackCount, *GetNameSafe(ItemInstance), Item.StackCount);

		// 创建新的 ItemInstance 作为丢弃物
		ULyraInventoryItemInstance* NewInstance = NewObject<ULyraInventoryItemInstance>(GetTransientPackage());
		NewInstance->SetItemDef(ItemInstance->GetItemDef());

		// 初始化 Fragments
		for (ULyraInventoryItemFragment* Fragment : GetDefault<ULyraInventoryItemDefinition>(ItemInstance->GetItemDef())->Fragments)
		{
			if (Fragment != nullptr)
			{
				Fragment->OnInstanceCreated(NewInstance);
			}
		}

		OutDroppedInstance = NewInstance;
		OutDroppedStackCount = StackCount;
	}
	else
	{
		// 全部丢弃, 直接从库存中移除
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::DropItem: Dropped all %d from ItemInstance %s"), Item.StackCount, *GetNameSafe(ItemInstance));

		// TODO FGameplayTagStackContainer 中有TMap, 可能需要手动重建它
		ULyraInventoryItemInstance* NewInstance = DuplicateObject<ULyraInventoryItemInstance>(ItemInstance, GetTransientPackage());

		RemoveItemInstance(ItemInstance);

		OutDroppedInstance = NewInstance;
		OutDroppedStackCount = StackCount;
	}

	return true;
}

bool ULyraInventoryManagerComponent::SwapItem(
	ULyraInventoryItemInstance* SourceSlotItemInstance,
	int32 SourceSlotIndex,
	int32 TargetSlotIndex)
{
	// 交换两个 slot 的物品实例
	// 需要验证权限和索引合法性
	if (!SourceSlotItemInstance) return false;
	if (!InventoryList.IsValidSlotIndex(SourceSlotIndex)) return false;
	if (!InventoryList.IsValidSlotIndex(TargetSlotIndex)) return false;
	if (SourceSlotIndex == TargetSlotIndex) return false;
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()) {return false;}

	if (!InventoryList.IsValidSlotIndex(SourceSlotIndex)) {return false;}
	if (!InventoryList.IsValidSlotIndex(TargetSlotIndex)) {return false;}
	int32 ExistsSlotIndex = InventoryList.FindSlotIndexByInstance(SourceSlotItemInstance);
	if (ExistsSlotIndex != SourceSlotIndex) {return false;}

	return InventoryList.SwapItem(SourceSlotIndex, TargetSlotIndex);
}

bool ULyraInventoryManagerComponent::StackItem(
	ULyraInventoryItemInstance* SourceSlotItemInstance,
	int32 SourceSlotIndex,
	int32 TargetSlotIndex)
{
	// 交换合并两个 slot 的数量
	// 需要验证权限和索引合法性
	if (!SourceSlotItemInstance) return false;
	if (!InventoryList.IsValidSlotIndex(SourceSlotIndex)) return false;
	if (!InventoryList.IsValidSlotIndex(TargetSlotIndex)) return false;
	if (SourceSlotIndex == TargetSlotIndex) return false;
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()) {return false;}

	int32 ExistsSlotIndex = InventoryList.FindSlotIndexByInstance(SourceSlotItemInstance);
	if (ExistsSlotIndex != SourceSlotIndex) {return false;}

	FLyraInventoryItem& SourceItemRef = InventoryList.Items[SourceSlotIndex];
	FLyraInventoryItem& TargetItemRef = InventoryList.Items[TargetSlotIndex];
	if (SourceItemRef.ItemInstance == nullptr || TargetItemRef.ItemInstance == nullptr) {return false;}
	if (SourceItemRef.ItemInstance->GetItemDef() != TargetItemRef.ItemInstance->GetItemDef())
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::StackItem failed: Item definitions do not match %s vs %s"),
			*GetNameSafe(SourceItemRef.ItemInstance->GetItemDef()),
			*GetNameSafe(TargetItemRef.ItemInstance->GetItemDef()));
		return false;
	}

	ULyraInventoryItemDefinition* CDO = SourceItemRef.ItemInstance->GetItemDef().GetDefaultObject();
	if (!CDO->bAllowStacking) return false;

	// 计算可以合并的数量
	int32 MaxStackCount = CDO->MaxStackCount;
	int32 AvailableSpace = MaxStackCount - TargetItemRef.StackCount;
	if (AvailableSpace <= 0)  {return false;}

	int32 StackCount = FMath::Min(AvailableSpace, SourceItemRef.StackCount);
	if (StackCount <= 0)  {return false;}

	// 执行合并
	if (SourceItemRef.StackCount != StackCount)
	{
		InventoryList.StackItem(SourceItemRef.ItemInstance.Get(), -StackCount);
	}
	else
	{
		// 源物品数量归零, 直接移除
		RemoveItemInstance(SourceItemRef.ItemInstance.Get());
	}

	InventoryList.StackItem(TargetItemRef.ItemInstance.Get(), StackCount);

	return true;
}


void ULyraInventoryManagerComponent::RemoveItemInstance(ULyraInventoryItemInstance* ItemInstance, bool MarkAsGarbage)
{
	if (!ItemInstance) {return;}
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()) {return;}

	// 从库存中删除, 客户端会通过复制自动更新, 注意客户端做好资源清理操作
	InventoryList.EraseItem(ItemInstance);

	// 从复制系统中移除, 并且标记为垃圾
	ClearItemInstanceInternal(ItemInstance, MarkAsGarbage);
}

void ULyraInventoryManagerComponent::ClearItemInstanceInternal(ULyraInventoryItemInstance* ItemInstance, bool MarkAsGarbage)
{
	if (IsUsingRegisteredSubObjectList())
	{
		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ULyraInventoryManagerComponent::ClearItemInstanceInternal: Removing replicated subobject %s"), *GetNameSafe(ItemInstance));
		RemoveReplicatedSubObject(ItemInstance);
	}

	if (MarkAsGarbage)
	{
		ItemInstance->MarkAsGarbage();
	}
}


TArray<ULyraInventoryItemInstance*> ULyraInventoryManagerComponent::GetAllItemInstances() const
{
	return InventoryList.GetAllItemInstances();
}

int32 ULyraInventoryManagerComponent::GetInventoryCapacity() const
{
	return InventoryCapacity;
}

int32 ULyraInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()){return 0;}
	if (!ItemDefinition || NumToConsume <= 0){return 0;}


	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		int32 SlotIndex = InventoryList.FindSlotIndexByDefinition(ItemDefinition);
		if (InventoryList.IsValidSlotIndex(SlotIndex))
		{
			FLyraInventoryItem& Item = InventoryList.Items[SlotIndex];
			ULyraInventoryItemInstance* ItemInstance = Item.ItemInstance.Get();

			// Item Instance 可能堆叠多个
			int32 StackCount = Item.StackCount;
			int32 RemainingToConsume = NumToConsume - TotalConsumed;


			// 因为RemoveItemInstance里面处理了UObject销毁的逻辑, 所以如果数量归零, 流程一定要经过 RemoveItemInstance 否则内存泄漏


			if (StackCount > RemainingToConsume)
			{
				// 足够数量, 直接减少堆叠数量然后退出
				InventoryList.StackItem(ItemInstance, -RemainingToConsume);
				TotalConsumed += RemainingToConsume;

				UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ConsumeItemsByDefinition: Consumed %d from ItemInstance %s, Remaining StackCount %d"), RemainingToConsume, *GetNameSafe(ItemInstance), Item.StackCount);
				break;
			}

			// 数量不够或者相等, 全部消耗掉然后继续寻找下一个
			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ConsumeItemsByDefinition: Consumed all %d from ItemInstance %s"), StackCount, *GetNameSafe(ItemInstance));

			TotalConsumed += StackCount;
			RemoveItemInstance(ItemInstance);
		}
		else
		{
			break;
		}
	}

	return TotalConsumed;
}

int32 ULyraInventoryManagerComponent::ConsumeItemsByInstance(ULyraInventoryItemInstance* ItemInstance, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority()){return 0;}
	if (!ItemInstance || NumToConsume <= 0){return 0;}
	int32 SlotIndex = InventoryList.FindSlotIndexByInstance(ItemInstance);
	if (!InventoryList.IsValidSlotIndex(SlotIndex)){return 0;}


	// 因为RemoveItemInstance里面处理了UObject销毁的逻辑, 所以如果数量归零, 流程一定要经过 RemoveItemInstance 否则内存泄漏


	FLyraInventoryItem& Item = InventoryList.Items[SlotIndex];
	int32 StackCount = Item.StackCount;
	if (StackCount > NumToConsume)
	{
		// 足够数量, 直接减少堆叠数量然后退出
		InventoryList.StackItem(ItemInstance, -NumToConsume);

		UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ConsumeItemsByInstance: Consumed %d from ItemInstance %s, Remaining StackCount %d"), NumToConsume, *GetNameSafe(ItemInstance), Item.StackCount);
		return NumToConsume;
	}

	// 数量不够或者相等, 全部消耗掉然后移除
	UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> ConsumeItemsByInstance: Consumed all %d from ItemInstance %s"), StackCount, *GetNameSafe(ItemInstance));

	RemoveItemInstance(ItemInstance);
	return StackCount;
}

/*

 * ReadyForReplication() 和 ReplicateSubobjects() 两者都是为了确保 UObject
 子对象正确网络复制, 但它们的角色和调用时机不同
 *
 * ReadyForReplication()
 * 作用: 注册需要复制的子对象 调用 AddReplicatedSubObject()
 * 调用时机: 在网络复制开始前(准备阶段)被调用一次,
 通知引擎哪些子对象要被追踪复制
 * 特点: 建立“已注册子对象列表”主要供 Iris 复制系统优化使用
 *
 * ReplicateSubobjects()
 * 作用: 实际执行子对象属性复制的逻辑 调用 Channel->ReplicateSubobject()
 * 调用时机: 每个网络复制周期，且对每个连接都调用，负责写入子对象的复制数据
 * 特点: 兼容旧版方式, 也直接驱动子对象复制, 通过返回值告知是否写入任何内容
 * 关系:
 * ReadyForReplication() 负责提前注册子对象让系统知道“复制范围”,
 属于准备和管理层面
 * ReplicateSubobjects() 则在复制阶段一帧一帧地调用, 推进网络数据的实际传输
 *
 * 在 Iris 网络系统中, 推荐用 ReadyForReplication() + AddReplicatedSubObject()
 注册子对象, 提高性能和代码清晰
 * ReplicateSubobjects() 作为兼容方案或额外定制复制时仍会被调用。
 *
 */

void ULyraInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing ULyraInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FLyraInventoryItem& Item : InventoryList.Items)
		{
			ULyraInventoryItemInstance* Instance = Item.ItemInstance.Get();

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

ULyraInventoryItemInstance* ULyraInventoryManagerComponent::GetItemInstance(int32 SlotIndex) const
{
	ULyraInventoryItemInstance* ItemInstance = nullptr;
	if (InventoryList.IsValidSlotIndex(SlotIndex))
	{
		ItemInstance = InventoryList.Items[SlotIndex].ItemInstance.Get();
	}
	return ItemInstance;
}

int32 ULyraInventoryManagerComponent::GetItemStackCount(int32 SlotIndex) const
{
	int32 StackCount = -1;
	if (InventoryList.IsValidSlotIndex(SlotIndex))
	{
		StackCount = InventoryList.Items[SlotIndex].StackCount;
	}
	return StackCount;
}

/*
 * 这个实现是UE4古早写法, 如果启用了 bReplicateUsingRegisteredSubObjectList, 可以不写这个方法
 */

// bool ULyraInventoryManagerComponent::ReplicateSubobjects(
// 	UActorChannel* Channel, class FOutBunch* Bunch,
// 	FReplicationFlags* RepFlags)
// {
// 	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
//
// 	for (FLyraInventoryItem& Item : InventoryList.Items)
// 	{
// 		ULyraInventoryItemInstance* Instance = Item.Instance;
//
// 		if (Instance && IsValid(Instance))
// 		{
// 			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
// 		}
// 	}
//
// 	return WroteSomething;
// }

//////////////////////////////////////////////////////////////////////
//

// UCLASS(Abstract)
// class ULyraInventoryFilter : public UObject
// {
// public:
// 	virtual bool PassesFilter(ULyraInventoryItemInstance* Instance) const {
// return true; }
// };

// UCLASS()
// class ULyraInventoryFilter_HasTag : public ULyraInventoryFilter
// {
// public:
// 	virtual bool PassesFilter(ULyraInventoryItemInstance* Instance) const {
// return true; }
// };



void ULyraInventoryManagerComponent::InventoryDebugOut() const
{
	UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ============> Inventory Debug Output Start"));

	// UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> Item Count: %d"), InventoryList.Items.Num());
	for (const FLyraInventoryItem& Item : InventoryList.Items)
	{
		ULyraInventoryItemInstance* ItemInstance = Item.ItemInstance.Get();
		if (IsValid(ItemInstance))
		{
			TSubclassOf<ULyraInventoryItemDefinition> ItemDef = ItemInstance->GetItemDef();
			FString ItemDefName = ItemDef ? ItemDef->GetName() : TEXT("InvalidItemDef");

			UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ====> SlotIndex: %d, Item: %s, StackCount: %d, Owner: %s"),
				Item.SlotIndex,
				*ItemDefName,
				Item.StackCount,
				*GetNameSafe(ItemInstance->GetOuter()));
		}
	}

	UE_LOG(LogLyraInventory, Warning, TEXT("LyraInventory ============> Inventory Debug Output End"));
}

void ULyraInventoryManagerComponent::ServerInventoryDebugOut_Implementation() const
{
	InventoryDebugOut();
}
