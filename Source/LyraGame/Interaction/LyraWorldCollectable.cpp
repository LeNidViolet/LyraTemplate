// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWorldCollectable.h"

#include "AbilitySystemComponent.h"
#include "Async/TaskGraphInterfaces.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Inventory/InventoryFragment_PickupInfo.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "System/LyraSystemStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraWorldCollectable)

struct FInteractionQuery;

ALyraWorldCollectable::ALyraWorldCollectable()
{
	bReplicates = true;
	SetReplicateMovement(true);
	SetCanBeDamaged(false);

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("SphereComp"));
	// CollisionComp 的碰撞属性在蓝图中设置
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("FallMovementComp"));
	ProjectileComp->UpdatedComponent = RootComponent;
	ProjectileComp->InitialSpeed = 0.f;
	ProjectileComp->MaxSpeed = 1000.f;
	ProjectileComp->bShouldBounce = true;
	ProjectileComp->Bounciness = 0.f;		// 掉落物不反弹
	ProjectileComp->Friction = 0.f;			// 初始摩擦为0 以实现撞墙下落
	ProjectileComp->ProjectileGravityScale = 1.f;

	if (!HasAuthority())
	{
		// Projectile 只在 Standalone / Server 中运行
		// Client 主要靠服务端同步位置
		SetProjectileActive(false);
	}
}

void ALyraWorldCollectable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALyraWorldCollectable, StaticInventory);
	DOREPLIFETIME(ALyraWorldCollectable, CollectableStateData);
	DOREPLIFETIME(ALyraWorldCollectable, bHasLanded);
}




void ALyraWorldCollectable::BeginPlay()
{
	Super::BeginPlay();

	// 如果设置了默认的StaticInventory(在蓝图里), 这里手动调用一次OnRep函数以触发相应逻辑 (主要是 Client / Standalone 模式下的可视化)
	// 因为此时StaticInventory不会通过复制系统同步到客户端
	// 如果后续服务端数据发生变化, 则会通过复制系统同步到客户端
	// 并再次调用OnRep函数
	if (StaticInventory.Definitions.Num() > 0 || StaticInventory.Instances.Num() > 0)
	{
		OnRep_StaticInventory();
	}


	// Standalone / Server 绑定反弹事件
	// Client 等待 Server 通知
	if (HasAuthority())
	{
		ProjectileComp->OnProjectileBounce.AddUniqueDynamic(this, &ALyraWorldCollectable::HandleProjectileBounce);
	}
}

void ALyraWorldCollectable::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ALyraWorldCollectable::SetActive(bool bNewActive)
{
	// 客户端CUE通过Mesh来做拾取的效果, 这里不能隐藏 Actor 会导致 Mesh 也被隐藏
	// 另外由于 SetActorEnableCollision 不会同步到客户端, 所以客户端也应调用本函数

	// SetActorHiddenInGame(!bNewActive);	// 会同步
	SetActorEnableCollision(bNewActive);	// 不会同步
	SetActorTickEnabled(bNewActive);		// 不会同步
}


