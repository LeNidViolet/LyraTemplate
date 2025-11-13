// Fill out your copyright notice in the Description page of Project Settings.

#include "LyraMarkerManagerComponent.h"

#include "Blueprint/UserWidget.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/Session/LyraDSPlayerState.h"
#include "Interaction/IInteractableMarker.h"
#include "Interaction/LyraWorldMarker.h"
#include "LyraGameplayTags.h"
#include "LyraIndicatorManagerComponent.h"
#include "Messages/LyraNotificationMessage_Callout.h"
#include "Messages/LyraNotificationMessage_Marker.h"
#include "NativeGameplayTags.h"
#include "Player/LyraPlayerController.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraMarkerManagerComponent)

const FName ULyraMarkerManagerComponent::NAME_ActorFeatureName("MarkerManager");

// Sets default values for this component's properties
ULyraMarkerManagerComponent::ULyraMarkerManagerComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void ULyraMarkerManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听所有 Feature 的任何状态变化
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);
	ensure(TryToChangeInitState(LyraGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();

	ScanRate = ULyraGameData::Get().MarkerScanRate;
	ScanRadius = ULyraGameData::Get().MarkerScanRadius;

	ALyraPlayerController* PC = Cast<ALyraPlayerController>(GetOwner());
	if (PC)
	{
		// 对于 standalone, client 和 listen server 都是本地玩家
		if (PC->IsLocalPlayerController())
		{
			RegisterMessageHandlers();

			// 这里设置一个定时器, 用于不断检测当前视线是否对准了标记点, 以显示交互提示
			UWorld* World = GetWorld();
			if (World)
			{
				World->GetTimerManager().SetTimer(
					TimerHandle,
					this,
					&ThisClass::ToggleMarkerPromptVisbility,
					ScanRate,
					true);
			}

			return;
		}
	}

	DestroyComponent();
}

void ULyraMarkerManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ALyraPlayerController* PC = Cast<ALyraPlayerController>(GetOwner());
	if (PC)
	{
		if (PC->IsLocalPlayerController())
		{
			UnregisterMessageHandlers();

			UWorld* World = GetWorld();
			if (World)
			{
				World->GetTimerManager().ClearTimer(TimerHandle);
			}
		}
	}

	UnregisterInitStateFeature();
	Super::EndPlay(EndPlayReason);
}

void ULyraMarkerManagerComponent::OnRegister()
{
	Super::OnRegister();

	// 注册 Init State 特性
	RegisterInitStateFeature();
}

void ULyraMarkerManagerComponent::RegisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (!MarkerAddEventListener.IsValid())
			{
				MarkerAddEventListener = MessageSubsystem->RegisterListener<FOnMarkerAddParameters>(
						LyraGameplayTags::Gameplay_Message_Marker_Add,
						this,
						&ThisClass::HandleMarkerAddEvent);
			}
			if (!MarkerRemoveEventListener.IsValid())
			{
				MarkerRemoveEventListener = MessageSubsystem->RegisterListener<FOnMarkerRemoveParameters>(
						LyraGameplayTags::Gameplay_Message_Marker_Remove,
						this,
						&ThisClass::HandleMarkerRemoveEvent);
			}
		}
	}
}

void ULyraMarkerManagerComponent::UnregisterMessageHandlers()
{
	if (MarkerAddEventListener.IsValid())
	{
		MarkerAddEventListener.Unregister();
	}
	if (MarkerRemoveEventListener.IsValid())
	{
		MarkerRemoveEventListener.Unregister();
	}
}







ULyraMarkerManagerComponent* ULyraMarkerManagerComponent::GetComponent(AController* Controller)
{
	if (Controller)
	{
		return Controller->FindComponentByClass<ULyraMarkerManagerComponent>();
	}

	return nullptr;
}


