// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraWorldMarker.h"

#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraLogChannels.h"
#include "LyraGameplayTags.h"
#include "GameModes/Session/LyraDSPlayerState.h"
#include "Messages/LyraNotificationMessage.h"
#include "Messages/LyraNotificationMessage_Marker.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraWorldMarker)

int32 ALyraWorldMarker::NextMarkerId = 1;

ALyraWorldMarker* ALyraWorldMarker::SpawnLyraWorldMarkerActor(UObject* WorldContextObject,
	TSubclassOf<ALyraWorldMarker> MarkerClass, AActor* TargetActor, const FVector& TargetLocation,
	ELyraWorldMarkerType MarkerType, APlayerState* OwnerPlayer, bool bAsPredictedMarker)
{
	if (!WorldContextObject) return nullptr;
	if (!MarkerClass) return nullptr;
	if (!OwnerPlayer) return nullptr;
	if (!TargetActor) return nullptr;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// TODO 做位置检测 TargetActor Location 的合法性
	// TODO 检测该玩家已有标记点数量限制

	// 调用者需要确保传入了正确的PlayerState
	ALyraDSPlayerState* DSPlayerState = Cast<ALyraDSPlayerState>(OwnerPlayer);
	if (!DSPlayerState)
	{
		return nullptr;
	}


	if (bAsPredictedMarker)
	{
		// 预测标记点只能在客户端生成
		if (DSPlayerState->HasAuthority())
		{
			return nullptr;
		}

		// 移除已有的预测标记点
		DSPlayerState->RemoveWorldMarkerFromCache(ALyraWorldMarker::PredictedId());
	}
	else
	{
		// 非预测标记点只能在服务器生成
		if (!DSPlayerState->HasAuthority())
		{
			return nullptr;
		}

		// TODO 目前只允许每种类型一个标记点
		DSPlayerState->RemoveWorldMarkerFromCache(MarkerType);
	}

	// 生成 Actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = nullptr;	// 标记点不设置Owner
	SpawnParams.Instigator = nullptr;
	FTransform SpawnTransform(FRotator::ZeroRotator, TargetLocation);

	ALyraWorldMarker* NewMarker = World->SpawnActorDeferred<ALyraWorldMarker>(
		MarkerClass,
		SpawnTransform,
		nullptr,
		nullptr
	);
	if (!NewMarker)
	{
		return nullptr;
	}

	FLyraWorldMarkerData InMarkerData;
	InMarkerData.OwnerPlayer = OwnerPlayer;
	InMarkerData.MarkerType = MarkerType;
	InMarkerData.Color = DSPlayerState->PlayerColor;
	InMarkerData.MarkerId = bAsPredictedMarker ? ALyraWorldMarker::PredictedId() : NextMarkerId++;
	InMarkerData.bPredictedMarker = bAsPredictedMarker;

	// 在 BeginPlay 之前设置参数
	NewMarker->Initialize(InMarkerData);

	// 保存到PlayerState
	// 服务端保存的是真实标记点 客户端保存的是预测标记点(客户端只会保存一个预测标记点)
	DSPlayerState->AddWorldMarkerToCache(NewMarker);

	if (MarkerType != ELyraWorldMarkerType::Waypoint)
	{
		// 如果不是世界标记点 就附加到目标 Actor 上, 并且是完全贴合 这会使NewMarker的位置随TargetActor移动而移动
		NewMarker->AttachToActor(TargetActor, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
	// 调用 FinishSpawningActor，此时 BeginPlay() 等生命周期函数会被触发
	NewMarker->FinishSpawning(SpawnTransform);

	return NewMarker;
}

ALyraWorldMarker::ALyraWorldMarker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 标记点不需要高频更新
	SetNetUpdateFrequency(1.f);
	bReplicates = true;

	// 确保所有客户端都能看到
	bAlwaysRelevant = true;

	// Disable culling by distance for world markers
	// SetNetCullDistanceSquared(225000000.f);

	// 创建静态网格组件 作为根组件 也作为向屏幕投影的体积
	SphereComp = CreateDefaultSubobject<USphereComponent>(FName("SphereComp"));
	// 目前没有使用碰撞球
	SphereComp->SetSphereRadius(50.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 调试打开显示
	// SphereComp->SetHiddenInGame(false);
	SetRootComponent(SphereComp);
}



void ALyraWorldMarker::BeginPlay()
{
	Super::BeginPlay();

	bComponentsInitialized = true;

	// 如果依附了 Actor，绑定它的销毁事件; 这可以在创建的时候进行Attach
	if (AActor* Parent = GetAttachParentActor())
	{
		Parent->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnParentDestroyed);
	}

	// 我们还需要响应一下标记查询请求
	// 如果Actor先于MarkerManager初始化完毕 此种情况下后者就无法得知该标记点的存在
	// 对于后加入的玩家可能存在此种情况
	if (!IsNetMode(NM_DedicatedServer))
	{
		RegisterMessageHandlers();
	}



	// 如果数据已经准备好了, 这里尝试触发 OnRep 方法
	// 1. DS服务器: 已经通过 Initialize 设置好了数据, 需要触发 OnRep 方法以执行服务端逻辑
	// 2. 本地 Client(预测标记点): 已经通过 Initialize 设置好了数据, 需要触发 OnRep 方法以执行客户端逻辑
	// 3. Standalone: 已经通过 Initialize 设置好了数据, 需要触发 OnRep 方法以执行客户端逻辑
	// 4. 本地 Client(非预测标记点): 数据会通过复制到达, 这里大概率不会命中OnRep_MarkerData
	// 5. 远程 Client: 数据会通过复制到达, 这里大概率不会命中OnRep_MarkerData
	if (MarkerData.MarkerId != InvalidId())
	{
		OnRep_MarkerData();
	}
}

void ALyraWorldMarker::OnParentDestroyed(AActor* DestroyedActor)
{
	if (HasAuthority() || MarkerData.MarkerId == PredictedId())
	{
		// 如果依附的 Actor 被销毁了 标记点也销毁
		Destroy();
	}
}

void ALyraWorldMarker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		// 只在Client/Standalone下广播
		Broadcast_MarkerRemovedMessage();

		UnregisterMessageHandlers();
	}

	Super::EndPlay(EndPlayReason);
}

