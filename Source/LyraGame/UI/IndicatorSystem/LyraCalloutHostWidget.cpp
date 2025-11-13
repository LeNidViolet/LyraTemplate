#include "LyraCalloutHostWidget.h"

#include "LyraGameplayTags.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/VerticalBox.h"
#include "Interaction/IInteractableCallout.h"
#include "Messages/LyraNotificationMessage_Callout.h"
#include "Rendering/DrawElements.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraCalloutHostWidget)


ULyraCalloutHostWidget::ULyraCalloutHostWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraCalloutHostWidget::SetTargetActor(AActor* InActor)
{
	TargetActor = InActor;
}

void ULyraCalloutHostWidget::SetTargetActorWorldOffset(FVector2D InOffset)
{
	if (InOffset.IsZero())
	{
		ActorScreenOffset = ULyraGameData::Get().CalloutTargetActorScreenOffset;
	}
	else
	{
		ActorScreenOffset = InOffset;
	}
}

void ULyraCalloutHostWidget::SetCalloutWidgetClass(TSubclassOf<UUserWidget> InCalloutWidgetClass)
{
	if (CalloutWidgetClass != InCalloutWidgetClass)
	{
		CalloutWidgetClass = InCalloutWidgetClass;

		if (CalloutWidget)
		{
			CalloutWidget->RemoveFromParent();
			CalloutWidget = nullptr;
		}

		if (VerticalBox)
		{
			VerticalBox->ClearChildren();
		}

		if (CalloutWidgetClass && VerticalBox)
		{
			CalloutWidget = CreateWidget<UUserWidget>(VerticalBox, CalloutWidgetClass);
			if (CalloutWidget)
			{
				VerticalBox->AddChild(CalloutWidget);
			}
		}
	}
}

void ULyraCalloutHostWidget::SetCalloutWidgetScreenOffset(FVector2D InOffset)
{
	if (InOffset.IsZero())
	{
		CalloutScreenOffset = ULyraGameData::Get().CalloutWidgetScreenOffset;
	}
	else
	{
		CalloutScreenOffset = InOffset;
	}
}

void ULyraCalloutHostWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 默认不显示
	SetVisibility(ESlateVisibility::Hidden);

	LineThickness = ULyraGameData::Get().CalloutWidgetLineThickness;
	LineColor = ULyraGameData::Get().CalloutWidgetLineColor;

	// 检查自动绑定是否成功
	if (VerticalBox)
	{
		VerticalBox->ClearChildren();

		if (CalloutWidgetClass)
		{
			CalloutWidget = CreateWidget<UUserWidget>(this, CalloutWidgetClass);
			if (CalloutWidget)
			{
				VerticalBox->AddChild(CalloutWidget);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VerticalBox pointer is null! Check UMG variable name matching."));
	}

	RegisterMessageHandlers();
}

void ULyraCalloutHostWidget::NativeDestruct()
{
	UnregisterMessageHandlers();
	Super::NativeDestruct();
}

void ULyraCalloutHostWidget::NativeTick(const FGeometry& MyGeometry,
                                    float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 每一帧都更新位置
	bIsTargetOnScreen = GetTargetScreenPosition(TargetActorPixelPos);
}

int32 ULyraCalloutHostWidget::NativePaint(const FPaintArgs& Args,
                                      const FGeometry& AllottedGeometry,
                                      const FSlateRect& MyCullingRect,
                                      FSlateWindowElementList& OutDrawElements,
                                      int32 LayerId,
                                      const FWidgetStyle& InWidgetStyle,
                                      bool bParentEnabled) const
{
	int32 MaxLayerId = Super::NativePaint(
		Args,
		AllottedGeometry,
		MyCullingRect,
		OutDrawElements,
		LayerId,
		InWidgetStyle,
		bParentEnabled);

	if (bIsTargetOnScreen && CalloutWidget)
	{
		// 计算 TargetActor 的位置

		// TargetActorScreenPos 得到的是像素空间位置, 这里需要转换为绝对屏幕空间位置
		FVector2D TargetActorAbsolutePos;
		USlateBlueprintLibrary::ScreenToWidgetAbsolute(
			this,
			TargetActorPixelPos + ActorScreenOffset,
			TargetActorAbsolutePos,
			true);
		FVector2D TargetActorPosLocal = USlateBlueprintLibrary::AbsoluteToLocal(
			AllottedGeometry,
			TargetActorAbsolutePos);


		// 计算 CalloutWidget 的位置

		// 1. 获取 Geometry
		const FGeometry& CalloutWidgetGeo = CalloutWidget->GetPaintSpaceGeometry();

		// 2. 计算 CalloutWidget 左下角的 Absolute 坐标
		FVector2D CalloutWidgetBottomLeftLocal = FVector2D(0.0f, CalloutWidgetGeo.GetLocalSize().Y);
		FVector2D CalloutWidgetBottomLeftAbsolute = CalloutWidgetGeo.LocalToAbsolute(CalloutWidgetBottomLeftLocal);

		// 3. 转到 CalloutWidget 的 Local Space
		FVector2D CalloutWidgetPosLocal = AllottedGeometry.AbsoluteToLocal(CalloutWidgetBottomLeftAbsolute);
		FVector2D CalloutWidgetPosLocal2 = FVector2D::ZeroVector;

		if (!CalloutScreenOffset.IsZero())
		{
			// --- 加上像素偏移 ---

			// 4. 获取 Local 到 Pixel 的缩放比例 (Scale)
			// AllottedGeometry.Scale 包含了 DPI Scale 和父级 Scale
			float LocalToPixelScale = AllottedGeometry.GetAccumulatedLayoutTransform().GetScale();

			// 5. 把像素偏移 (PixelOffset) 转成 Local Slate Units
			// 公式: LocalUnits = Pixels / Scale
			FVector2D LocalOffset = CalloutScreenOffset / LocalToPixelScale;

			// 6. 加上偏移
			CalloutWidgetPosLocal += LocalOffset;
		}
		else
		{
			// 计算 右下角的 Absolute 坐标
			FVector2D CalloutWidgetBottomRightLocal = FVector2D(CalloutWidgetGeo.GetLocalSize().X, CalloutWidgetGeo.GetLocalSize().Y);
			FVector2D CalloutWidgetBottomRightAbsolute = CalloutWidgetGeo.LocalToAbsolute(CalloutWidgetBottomRightLocal);
			CalloutWidgetPosLocal2 = AllottedGeometry.AbsoluteToLocal(CalloutWidgetBottomRightAbsolute);
		}





		// Draw Line
		TArray<FVector2f> Points;
		Points.Add(FVector2f(TargetActorPosLocal));
		Points.Add(FVector2f(CalloutWidgetPosLocal));
		// 如果没有指定偏移, 则连到右下角
		// if (!CalloutWidgetPosLocal2.IsZero())
		// {
		// 	Points.Add(FVector2f(CalloutWidgetPosLocal2));
		// }


		// FSlateDrawElement::MakeLines(
		// 	OutDrawElements,
		// 	LayerId + 1,
		// 	AllottedGeometry.ToPaintGeometry(),
		// 	Points,
		// 	ESlateDrawEffect::None,
		// 	LineColor,
		// 	true,
		// 	LineThickness);

		FSlateDrawElement::MakeDashedLines(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(),
			MoveTemp(Points),
			ESlateDrawEffect::None,
			LineColor,
			LineThickness,
			10.f,
			0.f);
	}

	return MaxLayerId;
}

bool ULyraCalloutHostWidget::GetTargetScreenPosition(FVector2D& OutScreenPos) const
{
	if (!bShouldShowCallout)
	{
		return false;
	}

	if (!TargetActor.IsValid())
	{
		return false;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return false;
	}

	FVector WorldLoc = TargetActor->GetActorLocation();
	return PC->ProjectWorldLocationToScreen(WorldLoc, OutScreenPos);
}

void ULyraCalloutHostWidget::RegisterMessageHandlers()
{
	UGameInstance* GameInstance = GetGameInstance<UGameInstance>();
	if (GameInstance)
	{
		UGameplayMessageSubsystem* MessageSubsystem = GameInstance->GetSubsystem<UGameplayMessageSubsystem>();
		if (MessageSubsystem)
		{
			if (!DisplayCalloutEventListener.IsValid())
			{
				DisplayCalloutEventListener = MessageSubsystem->RegisterListener<FOnCalloutDisplayParameters>(
					LyraGameplayTags::Gameplay_Message_Callout_Display,
					this,
					&ThisClass::HandleCalloutDisplayEvent);
			}
		}
	}
}

void ULyraCalloutHostWidget::UnregisterMessageHandlers()
{
	if (DisplayCalloutEventListener.IsValid())
	{
		DisplayCalloutEventListener.Unregister();
	}
}

void ULyraCalloutHostWidget::HandleCalloutDisplayEvent(FGameplayTag Channel, const FOnCalloutDisplayParameters& Parameters)
{
	// 不管是显示还是隐藏 都需要通知一下Widget, 这需要Widget实现我们预期的接口
	if (Parameters.bShow)
	{
		bShouldShowCallout = false;

		SetTargetActor(Parameters.Actor);
		SetTargetActorWorldOffset(Parameters.ActorScreenOffset);
		SetCalloutWidgetClass(Parameters.CalloutWidgetClass);
		SetCalloutWidgetScreenOffset(Parameters.CalloutWidgetScreenOffset);

		if (CalloutWidget)
		{
			if (CalloutWidget->GetClass()->ImplementsInterface(UInteractableCallout::StaticClass()))
			{
				IInteractableCallout::Execute_K2_OnShowCalloutInteractablePrompt(CalloutWidget, Parameters.Actor);
				SetVisibility(ESlateVisibility::Visible);
				bShouldShowCallout = true;
			}
		}
	}
	else
	{
		if (CalloutWidget)
		{
			if (CalloutWidget->GetClass()->ImplementsInterface(UInteractableCallout::StaticClass()))
			{
				IInteractableCallout::Execute_K2_OnHideCalloutInteractablePrompt(CalloutWidget, Parameters.Actor);
			}
		}

		SetVisibility(ESlateVisibility::Hidden);
		bShouldShowCallout = false;
	}
}
