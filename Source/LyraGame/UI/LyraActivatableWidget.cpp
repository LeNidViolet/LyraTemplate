// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraActivatableWidget.h"

#include "Editor/WidgetCompilerLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraActivatableWidget)

#define LOCTEXT_NAMESPACE "Lyra"

ULyraActivatableWidget::ULyraActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TOptional<FUIInputConfig> ULyraActivatableWidget::GetDesiredInputConfig() const
{
	FUIInputConfig Result;

	switch (InputConfig)
	{
	case ELyraWidgetInputMode::GameAndMenu:
		Result = FUIInputConfig(ECommonInputMode::All, GameMouseCaptureMode);
		break;
	case ELyraWidgetInputMode::Game:
		Result = FUIInputConfig(ECommonInputMode::Game, GameMouseCaptureMode);
		break;
	case ELyraWidgetInputMode::Menu:
		Result = FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
		break;
	case ELyraWidgetInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}

	Result.bIgnoreLookInput = bIgnoreLookInput;
	return Result;
}

#if WITH_EDITOR

void ULyraActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);

	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(ULyraActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == ULyraActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			//TODO - Note for now, because we can't guarantee it isn't implemented in a native subclass of this one.
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen.  If it was implemented in the native base class you can ignore this message."));
		}
	}
}

void ULyraActivatableWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 在最顶级页面上禁用导航, 避免焦点移动到下层页面上

	SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Next, EUINavigationRule::Stop);
	SetNavigationRuleBase(EUINavigation::Previous, EUINavigationRule::Stop);
}

#endif

#undef LOCTEXT_NAMESPACE
