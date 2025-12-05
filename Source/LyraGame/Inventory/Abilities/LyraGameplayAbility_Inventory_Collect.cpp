// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraGameplayAbility_Inventory_Collect.h"

#include "AbilitySystemComponent.h"
#include "LyraGameplayTags.h"
#include "Interaction/LyraWorldCollectable.h"
#include "Inventory/LyraInventoryManagerComponent.h"

ULyraGameplayAbility_Inventory_Collect::ULyraGameplayAbility_Inventory_Collect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	ActivationPolicy = ELyraAbilityActivationPolicy::OnInputTriggered;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void ULyraGameplayAbility_Inventory_Collect::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                            const FGameplayAbilityActorInfo* ActorInfo,
                                                            const FGameplayAbilityActivationInfo ActivationInfo,
                                                            const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bool bSuccess = false;
	do
	{
		// 取得被拾取物Actor
		const ALyraWorldCollectable* ConstObj = Cast<ALyraWorldCollectable>(TriggerEventData->Target);
		ALyraWorldCollectable* CollectableActor = const_cast<ALyraWorldCollectable*>(ConstObj);
		if (!CollectableActor) { break; }
		if (!CollectableActor->CanBePickedUp()) { break; }

		// 动作发起者, 即当前玩家
		const AActor* Instigator = TriggerEventData->Instigator.Get();
		const APawn* InstigatorPawn = Cast<APawn>(Instigator);
		if (!InstigatorPawn) { break; }

		// 取得拾取物品信息
		FInventoryPickup Pickup = CollectableActor->GetPickupInventory();
		if (Pickup.Instances.Num() == 0 && Pickup.Definitions.Num() == 0) { break; }

		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		AController* Controller = GetControllerFromActorInfo();
		if (!Controller || !ASC) { break; }

		// 背包组件存在于Controller上
		ULyraInventoryManagerComponent* InventoryManager = Controller->FindComponentByClass<ULyraInventoryManagerComponent>();
		if (!InventoryManager) { break; }

		EInventoryStackBehavior StackBehavior = EInventoryStackBehavior::ESB_StackIntoExisting;
		if (TriggerEventData->InstigatorTags.HasTag(LyraGameplayTags::Ability_Interaction_Pickup_Active))
		{
			StackBehavior = EInventoryStackBehavior::ESB_StackOrCreateNew;
		}
		if (TriggerEventData->InstigatorTags.HasTag(LyraGameplayTags::Ability_Interaction_Pickup_Auto))
		{
			StackBehavior = EInventoryStackBehavior::ESB_StackIntoExisting;
		}


		// 查看背包是否可以容纳
		if (InventoryManager->CanAddItem(CollectableActor, StackBehavior) != EInventoryCanAddItemResult::EICAR_Success) { break; }

		// 这将在服务端停止 Projectile Component, 停止 Actor 碰撞, Tick, 播放CUE
		// 将在客户端 停止 Actor 碰撞, Tick, (通过属性同步), 播放CUE
		CollectableActor->NotifyPickedUp(ASC, InstigatorPawn);

		if (ActorInfo->IsNetAuthority())
		{
			FInventoryPickup RestPickup;
			InventoryManager->AddItem(CollectableActor,RestPickup, StackBehavior);

			// 这里需要检查是否全部拾取成功, 如果没有全部拾取成功则需要将剩余的物品放回到世界中
			if (RestPickup.Instances.Num() > 0 || RestPickup.Definitions.Num() > 0)
			{
				CollectableActor->SetPickupInventory(RestPickup);

				// 这将在服务端启动 Projectile Component, 启动 Actor 碰撞, Tick
				// 将在客户端 启动 Actor 碰撞, Tick, 重置 Mesh Transform
				CollectableActor->NotifyDropped(ASC, InstigatorPawn);
			}
			else
			{
				// Actor 的销毁工作由它自己完成
				CollectableActor->NotifyFinished(ASC, InstigatorPawn);
			}
		}

		bSuccess = true;
	}
	while (false);

	if (bSuccess)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	else
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}
