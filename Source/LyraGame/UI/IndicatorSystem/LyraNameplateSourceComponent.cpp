// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraNameplateSourceComponent.h"
#include "NativeGameplayTags.h"
#include "LyraGameplayTags.h"
#include "Messages/LyraNotificationMessage_Nameplate.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraNameplateSourceComponent)


// Sets default values for this component's properties
ULyraNameplateSourceComponent::ULyraNameplateSourceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

ULyraNameplateSourceComponent* ULyraNameplateSourceComponent::GetComponent(const AActor* Actor)
{
	return (Actor ? Actor->FindComponentByClass<ULyraNameplateSourceComponent>() : nullptr);
}


// Called when the game starts
void ULyraNameplateSourceComponent::BeginPlay()
{
	Super::BeginPlay();
	RegisterMessageHandlers();
}

void ULyraNameplateSourceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterMessageHandlers();
	Super::EndPlay(EndPlayReason);
}

void ULyraNameplateSourceComponent::HandleNameplateDiscoverRequest(FGameplayTag Channel, const FOnNameplateDiscoverParameters& Parameters)
{
	BroadcastNameplateAddMessage();
}

void ULyraNameplateSourceComponent::RegisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			// 游戏中的每个 HeroCharacter 都会被注入一个 Nameplate Source Component, 不管是本地玩家还是远程玩家
			// 在 HeroCharacter 初始化时, Nameplate Source Component 会广播一个添加 Nameplate 的消息
			// Nameplate Manager Component 监听该消息, 并为对应的 Pawn 创建 Nameplate 指示器
			// 这样本地玩家和远程玩家的 Nameplate 都能被正确创建和管理

			// 主动广播
			BroadcastNameplateAddMessage();

			// 被动应答
			if (!NameplateDiscoverListenerHandle.IsValid())
			{
				MessageSubsystem->RegisterListener<FOnNameplateDiscoverParameters>(
					LyraGameplayTags::Gameplay_Message_Nameplate_Discover,
					this,
					&ThisClass::HandleNameplateDiscoverRequest
				);
			}
		}
	}
}

void ULyraNameplateSourceComponent::UnregisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			BroadcastNameplateRemoveMessage();

			if (NameplateDiscoverListenerHandle.IsValid())
			{
				NameplateDiscoverListenerHandle.Unregister();
			}
		}
	}
}

void ULyraNameplateSourceComponent::BroadcastNameplateAddMessage()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			// 广播添加 Nameplate 的消息
			FOnNameplateAddParameters AddParameters;
			AddParameters.Pawn = GetPawn<APawn>();

			MessageSubsystem->BroadcastMessage(
				LyraGameplayTags::Gameplay_Message_Nameplate_Add,
				AddParameters
				);
		}
	}
}

void ULyraNameplateSourceComponent::BroadcastNameplateRemoveMessage()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			// 广播移除 Nameplate 的消息
			FOnNameplateRemoveParameters Parameters;
			Parameters.Pawn = GetPawn<APawn>();

			MessageSubsystem->BroadcastMessage(
				LyraGameplayTags::Gameplay_Message_Nameplate_Remove,
				Parameters);
		}
	}
}