TArray<FLyraMarkerInstance*> ULyraMarkerManagerComponent::GetMarkerInstancesForOwnerPlayer(
	APlayerState* PlayerState,  bool bWithInvisible) const
{
	TArray<FLyraMarkerInstance*> Results;

	for (const TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		TWeakObjectPtr<ALyraWorldMarker> MarkerActor = MarkerInstance->MarkerActor;
		if (MarkerActor.IsValid() && MarkerActor->GetOwnerPlayer() == PlayerState)
		{
			TWeakObjectPtr<UIndicatorDescriptor> IndicatorDescriptor = MarkerInstance->IndicatorDescriptor;
			if (IndicatorDescriptor.IsValid())
			{
				if (!IndicatorDescriptor->GetIsVisible())
				{
					if (bWithInvisible) Results.Add(MarkerInstance.Get());
				}
				else
				{
					Results.Add(MarkerInstance.Get());
				}
			}
		}
	}

	return Results;
}


TArray<ALyraWorldMarker*> ULyraMarkerManagerComponent::GetMarkerActorsForOwnerPlayer(
	APlayerState* PlayerState, bool bWithInvisible) const
{
	TArray<ALyraWorldMarker*> Results;

	for (const TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		TWeakObjectPtr<ALyraWorldMarker> MarkerActor = MarkerInstance->MarkerActor;
		if (MarkerActor.IsValid() && MarkerActor->GetOwnerPlayer() == PlayerState)
		{
			TWeakObjectPtr<UIndicatorDescriptor> IndicatorDescriptor = MarkerInstance->IndicatorDescriptor;
			if (IndicatorDescriptor.IsValid())
			{
				if (!IndicatorDescriptor->GetIsVisible())
				{
					if (bWithInvisible) Results.Add(MarkerActor.Get());
				}
				else
				{
					Results.Add(MarkerActor.Get());
				}
			}
		}
	}

	return Results;
}

ALyraWorldMarker* ULyraMarkerManagerComponent::GetMarkerActorForIndicatorDescriptor(UIndicatorDescriptor* IndicatorDescriptor) const
{
	for (const TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		if (MarkerInstance->IndicatorDescriptor.IsValid() && MarkerInstance->IndicatorDescriptor == IndicatorDescriptor)
		{
			TWeakObjectPtr<ALyraWorldMarker> MarkerActor = MarkerInstance->MarkerActor;
			if (MarkerActor.IsValid())
			{
				return MarkerActor.Get();
			}
			break;
		}
	}

	return nullptr;
}

int32 ULyraMarkerManagerComponent::GetDistanceToLocation(UIndicatorDescriptor* IndicatorDescriptor, const FVector& TargetLocation) const
{
	int32 Distance = -1;

	for (const TSharedPtr<FLyraMarkerInstance>& MarkerInstance : MarkerList)
	{
		if (MarkerInstance->IndicatorDescriptor.IsValid() && MarkerInstance->IndicatorDescriptor == IndicatorDescriptor)
		{
			TWeakObjectPtr<ALyraWorldMarker> MarkerActor = MarkerInstance->MarkerActor;
			if (MarkerActor.IsValid())
			{
				Distance = FMath::RoundToInt(FVector::Dist(TargetLocation, MarkerActor->GetTransform().GetLocation()) / 100.0f);
				break;
			}
		}
	}

	return Distance;
}