void ALyraWorldCollectable::ResetStaticMesh()
{
	// 默认Cue通过Mesh来做拾取的效果, 所以这里需要重置 Mesh 的 Transform
	// 如果换成了其他方式的Cue, 这里的操作也没有副作用

	if (!IsNetMode(NM_DedicatedServer))
	{
		// 只有客户端 Mesh 会变更Transform
		MeshComp->SetVisibility(true, true);
		MeshComp->SetWorldLocation(GetActorLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		MeshComp->SetWorldScale3D(FVector::OneVector);
		MeshComp->SetRelativeLocation(FVector::ZeroVector);
		MeshComp->SetRelativeScale3D(FVector::OneVector);
	}
}

bool ALyraWorldCollectable::CanBePickedUp() const
{
	return CollectableStateData.CollectableState == ECollectableState::ECS_Active;
}

FInventoryPickup ALyraWorldCollectable::GetPickupInventory() const
{
	return StaticInventory;
}

void ALyraWorldCollectable::SetPickupInventory(const FInventoryPickup& InInventoryPickup)
{
	if (HasAuthority())
	{
		StaticInventory = InInventoryPickup;
        OnRep_StaticInventory();
	}
}


void ALyraWorldCollectable::OnRep_StaticInventory()
{
	// 在客户端/服务端都调整一下 Collision 以适应新的物品形状
	// 主要是客户端需要调整碰撞以便拾取检测
	FitCollisionToMesh();

	if (!IsNetMode(NM_DedicatedServer))
	{
		// 目前只在 Standalone / Client 进行可视化更新

		// 蓝图可在此进行可视化处理
		K2_OnUpdated();
		OnUpdated.Broadcast();
	}
}

void ALyraWorldCollectable::OnRep_CollectableStateData()
{
	if (CollectableStateData.CollectableState == ECollectableState::ECS_Active)
	{
		ResetStaticMesh();
		SetActive(true);

		// 被重新激活, 就是重新丢出来了
		K2_OnDropped(CollectableStateData.LastInstigatorPawn.Get());
		OnDropped.Broadcast(CollectableStateData.LastInstigatorPawn.Get());
	}
	if (CollectableStateData.CollectableState == ECollectableState::ECS_PickedUp)
	{
		SetActive(false);

		// 拾取
		K2_OnPickupStarted(CollectableStateData.LastInstigatorPawn.Get());
		OnPickupStarted.Broadcast(CollectableStateData.LastInstigatorPawn.Get());
	}
	if (CollectableStateData.CollectableState == ECollectableState::ECS_CoolingDown)
	{
		// 这里等待服务器重新激活就好了, 没什么要做的
		K2_OnCoolingDown();
	}
	if (CollectableStateData.CollectableState == ECollectableState::ECS_Finished)
	{
		// 即将销毁
		K2_OnFinished();
		OnFinished.Broadcast();
	}
}

void ALyraWorldCollectable::HandleProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// 这里只在 Standalone / Server 时被执行

	// 判断是否落地 (Z > 0.7f 表示撞击面大致朝上)
	if (ImpactResult.ImpactNormal.Z > 0.7f)
	{
		// Server: 主动调用 OnRep_HasLanded 修改状态, 并引发客户端 OnRep_HasLanded 同步修改状态
		// Standalone: 直接调用 OnRep_HasLanded 修改状态 (不会触发 OnRep)
		// Client: 等待 Server 通知, 被动调用 OnRep_HasLanded 修改状态

		if (bHasLanded) return;

		bHasLanded = true;
		OnRep_HasLanded();

		SetProjectileActive(false);
	}
}

void ALyraWorldCollectable::OnRep_HasLanded()
{
	if (bHasLanded)
	{
		K2_OnLanded();
		OnLanded.Broadcast();
	}
}

void ALyraWorldCollectable::SetProjectileActive(bool bNewActive)
{
	// 只有 Standalone / Server 会调用本函数
	if (!HasAuthority()) return;

	if (bNewActive)
	{
		if (!ProjectileComp->UpdatedComponent)
		{
			ProjectileComp->SetUpdatedComponent(RootComponent);
		}
		ProjectileComp->SetComponentTickEnabled(true);
		ProjectileComp->Activate();

		ProjectileComp->Friction = 0.f;
		// 给个初速度 (或者直接设为 0，只要 Tick 开了，重力通常会自动接管) 但给个非零值最稳妥
		ProjectileComp->Velocity = FVector(0, 0, -10.0f);
	}
	else
	{
		ProjectileComp->Deactivate();
		ProjectileComp->Velocity = FVector::ZeroVector; // 清空速度
		ProjectileComp->Friction = 10.f; // 增加摩擦力防止继续滑动
		ProjectileComp->SetComponentTickEnabled(false);	// 省点电
	}
}


void ALyraWorldCollectable::NotifyPickedUp(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn)
{
	// 播放拾取Cue
	// 客户端 SetActive 如果依赖 OnRep_CollectableStateData, 可能会滞后于 Cue, 所以在 Cue 中也要调用 SetActive
	if (ASC && GameplayCueTagCollect.IsValid())
	{
		FGameplayCueParameters CueParameters;
		CueParameters.Instigator = const_cast<APawn*>(InstigatorPawn);
		CueParameters.EffectCauser = this;
		CueParameters.SourceObject = this;
		ASC->ExecuteGameplayCue(GameplayCueTagCollect, CueParameters);
	}

	// Standalone / Server
	if (!HasAuthority()) return ;

	ensure(CollectableStateData.CollectableState == ECollectableState::ECS_Active);

	// 停止模拟运动
	SetProjectileActive(false);

	// Server / Standalone 调用进 OnRep_CollectableStateData 由于直接调用
	// Client 调用进 OnRep_CollectableStateData 由于复制系统同步
	// 所以两端都会触发 OnRep_CollectableStateData 回调
	// 这需要 OnRep_CollectableStateData 写下两端通用的逻辑


	// 修改 CollectableStateData 导致  Client OnRep 回调
	CollectableStateData.LastInstigatorPawn = const_cast<APawn*>(InstigatorPawn);
	CollectableStateData.CollectableState = ECollectableState::ECS_PickedUp;

	// Server / Standalone 需要手动触发
	OnRep_CollectableStateData();
}

