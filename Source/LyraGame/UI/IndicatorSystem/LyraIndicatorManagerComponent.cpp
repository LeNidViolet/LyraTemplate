// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraIndicatorManagerComponent.h"

#include "IndicatorDescriptor.h"
#include "LyraGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraIndicatorManagerComponent)

const FName ULyraIndicatorManagerComponent::NAME_ActorFeatureName("IndicatorManager");

ULyraIndicatorManagerComponent::ULyraIndicatorManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoRegister = true;
	bAutoActivate = true;
}

/*static*/ ULyraIndicatorManagerComponent* ULyraIndicatorManagerComponent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<ULyraIndicatorManagerComponent>();
	}

	return nullptr;
}

void ULyraIndicatorManagerComponent::AddIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	IndicatorDescriptor->SetIndicatorManagerComponent(this);
	OnIndicatorAdded.Broadcast(IndicatorDescriptor);
	Indicators.Add(IndicatorDescriptor);
}

void ULyraIndicatorManagerComponent::RemoveIndicator(UIndicatorDescriptor* IndicatorDescriptor)
{
	if (IndicatorDescriptor)
	{
		ensure(IndicatorDescriptor->GetIndicatorManagerComponent() == this);
	
		OnIndicatorRemoved.Broadcast(IndicatorDescriptor);
		Indicators.Remove(IndicatorDescriptor);
	}
}

void ULyraIndicatorManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听所有 Feature 的任何状态变化
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	ensure(TryToChangeInitState(LyraGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void ULyraIndicatorManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void ULyraIndicatorManagerComponent::OnRegister()
{
	Super::OnRegister();

	// 注册 Init State 特性
	RegisterInitStateFeature();
}


bool ULyraIndicatorManagerComponent::CanChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	// Indicator Manager 不需要等待其他组件
	// Indicator Manager 是纯客户端组件, 不涉及服务器同步, 这里只是用最容易得方式来完成状态转换
	// 在BeginPlay的时候开始推进状态, 所以能检查到 Indicator Manager 的状态说明已经初始化完毕了
	return true;
}

void ULyraIndicatorManagerComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	// Indicator Manager 不需要做任何特殊处理
}

void ULyraIndicatorManagerComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// Indicator Manager 不需要对其他组件的状态变化做出反应
}

void ULyraIndicatorManagerComponent::CheckDefaultInitialization()
{
	// 固定队列
	static const TArray<FGameplayTag> StateChain = {
		LyraGameplayTags::InitState_Spawned,
		LyraGameplayTags::InitState_DataAvailable,
		LyraGameplayTags::InitState_DataInitialized,
		LyraGameplayTags::InitState_GameplayReady
	};

	// 尝试按照默认队列进行状态转换
	ContinueInitStateChain(StateChain);
}