void ULyraMarkerManagerComponent::HandleMarkerAddEvent(FGameplayTag Channel, const FOnMarkerAddParameters& Parameters)
{
	FGameplayTag CurrentState = GetInitState();
	if (CurrentState != LyraGameplayTags::InitState_GameplayReady)
	{
		// 尚未初始化完成, 不处理消息
		UE_LOG(LogTemp, Warning, TEXT("ULyraMarkerManagerComponent Init State not ready. Current State: %s"), *CurrentState.ToString());
		return;
	}



	ALyraWorldMarker* MarkerActor = Parameters.MarkerActor.Get();
	if (!IsValid(MarkerActor)) return;

	if (IsMarkerActorExist(MarkerActor))
	{
		return;
	}

	// UE_LOG(LogTemp, Error, TEXT("HandleAddMarkerEvent Player %s  Id %d"),
	// *MarkerActor->GetOwnerPlayer()->GetName(), MarkerActor->GetMarkerId());

	AController* LocalController = Cast<AController>(GetOwner());
	if (!LocalController) return;

	APlayerState* LocalPlayerState = LocalController->GetPlayerState<APlayerState>();
	if (!LocalPlayerState) return;

	ULyraIndicatorManagerComponent* IndicatorManager = LocalController->GetComponentByClass<ULyraIndicatorManagerComponent>();
	if (!IndicatorManager) return;

	// 从全局数据表中得到需要使用的 IndicatorDescriptor 类
	TSubclassOf<UIndicatorDescriptor> IndicatorDescriptorClass = ULyraGameData::Get().GetIndicatorClassForMarkerType(MarkerActor->GetMarkerType());
	if (!IndicatorDescriptorClass) return;

	// The rest of the settings are coming from the **CDO**.
	UIndicatorDescriptor* Descriptor = NewObject<UIndicatorDescriptor>(this, IndicatorDescriptorClass);
	Descriptor->SetDataObject(MarkerActor);
	Descriptor->SetSceneComponent(MarkerActor->GetRootComponent());

	// 根据不同子类进行差异化布局调整
	Descriptor->LayoutIndicator(MarkerActor);

	if (MarkerActor->IsPredictedMarker())
	{
		// 预测标记点
		if (LocalPlayerState)
		{
			// 预测标记点一定是属于本地玩家的
			ensure(MarkerActor->GetOwnerPlayer() == LocalPlayerState);
		}
	}
	else
	{
		// 如果是本地玩家的非预测标记点, 通知本地玩家去删除可能存在的预测标记点
		if (MarkerActor->GetOwnerPlayer() == LocalPlayerState)
		{
			ALyraDSPlayerState* DSPlayerState = Cast<ALyraDSPlayerState>(LocalPlayerState);
			DSPlayerState->RemoveWorldMarkerFromCache(ALyraWorldMarker::PredictedId());
		}
	}

	TSharedPtr<FLyraMarkerInstance> MarkerInstance = MakeShared<FLyraMarkerInstance>();
	MarkerInstance->IndicatorDescriptor = Descriptor;
	MarkerInstance->MarkerActor = MarkerActor;
	MarkerList.Add(MarkerInstance);

	IndicatorManager->AddIndicator(Descriptor);
}

void ULyraMarkerManagerComponent::HandleMarkerRemoveEvent(FGameplayTag Channel, const FOnMarkerRemoveParameters& Parameters)
{
	int32 FoundIndex = MarkerList.IndexOfByPredicate([Parameters](const TSharedPtr<FLyraMarkerInstance>& MarkerInstance)
	{
		return MarkerInstance->MarkerActor.IsValid() && MarkerInstance->MarkerActor->GetMarkerId() == Parameters.MarkerId;
	});

	if (MarkerList.IsValidIndex(FoundIndex))
	{
		TSharedPtr<FLyraMarkerInstance>& MarkerInstance = MarkerList[FoundIndex];
		if (MarkerInstance->IndicatorDescriptor.IsValid())
		{
			MarkerInstance->IndicatorDescriptor->UnregisterIndicator();
		}

		MarkerInstance->MarkerActor = nullptr;
		MarkerInstance->IndicatorDescriptor = nullptr;

		MarkerList.RemoveAt(FoundIndex);
	}
}

bool ULyraMarkerManagerComponent::IsMarkerActorExist(ALyraWorldMarker* MarkerActor) const
{
	int32 FoundIndex = MarkerList.IndexOfByPredicate([MarkerActor](const TSharedPtr<FLyraMarkerInstance>& MarkerInstance)
	{
		return MarkerInstance->MarkerActor.IsValid() &&
			MarkerInstance->MarkerActor == MarkerActor;
	});

	return MarkerList.IsValidIndex(FoundIndex);
}