void ALyraWorldCollectable::NotifyDropped(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn)
{
	// Standalone / Server
	if (!HasAuthority()) return;

	ensure(CollectableStateData.CollectableState != ECollectableState::ECS_CoolingDown);

	// 首先触发 CoolingDown
	// CoolingDown 时间过后重新 Activate

	// 修改 CollectableStateData 导致  Client OnRep 回调
	CollectableStateData.LastInstigatorPawn = const_cast<APawn*>(InstigatorPawn);
	CollectableStateData.CollectableState = ECollectableState::ECS_CoolingDown;

	// Server / Standalone 需要手动触发
	OnRep_CollectableStateData();



	// 设置一个定时器, Collectable 的状态需要修改为 Active 才可以被再次拾取

	TWeakObjectPtr<ALyraWorldCollectable> WeakThis(this);
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
	TWeakObjectPtr<const APawn> WeakInstigatorPawn(InstigatorPawn);

	// 设置定时器 稍后改为 Active
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [WeakThis, WeakASC, WeakInstigatorPawn]()
	{
		// 检查 Actor 是否还活着
		if (ALyraWorldCollectable* StrongThis = WeakThis.Get())
		{
			StrongThis->OnCoolingDownTimeout(WeakASC.Get(), WeakInstigatorPawn.Get());
		}
	}, DropCooldownDuration, false);
}

void ALyraWorldCollectable::OnCoolingDownTimeout(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn)
{
	// 对于 Projectile 的设置要在服务端进行, 所以不能放在 OnRep_CollectableStateData
	SetProjectileActive(true);

	AActor* Actor = this;
	if (InstigatorPawn) Actor = const_cast<APawn*>(InstigatorPawn);


	// 这里有三个异步操作
	// 1. 将Actor位置设置为丢弃位置, 这将通过同步方式更新到客户端
	// 2. 播放丢弃Cue, 这将在客户端播放
	// 3. 修改 CollectableStateData 导致客户端 OnRep_CollectableStateData 回调
	// 这三个异步操作在客户端执行顺序不固定


	FVector NewLocation;
	bool bOk = ULyraSystemStatics::FindValidSpawnLocationInCone(
		NewLocation,
		Actor);
	if (!bOk)
	{
		NewLocation = GetActorLocation();
	}
	// 放回到地面上
	SetActorLocation(NewLocation);

	// 播放丢弃Cue
	// TODO 这里如果设置了Cue的话这个特效不会在Owner客户端播放? 因为这里是在Authority下运行的
	if (ASC && GameplayCueTagDrop.IsValid())
	{
		FGameplayCueParameters CueParameters;
		CueParameters.Instigator = const_cast<APawn*>(InstigatorPawn);
		CueParameters.EffectCauser = this;
		CueParameters.SourceObject = this;
		CueParameters.Location = NewLocation;	// 最终丢弃位置也传递过去
		ASC->ExecuteGameplayCue(GameplayCueTagDrop, CueParameters);
	}

	CollectableStateData.CollectableState = ECollectableState::ECS_Active;
	CollectableStateData.LastInstigatorPawn = const_cast<APawn*>(InstigatorPawn);
	// ResetStaticMesh, SetActive
	OnRep_CollectableStateData();
}



void ALyraWorldCollectable::NotifyFinished(UAbilitySystemComponent* ASC, const APawn* InstigatorPawn)
{
	if (!HasAuthority()) return;

	ensure(CollectableStateData.CollectableState != ECollectableState::ECS_Finished);

	// 修改 CollectableStateData 导致  Client OnRep 回调
	CollectableStateData.LastInstigatorPawn = const_cast<APawn*>(InstigatorPawn);
	CollectableStateData.CollectableState = ECollectableState::ECS_Finished;

	// Server / Standalone 需要手动触发
	OnRep_CollectableStateData();

	// 在服务端销毁它
	this->SetLifeSpan(LifeAfterFinished);
}



void ALyraWorldCollectable::FitCollisionToMesh()
{
	// 根据 Mesh 重新调整碰撞盒大小
	ULyraInventoryItemDefinition* ItemDefinition = nullptr;
	if (StaticInventory.Definitions.Num() > 0)
	{
		ItemDefinition = StaticInventory.Definitions[0].ItemDef.GetDefaultObject();
	}
	else if (StaticInventory.Instances.Num() > 0)
	{
		if (StaticInventory.Instances[0].Item)
			ItemDefinition = StaticInventory.Instances[0].Item->GetItemDef().GetDefaultObject();
	}
	if (ItemDefinition)
	{
		const UInventoryFragment_PickupInfo* PickupInfo = Cast<UInventoryFragment_PickupInfo>(ItemDefinition->FindFragmentByClass(UInventoryFragment_PickupInfo::StaticClass()));
		if (PickupInfo && PickupInfo->DisplayMesh)
		{
			FBoxSphereBounds Bounds = PickupInfo->DisplayMesh->GetBounds();
			FVector Scale3D = CollisionComp->GetRelativeScale3D();

			CollisionComp->SetBoxExtent(Bounds.BoxExtent * Scale3D);
			MeshComp->SetRelativeLocation(Bounds.Origin * Scale3D * -1.0f);
		}
	}
}