void ALyraWorldMarker::Initialize(FLyraWorldMarkerData InMarkerData)
{
	// 对于非复制的Actor执行环境来说, Initialize会早于BeginPlay调用, 对于复制的Actor来说, 不会发生Initialize调用

	MarkerData = InMarkerData;

	// 这里的调用在任何环境下都不会有效 只是占位
	OnRep_MarkerData();
}

void ALyraWorldMarker::OnRep_MarkerData()
{
	// 就当前的场景来说, Initialize 调用的时候BeginPlay还未调用, 因为上层通过SpawnActorDeferred生成Actor
	// 所以这里需要检查组件是否已经初始化完成, 以防出现问题
	if (!bComponentsInitialized)
	{
		// 组件还未初始化 完成初始化后会再次调用该函数
		return;
	}

	// DS 服务端这里不执行任何逻辑
	// Standalone/Client 会广播添加标记点的消息
	if (!IsNetMode(NM_DedicatedServer))
	{
		Broadcast_MarkerAddedMessage();
	}
}

void ALyraWorldMarker::Broadcast_MarkerAddedMessage()
{
	FOnMarkerAddParameters Parameters;
	Parameters.MarkerActor = this;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Marker_Add, Parameters);
		}
	}
}

void ALyraWorldMarker::Broadcast_MarkerRemovedMessage()
{
	FOnMarkerRemoveParameters Parameters;
	Parameters.MarkerId = MarkerData.MarkerId;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Marker_Remove, Parameters);
		}
	}
}

void ALyraWorldMarker::RegisterMessageHandlers()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (!MarkerDiscoverRequestListener.IsValid())
			{
				MarkerDiscoverRequestListener = MessageSubsystem->RegisterListener<FOnMarkerDiscoverParameters>(
					LyraGameplayTags::Gameplay_Message_Marker_Discover,
					this,
					&ThisClass::HandleMarkerDiscoverRequest);
			}
		}
	}
}

void ALyraWorldMarker::UnregisterMessageHandlers()
{
	if (MarkerDiscoverRequestListener.IsValid())
		MarkerDiscoverRequestListener.Unregister();
}

void ALyraWorldMarker::HandleMarkerDiscoverRequest(FGameplayTag Channel, const FOnMarkerDiscoverParameters& Parameters)
{
	Broadcast_MarkerAddedMessage();
}


void ALyraWorldMarker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALyraWorldMarker, MarkerData);
}