void ULyraMarkerManagerComponent::ToggleMarkerPromptVisbility()
{
	ALyraPlayerController* PlayerController = GetController<ALyraPlayerController>();
	if (PlayerController)
	{
		UIndicatorDescriptor* ClosestIndicatorDescriptor = nullptr;
		ALyraWorldMarker* ClosestWorldMarker = nullptr;
		float ClosestDistance = ScanRadius;

		// 取得屏幕中心点
		int32 ViewportX, ViewportY;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

		// 扫描属于本地玩家的所有标记点 找到距离屏幕中心最近的那个
		TArray<FLyraMarkerInstance*> MarkerInstances = GetMarkerInstancesForOwnerPlayer(PlayerController->GetPlayerState<APlayerState>(), true);
		for (FLyraMarkerInstance* MarkerInstance : MarkerInstances)
		{
			ALyraWorldMarker* MarkerActor = MarkerInstance->MarkerActor.Get();
			UIndicatorDescriptor* IndicatorDescriptor = MarkerInstance->IndicatorDescriptor.Get();

			if (!MarkerActor || !IndicatorDescriptor) continue;


			// 得到标记点中心投影在屏幕上的位置
			FVector2D MarkerScreenPos;
			bool bIsOnScreen = PlayerController->ProjectWorldLocationToScreen(
				MarkerActor->GetActorLocation(),
				MarkerScreenPos,
				true);
			if (!bIsOnScreen) continue;

			// 探测 Widget 大小, 用来更准确的定位
			TWeakObjectPtr<UUserWidget> Widget = IndicatorDescriptor->IndicatorWidget;
			if (!Widget.IsValid()) continue;
			if (!Widget->GetClass()->ImplementsInterface(UInteractableMarker::StaticClass())) continue;
			FVector2D WidgetSize = IInteractableMarker::Execute_K2_OnGetWidgetPixelSize(Widget.Get());
			if (WidgetSize.X == 0 || WidgetSize.Y == 0) continue;

			FVector2D HalfSize = WidgetSize / 2.0f;

			// 定义 Widget 在屏幕上的矩形 (FBox2D)
			FVector2D MinBounds = MarkerScreenPos - HalfSize;
			FVector2D MaxBounds = MarkerScreenPos + HalfSize;

			// 计算屏幕中心点 (ScreenCenter) 到这个矩形的最短距离
			FVector2D ClosestPointInBox;

			// Clamp X 轴：将 ScreenCenter.X 限制在 [MinBounds.X, MaxBounds.X] 之间
			ClosestPointInBox.X = FMath::Clamp(ScreenCenter.X, MinBounds.X, MaxBounds.X);
			// Clamp Y 轴：将 ScreenCenter.Y 限制在 [MinBounds.Y, MaxBounds.Y] 之间
			ClosestPointInBox.Y = FMath::Clamp(ScreenCenter.Y, MinBounds.Y, MaxBounds.Y);

			// 3. 计算实际的像素距离
			const float PixelDistance = FVector2D::Distance(ScreenCenter, ClosestPointInBox);
			// const float PixelDistance = FVector2D::Distance(MarkerScreenPos, ScreenCenter);

			// 选择最接近屏幕中心的标记点
			if (PixelDistance <= ClosestDistance)
			{
				ClosestDistance = PixelDistance;
				ClosestIndicatorDescriptor = IndicatorDescriptor;
				ClosestWorldMarker = MarkerActor;
			}
		}

		bool bNewObject = (ClosestIndicatorDescriptor != nullptr) && (LastPromptIndicatorDescriptor != ClosestIndicatorDescriptor);
		bool bShouldShowPrompt = ClosestIndicatorDescriptor != nullptr;

		// 检查状态是否真的需要改变: 是否找到了新目标，或者可见性需要切换
		if ((bShouldShowPrompt != bLastPromptVisible) || bNewObject)
		{
			// 如果找到了一个新对象, 或者目标存在, 但上一个不存在 从 A 切换到 B
			if (bNewObject)
			{
				// 1. 隐藏旧的提示
				if (LastPromptMarkerActor.IsValid())
					ShowOrHideCalloutPrompt(LastPromptMarkerActor.Get(), false);

				// 2. 显示新的提示
				ShowOrHideCalloutPrompt(ClosestWorldMarker, true);

				// 3. 更新所有状态追踪变量
				LastPromptIndicatorDescriptor = ClosestIndicatorDescriptor;
				LastPromptMarkerActor = ClosestWorldMarker;
				bLastPromptVisible = true;
			}
			// 否则，只是可见性状态的简单切换 (例如，从显示到隐藏，或反之)
			else
			{
				if (!bShouldShowPrompt) // 状态变化为“隐藏”
				{
					// 使用 LastPromptIndicatorDescriptor 来隐藏上一次显示的提示
					if (LastPromptIndicatorDescriptor.IsValid())
					{
						ShowOrHideCalloutPrompt(LastPromptMarkerActor.Get(), false);
						// 清空 LastPromptIndicatorDescriptor 以防再次触发
						LastPromptIndicatorDescriptor = nullptr;
						LastPromptMarkerActor = nullptr;
					}
				}
				else // 状态变化为“显示” (理论上不应该发生，因为 NewObject == false)
				{
					// 如果走到这里，说明是 LastPromptIndicatorDescriptor != nullptr 且
					// ClosestIndicatorDescriptor != nullptr 且它们相等， 并且上一个 Tick
					// 隐藏了，这个 Tick 应该显示，这种路径非常少见
					ShowOrHideCalloutPrompt(ClosestWorldMarker, true);
				}

				bLastPromptVisible = bShouldShowPrompt;
			}
		}
	}
}

