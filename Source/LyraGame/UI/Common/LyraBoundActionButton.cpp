// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraBoundActionButton.h"

#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraBoundActionButton)

class UCommonButtonStyle;

void ULyraBoundActionButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		if (!InputMethodChangedHandle.IsValid())
		{
			InputMethodChangedHandle = InputSubsystem->OnInputMethodChangedNative.AddUObject(this, &ThisClass::HandleInputMethodChanged);
			HandleInputMethodChanged(InputSubsystem->GetCurrentInputType());
		}
	}
}

void ULyraBoundActionButton::NativeDestruct()
{
	if (UCommonInputSubsystem* InputSubsystem = GetInputSubsystem())
	{
		if (InputMethodChangedHandle.IsValid())
		{
			InputSubsystem->OnInputMethodChangedNative.Remove(InputMethodChangedHandle);
			InputMethodChangedHandle.Reset();
		}
	}

	Super::NativeDestruct();
}

void ULyraBoundActionButton::HandleInputMethodChanged(ECommonInputType NewInputMethod)
{
	TSubclassOf<UCommonButtonStyle> NewStyle;

	if (NewInputMethod == ECommonInputType::Gamepad)
	{
		NewStyle = GamepadStyle;
	}
	else if (NewInputMethod == ECommonInputType::Touch)
	{
		NewStyle = TouchStyle;
	}
	else
	{
		NewStyle = KeyboardStyle;
	}

	if (NewStyle)
	{
		SetStyle(NewStyle);
	}
}

