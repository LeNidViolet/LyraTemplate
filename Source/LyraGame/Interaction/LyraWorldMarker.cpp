// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraWorldMarker.h"

#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraLogChannels.h"
#include "LyraGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameModes/Session/LyraDSPlayerState.h"
#include "Messages/LyraNotificationMessage.h"
#include "Messages/LyraNotificationMessage_Marker.h"


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
		UE_LOG(LogLyra, Error, TEXT("SpawnLyraWorldMarkerActor Failed to get World from context object!"));
		return nullptr;
	}

	// TODO 做位置检测 TargetActor Location 的合法性
	// TODO 检测该玩家已有标记点数量限制

	// 调用者需要确保传入了正确的PlayerState
	ALyraDSPlayerState* DSPlayerState = Cast<ALyraDSPlayerState>(OwnerPlayer);
	check(DSPlayerState);
	if (bAsPredictedMarker)
	{
		check(!DSPlayerState->HasAuthority());
		DSPlayerState->RemoveWorldMarkerFromCache(ALyraWorldMarker::PredictedId());
	}
	else
	{
		check(DSPlayerState->HasAuthority());
		DSPlayerState->RemoveWorldMarkerFromCache(MarkerType);
	}


	// 生成 Actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = TargetActor;
	FTransform SpawnTransform(FRotator::ZeroRotator, TargetLocation);

	ALyraWorldMarker* NewMarker = World->SpawnActorDeferred<ALyraWorldMarker>(
		MarkerClass,
		SpawnTransform,
		TargetActor,
		nullptr
	);
	if (!NewMarker)
	{
		return nullptr;
	}
	FLinearColor PlayerColor = DSPlayerState ? DSPlayerState->PlayerColor : FLinearColor::White;
	// 在 BeginPlay 之前设置参数
	NewMarker->Initialize(
		OwnerPlayer,
		MarkerType,
		PlayerColor,
		bAsPredictedMarker ? ALyraWorldMarker::PredictedId() : NextMarkerId++,
		bAsPredictedMarker);

	// 保存到PlayerState
	DSPlayerState->AddWorldMarkerToCache(NewMarker);

	if (MarkerType != ELyraWorldMarkerType::Waypoint)
	{
		// 如果不是世界标记点 就附加到目标 Actor 上, 并且是完全贴合
		// SnapToTargetIncludingScale 规则非常适合 UI 锚定
		NewMarker->AttachToActor(TargetActor, FAttachmentTransformRules::SnapToTargetIncludingScale);
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
	SphereComp->SetSphereRadius(50.f);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 调试打开显示
	// SphereComp->SetHiddenInGame(false);
	SetRootComponent(SphereComp);
}

void ALyraWorldMarker::Initialize(
	APlayerState* InOwnerPlayer, ELyraWorldMarkerType InMarkerType,
	const FLinearColor& InColor, int32 InMarkerId, bool InPredictedMarker)
{
	/* 这些都是复制属性, 只在服务端初始化; 或者客户端预测时创建本地实例时进行初始化 */

	OwnerPlayer = InOwnerPlayer;
	MarkerType = InMarkerType;
	Color = InColor;
	MarkerId = InPredictedMarker ? ALyraWorldMarker::PredictedId() : InMarkerId;
	bPredictedMarker = InPredictedMarker;
}


void ALyraWorldMarker::BeginPlay()
{
	Super::BeginPlay();

	if (!IsNetMode(NM_DedicatedServer))
	{
		// 目前只在Client/Standalone下广播
		Broadcast_MarkerAddedMessage();
	}
}

void ALyraWorldMarker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!IsNetMode(NM_DedicatedServer))
	{
		// 目前只在Client/Standalone下广播
		Broadcast_MarkerRemovedMessage();
	}

	Super::EndPlay(EndPlayReason);
}

void ALyraWorldMarker::Broadcast_MarkerAddedMessage()
{
	FOnAddMarkerParameters Parameters;
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
	FOnRemoveMarkerParameters Parameters;
	Parameters.MarkerId = MarkerId;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Marker_Remove, Parameters);
		}
	}
}


void ALyraWorldMarker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALyraWorldMarker, OwnerPlayer);
	DOREPLIFETIME(ALyraWorldMarker, MarkerType);
	DOREPLIFETIME(ALyraWorldMarker, Color);
	DOREPLIFETIME(ALyraWorldMarker, MarkerId);
	DOREPLIFETIME(ALyraWorldMarker, bPredictedMarker);
	DOREPLIFETIME(ALyraWorldMarker, SphereComp);
}