void ULyraMarkerManagerComponent::ShowOrHideCalloutPrompt(
	ALyraWorldMarker* MarkerActor, bool bShow)
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			FOnCalloutDisplayParameters Parameters;
			Parameters.bShow = bShow;
			Parameters.Actor = MarkerActor;
			Parameters.ActorScreenOffset = FVector2D::ZeroVector;
			if (MarkerActor)
				Parameters.CalloutWidgetClass = ULyraGameData::Get().GetCalloutClassForMarkerType(MarkerActor->GetMarkerType());
			Parameters.CalloutWidgetScreenOffset = FVector2D::ZeroVector;

			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Callout_Display, Parameters);
		}
	}
}

bool ULyraMarkerManagerComponent::IsAimingAtMarker() const
{
	return bLastPromptVisible;
}

ALyraWorldMarker* ULyraMarkerManagerComponent::GetAimingMarkerActor() const
{
	if (IsAimingAtMarker() && LastPromptMarkerActor.IsValid())
	{
		return LastPromptMarkerActor.Get();
	}
	return nullptr;
}

void ULyraMarkerManagerComponent::BroadcastDiscoverMessage()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			FOnMarkerDiscoverParameters Parameters;
			MessageSubsystem->BroadcastMessage(LyraGameplayTags::Gameplay_Message_Marker_Discover, Parameters);
		}
	}
}

bool ULyraMarkerManagerComponent::CanChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState) const
{
	if (DesiredState == LyraGameplayTags::InitState_GameplayReady)
	{
		// 等待 Indicator Manager 组件初始化完成
		return Manager->HasFeatureReachedInitState(
			GetOwner(), ULyraIndicatorManagerComponent::NAME_ActorFeatureName,
			LyraGameplayTags::InitState_GameplayReady);
	}
	return true;
}

void ULyraMarkerManagerComponent::HandleChangeInitState(
	UGameFrameworkComponentManager* Manager,
	FGameplayTag CurrentState,
	FGameplayTag DesiredState)
{
	// HandleChangeInitState 是在状态发生变化时触发调用,
	// 所以下面的代码判断出的是最后的状态
	if (DesiredState == LyraGameplayTags::InitState_GameplayReady)
	{
		// 等到 Indicator Manager 组件初始化完成后, 广播发现标记点消息
		// 延迟一帧广播 确保状态已完全更新 且避免调用栈过深
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimerForNextTick([WeakThis = TWeakObjectPtr<ULyraMarkerManagerComponent>(this)]()
			{
				if (ULyraMarkerManagerComponent* StrongThis = WeakThis.Get())
				{
					StrongThis->BroadcastDiscoverMessage();
				}
			});
		}
	}
}

void ULyraMarkerManagerComponent::OnActorInitStateChanged(
	const FActorInitStateChangedParams& Params)
{
	// 我们只对 Actor 的 Indicator Manager 组件的状态变化感兴趣
	if (Params.FeatureName == ULyraIndicatorManagerComponent::NAME_ActorFeatureName)
	{
		CheckDefaultInitialization();
	}
}

void ULyraMarkerManagerComponent::CheckDefaultInitialization()
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
