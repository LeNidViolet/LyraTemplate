// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Messages/LyranotificationMessage_Inventory.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LyraInventoryManagerComponent.generated.h"

#define UE_API LYRAGAME_API

class ALyraWorldCollectable;
struct FInventoryPickup;
class ULyraInventoryItemDefinition;
class ULyraInventoryItemInstance;
class ULyraInventoryManagerComponent;
class UObject;
struct FFrame;
struct FLyraInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FLyraInventoryItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLyraInventoryItem() {}

	FString GetDebugString() const;

	bool UpdateItem(ULyraInventoryItemInstance* InInstance, int32 InStackCount);

private:
	friend FLyraInventoryList;
	friend ULyraInventoryManagerComponent;

	// 当前节点索引 也就是物品在库存中的位置 这个是不变的
	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	// 物品实例
	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> ItemInstance = nullptr;
	// 物品堆叠数量
	UPROPERTY()
	int32 StackCount = 0;

	// 最后记录的数据, 用于变更检测
	UPROPERTY(NotReplicated)
	TObjectPtr<ULyraInventoryItemInstance> LastObservedInstance = nullptr;
	UPROPERTY(NotReplicated)
	int32 LastObservedStackCount = 0;

	// 注意在添加字段时需要 增删查改 接口里面的同步处理
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FLyraInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FLyraInventoryList() : OwnerComponent(nullptr){}

	FLyraInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent){}

	// 初始化库存列表(空)
	void Initialize(int32 Capacity);

	bool IsValidSlotIndex(int32 Index) const;

	// 获取所有物品实例
	TArray<ULyraInventoryItemInstance*> GetAllItemInstances() const;

	// 找到首个空闲位置
	int32 FindFreeSlotIndex() const;
	// 获取当前空闲库存数量
	int32 GetFreeSlotsCount() const;

	// 通过物品实例查找物品索引
	int32 FindSlotIndexByInstance(ULyraInventoryItemInstance* ItemInstance) const;
	// 通过物品定义查找物品索引
	int32 FindSlotIndexByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition) const;
	// 通过物品定义查找首个物品实例
	ULyraInventoryItemInstance* FindItemInstanceByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition) const;

	// 获取指定物品定义的实例数量(占用的slot数量)
	int32 GetSlotsCountByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition) const;

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FLyraInventoryItem, FLyraInventoryList>(
			Items, DeltaParms, *this);
	}

	// FLyraInventoryList 目前只通过 ULyraInventoryItemInstance 进行物品操作
	// 如果需要通过 Definition 进行操作, 在 ULyraInventoryManagerComponent
	// 中生成对应的实例再调用下面的接口 分配一个物品实例到库存空闲位置
	bool AllocItem(ULyraInventoryItemInstance* ItemInstance, int32 StackCount);
	// 从库存中移除指定物品实例
	void EraseItem(ULyraInventoryItemInstance* ItemInstance);
	// 变更物品堆叠数量, 如果 NewCount <= 0 则移除该物品实例, 增量为 Delta
	bool StackItem(ULyraInventoryItemInstance* ItemInstance, int32 Delta);
	// 交换两个物品槽位
	bool SwapItem(int32 SourceSlotIndex, int32 TargetSlotIndex);
	// 合并两个物品数量
	bool StackItem(int32 SourceSlotIndex, int32 TargetSlotIndex);

private:
	void BroadcastChangeMessage(FLyraInventoryItem& Item,
	                            EInventoryStackChangeMessageType MessageType,
	                            int32 OldCount, int32 NewCount);

	// 物品实例是否存在于库存中
	bool IsItemInstanceInInventory(ULyraInventoryItemInstance* ItemInstance) const;

private:
	friend ULyraInventoryManagerComponent;

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FLyraInventoryItem> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template <>
struct TStructOpsTypeTraits<FLyraInventoryList>
	: public TStructOpsTypeTraitsBase2<FLyraInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

// 定义库存添加物品时的堆叠行为
UENUM(BlueprintType)
enum class EInventoryStackBehavior : uint8
{
	// 堆入现有堆叠 已有 slot
	ESB_StackIntoExisting UMETA(DisplayName = "Stack Into Existing"),
	// 创建新堆叠
	ESB_CreateNewStack UMETA(DisplayName = "Create New Stack"),
	// 尝试堆入现有堆叠, 如果没有空间则创建新堆叠
	ESB_StackOrCreateNew UMETA(DisplayName = "Stack Or Create New"),
};

// 定义不能添加物品的原因
UENUM(BlueprintType)
enum class EInventoryCanAddItemResult : uint8
{
	// 可以添加
	EICAR_Success UMETA(DisplayName = "Success"),
	// 无效的请求
	EICAR_InvalidRequest UMETA(DisplayName = "Invalid Request"),
	// 库存已满
	EICAR_InventoryFull UMETA(DisplayName = "Inventory Full"),
	// 已达最大持有数量
	EICAR_ExceedsMaxItemCount UMETA(DisplayName = "Carry Limit"),
	// 已达最大堆叠数量
	EICAR_ExceedsMaxStackCount UMETA(DisplayName = "Stack Full"),
	// 物品定义无效
	EICAR_InvalidItemDefinition UMETA(DisplayName = "Bad Def"),
	// 物品实例无效
	EICAR_InvalidItemInstance UMETA(DisplayName = "Bad Inst"),
	// 物品实例已存在于库存中
	EICAR_ItemInstanceAlreadyInInventory UMETA(DisplayName = "Exists"),
};

