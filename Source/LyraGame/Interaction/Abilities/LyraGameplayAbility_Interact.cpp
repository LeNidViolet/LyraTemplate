// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "LyraGameplayTags.h"
#include "Interaction/IInteractableTarget.h"
#include "Interaction/InteractionStatics.h"
#include "Interaction/Tasks/AbilityTask_GrantNearbyInteraction.h"
#include "Interaction/Tasks/AbilityTask_WaitForInteractableTargets_SingleLineTrace.h"
#include "Player/LyraPlayerController.h"
#include "System/LyraGameData.h"
#include "UI/IndicatorSystem/IndicatorDescriptor.h"
#include "UI/IndicatorSystem/LyraIndicatorManagerComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameplayAbility_Interact)

class UUserWidget;


ULyraGameplayAbility_Interact::ULyraGameplayAbility_Interact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void ULyraGameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);


	// UAbilityTask_GrantNearbyInteraction 的作用是扫描附近的可交互对象，并将它们身上的能力赋予角色
	// 这个动作只在服务端进行, 因为GAS会将技能同步给客户端
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UAbilityTask_GrantNearbyInteraction* Task = UAbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractors(
			this,
			InteractScanRange,
			InteractScanRate);
		Task->ReadyForActivation();
	}

	// 两端都需要监听输入按下事件，以便触发交互
	CreateWaitInputPressTask();



	FInteractionQuery InteractionQuery;
	InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
	InteractionQuery.RequestingController = GetLyraPlayerControllerFromActorInfo();

	// 等待更新可交互对象视觉反馈 这里在客户端服务端都运行了, 感觉可以优化为只在客户端运行, 之后通过TargetData同步给服务端
	UAbilityTask_WaitForInteractableTargets_SingleLineTrace* WaitForInteractablesTask =
		UAbilityTask_WaitForInteractableTargets_SingleLineTrace::WaitForInteractableTargets_SingleLineTrace(
			this,
			InteractionQuery,
			FCollisionProfileName(InteractTraceProfileName),
			MakeTargetLocationInfoFromOwnerActor(),
			InteractScanRange,	// 这里使用了与交互能力相同的扫描范围 也可以使用单独的配置
			InteractScanRate);
	WaitForInteractablesTask->InteractableObjectsChanged.AddUniqueDynamic(this, &ULyraGameplayAbility_Interact::UpdateInteractions);
	WaitForInteractablesTask->ReadyForActivation();
}

void ULyraGameplayAbility_Interact::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	TriggerTag = ULyraGameData::Get().InteractTriggerTag;
	InteractTraceProfileName = ULyraGameData::Get().InteractTraceProfileName;
	InteractScanRate = ULyraGameData::Get().InteractScanRate;
	InteractScanRange = ULyraGameData::Get().InteractScanRange;
	IndicatorClass = ULyraGameData::Get().CollectableIndicatorClass;
}

void ULyraGameplayAbility_Interact::UpdateInteractions(const TArray<FInteractionOption>& InteractiveOptions)
{
	if (ALyraPlayerController* PC = GetLyraPlayerControllerFromActorInfo())
	{
		if (ULyraIndicatorManagerComponent* IndicatorManager = ULyraIndicatorManagerComponent::GetComponent(PC))
		{
			for (UIndicatorDescriptor* Indicator : Indicators)
			{
				IndicatorManager->RemoveIndicator(Indicator);
			}
			Indicators.Reset();

			for (const FInteractionOption& InteractionOption : InteractiveOptions)
			{
				AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

				UIndicatorDescriptor* Indicator = NewObject<UIndicatorDescriptor>(this, IndicatorClass);
				Indicator->SetDataObject(InteractableTargetActor);
				Indicator->SetSceneComponent(InteractableTargetActor->GetRootComponent());

				if (!InteractionOption.InteractionWidgetClass.IsNull())
				{
					// 使用 InteractionOption 中指定的 Widget Class
					Indicator->SetIndicatorClass(InteractionOption.InteractionWidgetClass);
				}

				IndicatorManager->AddIndicator(Indicator);
				Indicators.Add(Indicator);
			}
		}
		else
		{
			//TODO This should probably be a noisy warning.  Why are we updating interactions on a PC that can never do anything with them?
		}
	}

	CurrentOptions = InteractiveOptions;
}

void ULyraGameplayAbility_Interact::TriggerInteraction()
{
	if (CurrentOptions.Num() == 0)
	{
		return;
	}

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem)
	{
		const FInteractionOption& InteractionOption = CurrentOptions[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableTargetActor = UInteractionStatics::GetActorFromInteractableTarget(InteractionOption.InteractableTarget);

		// Allow the target to customize the event data we're about to pass in, in case the ability needs custom data
		// that only the actor knows.
		FGameplayEventData Payload;
		Payload.EventTag = TriggerTag;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableTargetActor;

		// 标记为主动拾取
		Payload.InstigatorTags.AddTag(LyraGameplayTags::Ability_Interaction_Pickup_Active);

		// If needed we allow the interactable target to manipulate the event data so that for example, a button on the wall
		// may want to specify a door actor to execute the ability on, so it might choose to override Target to be the
		// door actor.
		InteractionOption.InteractableTarget->CustomizeInteractionEventData(TriggerTag, Payload);

		// Grab the target actor off the payload we're going to use it as the 'avatar' for the interaction, and the
		// source InteractableTarget actor as the owner actor.
		AActor* TargetActor = const_cast<AActor*>(ToRawPtr(Payload.Target));

		// The actor info needed for the interaction.
		FGameplayAbilityActorInfo ActorInfo;
		ActorInfo.InitFromActor(InteractableTargetActor, TargetActor, InteractionOption.TargetAbilitySystem);

		// 因为 WaitInputPress 会在两端运行, 所以这里触发技能也是两端分开的, 不涉及到数据传输?
		const bool bSuccess = InteractionOption.TargetAbilitySystem->TriggerAbilityFromGameplayEvent(
			InteractionOption.TargetInteractionAbilityHandle,
			&ActorInfo,
			TriggerTag,
			&Payload,
			*InteractionOption.TargetAbilitySystem
		);
	}
}

void ULyraGameplayAbility_Interact::CreateWaitInputPressTask()
{
	UAbilityTask_WaitInputPress* WaitInputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(this, false);
	WaitInputPressTask->OnPress.AddUniqueDynamic(this, &ULyraGameplayAbility_Interact::OnInputPressedCallback);
	WaitInputPressTask->ReadyForActivation();
}

void ULyraGameplayAbility_Interact::OnInputPressedCallback(float TimeWaited)
{
	TriggerInteraction();
	CreateWaitInputPressTask();
}