ALyraWorldCollectable* ALyraWorldCollectable::SpawnCollectableForDefinition(
	UObject* WorldContextObject,
	TSubclassOf<ULyraInventoryItemDefinition> ItemDefinition,
	int32 SpawnCount,
	const FVector& Location,
	const FRotator& Rotation,
	APawn* InstigatorPawn,
	AActor* OwnerActor)
{
	FInventoryPickup InventoryPickup;
	InventoryPickup.Instances.SetNum(0);
	InventoryPickup.Definitions.SetNum(1);
	InventoryPickup.Definitions[0].ItemDef = ItemDefinition;
	InventoryPickup.Definitions[0].StackCount = SpawnCount;

	return SpawnCollectable(
		WorldContextObject,
		InventoryPickup,
		Location,
		Rotation,
		InstigatorPawn,
		OwnerActor);
}

ALyraWorldCollectable* ALyraWorldCollectable::SpawnCollectableForInstance(
	UObject* WorldContextObject,
	ULyraInventoryItemInstance* ItemInstance,
	int32 SpawnCount,
	const FVector& Location,
	const FRotator& Rotation,
	APawn* InstigatorPawn,
	AActor* OwnerActor)
{
	FInventoryPickup InventoryPickup;
	InventoryPickup.Instances.SetNum(1);
	InventoryPickup.Instances[0].Item = ItemInstance;
	InventoryPickup.Instances[0].StackCount = SpawnCount;
	InventoryPickup.Definitions.SetNum(0);

	return SpawnCollectable(
		WorldContextObject,
		InventoryPickup,
		Location,
		Rotation,
		InstigatorPawn,
		OwnerActor);
}

ALyraWorldCollectable* ALyraWorldCollectable::SpawnCollectable(
	UObject* WorldContextObject,
	FInventoryPickup InventoryPickup,
	const FVector& Location,
	const FRotator& Rotation,
	APawn* InstigatorPawn,
	AActor* OwnerActor)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	if (InventoryPickup.Definitions.Num() == 0 && InventoryPickup.Instances.Num() == 0)
	{
		return nullptr;
	}

	TSubclassOf<ALyraWorldCollectable> WorldCollectableClass;
	// 需要每一个 Definition 和 Instance 都合法
	for (int32 i = 0; i < InventoryPickup.Definitions.Num(); i++)
	{
		if (InventoryPickup.Definitions[i].StackCount <= 0 ||
			!InventoryPickup.Definitions[i].ItemDef)
		{
			return nullptr;
		}

		WorldCollectableClass = InventoryPickup.Definitions[i].ItemDef.GetDefaultObject()->WorldCollectableClass;
	}
	for (int32 i = 0; i < InventoryPickup.Instances.Num(); i++)
	{
		if (InventoryPickup.Instances[i].StackCount <= 0 || !InventoryPickup.Instances[i].Item.Get())
		{
			return nullptr;
		}
		WorldCollectableClass = InventoryPickup.Instances[i].Item.Get()->GetItemDef().GetDefaultObject()->WorldCollectableClass;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}
	// 仅在服务器生成
	if (World->IsNetMode(NM_Client))
	{
		return nullptr;
	}


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.Instigator = InstigatorPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ALyraWorldCollectable* SpawnedCollectable = World->SpawnActor<ALyraWorldCollectable>(
		WorldCollectableClass,
		Location,
		Rotation,
		SpawnParams);
	if (!SpawnedCollectable)
	{
		return nullptr;
	}

	// 如果是通过 ItemInstance 创建的 Actor 这里需要转移所有权以及复制队列 以确保 ItemInstance 能够正确复制到客户端
	for (const FPickupInstance& PickupInstance : InventoryPickup.Instances)
	{
		ULyraInventoryItemInstance* ItemInstance = PickupInstance.Item.Get();
		ItemInstance->Rename(nullptr, SpawnedCollectable);
		SpawnedCollectable->AddReplicatedSubObject(PickupInstance.Item.Get());
	}

	SpawnedCollectable->SetPickupInventory(InventoryPickup);

	return SpawnedCollectable;
}