// 拾取逻辑:
// 直接使用 WorldCollectable, 通过操作其中的 InventoryPickup 来逐一进行添加
// 考虑到物品可能不会全部添加成功, 因此剩余的物品应该使用原 WorldCollectable
// 放回世界内; 调用者应当检查是否全部添加成功, 并负责 WorldCollectable
// 的销毁操作 如果 Instance 只有部分能放进背包, 会创建一个新的 Instance
// 放到背包中, 原 Instance 会更新剩余数量留在 WorldCollectable 中
// 如果全部添加成功, Instance 会随 WorldCollectable 销毁, 背包中通过DuplicateObject创建新的 Instance


// 丢弃逻辑:
// 如果是全部丢弃, 将创建新的 Instance 放入 InventoryPickup, 原 Instance 销毁
// 如果是部分丢弃, 将创建新的 Instance 放入 InventoryPickup, 原 Instance 留在背包中

/**
 * Manages an inventory
 */
UCLASS(MinimalAPI, BlueprintType)
class ULyraInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UE_API ULyraInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 检查是否可以添加拾取物到库存中(数组中可能包含多个物品定义和实例)
	UFUNCTION(BlueprintCallable, Category = Inventory)
	UE_API EInventoryCanAddItemResult CanAddItem(
		ALyraWorldCollectable* Collectable,
		EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew) const;

	// 检查是否可以添加拾取物到库存中(物品定义)
	UFUNCTION(BlueprintCallable, Category = Inventory)
	UE_API EInventoryCanAddItemResult CanAddItemForDefinition(
		TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
		int32 StackCount = 1,
		EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew) const;

	// 检查是否可以添加拾取物到库存中(物品实例)
	UFUNCTION(BlueprintCallable, Category = Inventory)
	UE_API EInventoryCanAddItemResult CanAddItemForInstance(
		ULyraInventoryItemInstance* ItemInstance,
		int32 StackCount = 1,
		EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew) const;

	// 向库存中添加拾取物(数组中可能包含多个物品定义和实例)
	// 注意如果通过 Instance 添加的话, 当方法返回时, Instance 有可能已经释放
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API int32 AddItem(
		ALyraWorldCollectable* Collectable,
		FInventoryPickup& RestPickup,
		EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew);

	// 向库存中添加拾取物(物品定义)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API int32 AddItemDefinition(
		TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
	    int32 StackCount = 1,
	    EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew);

	// 向库存中添加拾取物(物品实例)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API int32 AddItemInstance(
		ULyraInventoryItemInstance* ItemInstance,
		AActor* OwnerActor,
		int32 StackCount = 1,
		EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew);

	// 从库存中丢弃物品 因为库存中都是 ItemInstance, 所以必须通过 ItemInstance 来丢弃
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API bool DropItem(
		ULyraInventoryItemInstance* ItemInstance,
		int32 StackCount,
		ULyraInventoryItemInstance*& OutDroppedInstance,
		int32& OutDroppedStackCount);

	// 交换库存槽位
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API bool SwapItem(
		ULyraInventoryItemInstance* SourceSlotItemInstance,
		int32 SourceSlotIndex,
		int32 TargetSlotIndex);

	// 合并库存槽位
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API bool StackItem(
		ULyraInventoryItemInstance* SourceSlotItemInstance,
		int32 SourceSlotIndex,
		int32 TargetSlotIndex);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API void RemoveItemInstance(ULyraInventoryItemInstance* ItemInstance, bool MarkAsGarbage = true);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Inventory)
	UE_API TArray<ULyraInventoryItemInstance*> GetAllItemInstances() const;

	// 获取库存总容量
	UFUNCTION(BlueprintPure, Category = Inventory)
	UE_API int32 GetInventoryCapacity() const;

	// 消费库存
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API int32 ConsumeItemsByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition, int32 NumToConsume);

	// 消费库存
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory)
	UE_API int32 ConsumeItemsByInstance(ULyraInventoryItemInstance* ItemInstance, int32 NumToConsume);

	//~UObject interface
	// UE_API virtual bool ReplicateSubobjects(class UActorChannel* Channel,
	//                                         class FOutBunch* Bunch,
	//                                         FReplicationFlags* RepFlags)
	//                                         override;
	UE_API virtual void ReadyForReplication() override;
	//~UObject interface

	UFUNCTION(BlueprintPure, Category = Inventory)
	UE_API ULyraInventoryItemInstance* GetItemInstance(int32 SlotIndex) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	UE_API int32 GetItemStackCount(int32 SlotIndex) const;

private:
	EInventoryCanAddItemResult CanAddItemToExisting(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition, int32 StackCount) const;
	EInventoryCanAddItemResult CanAddItemToNew(TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition, int32 StackCount) const;

	// 查看指定物品实例是否存在于库存中
	bool IsItemInstanceInInventory(ULyraInventoryItemInstance* ItemInstance) const;

	// 添加物品实例, 返回实际添加的数量
	int32 AddItemInstanceInternal(
		bool FromInstance,
		AActor* OwnerActor,
		TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
		ULyraInventoryItemInstance* ItemInstance, int32 StackCount,
		EInventoryStackBehavior StackBehavior);

	// 最终清理 ItemInstance
	void ClearItemInstanceInternal(ULyraInventoryItemInstance* ItemInstance, bool MarkAsGarbage = true);

	UPROPERTY(Replicated)
	FLyraInventoryList InventoryList;

	// 库存容量 具体数值从GameData中获取
	int32 InventoryCapacity = 0;


public:
	// 打印背包数据
	void InventoryDebugOut() const;

	UFUNCTION(Server, Reliable)
	void ServerInventoryDebugOut() const;
};

#undef UE_API
