// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraConfirmationScreen.h"

#if WITH_EDITOR
#include "CommonInputSettings.h"
#include "Editor/WidgetCompilerLog.h"
#endif

#include "CommonBorder.h"
#include "CommonRichTextBlock.h"
#include "CommonTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "ICommonInputModule.h"
#include "LyraButtonBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraConfirmationScreen)

void ULyraConfirmationScreen::SetupDialog(UCommonGameDialogDescriptor* Descriptor, FCommonMessagingResultDelegate ResultCallback)
{
	Super::SetupDialog(Descriptor, ResultCallback);

	Text_Title->SetText(Descriptor->Header);
	RichText_Description->SetText(Descriptor->Body);

	EntryBox_Buttons->Reset<ULyraButtonBase>([](ULyraButtonBase& Button)
	{
		Button.OnClicked().Clear();
	});

	for (const FConfirmationDialogAction& Action : Descriptor->ButtonActions)
	{
		if (EnhancedCancelAction)
		{
			UInputAction* EnhancedAction = nullptr;

			if (EnhancedCancelAction)
			{
				switch(Action.Result)
				{
				case ECommonMessagingResult::Confirmed:
					EnhancedAction = ICommonInputModule::GetSettings().GetEnhancedInputClickAction();
					break;
				case ECommonMessagingResult::Declined:
					EnhancedAction = ICommonInputModule::GetSettings().GetEnhancedInputBackAction();
					break;
				case ECommonMessagingResult::Cancelled:
					EnhancedAction = EnhancedCancelAction;
					break;
				default:
					ensure(false);
					continue;
				}
			}


 			ULyraButtonBase* Button = EntryBox_Buttons->CreateEntry<ULyraButtonBase>();
			if (EnhancedAction)
			{
				Button->SetTriggeringEnhancedInputAction(EnhancedAction);
			}
 			Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
 			Button->SetButtonText(Action.OptionalDisplayText);

 			LastButton = Button;
		}
		else
		{
			FDataTableRowHandle ActionRow;

			if (!CancelAction.IsNull())
			{
				switch(Action.Result)
				{
				case ECommonMessagingResult::Confirmed:
					ActionRow = ICommonInputModule::GetSettings().GetDefaultClickAction();
					break;
				case ECommonMessagingResult::Declined:
					ActionRow = ICommonInputModule::GetSettings().GetDefaultBackAction();
					break;
				case ECommonMessagingResult::Cancelled:
					ActionRow = CancelAction;
					break;
				default:
					ensure(false);
					continue;
				}
			}

			ULyraButtonBase* Button = EntryBox_Buttons->CreateEntry<ULyraButtonBase>();
			if (!ActionRow.IsNull())
			{
				Button->SetTriggeringInputAction(ActionRow);
			}
			Button->OnClicked().AddUObject(this, &ThisClass::CloseConfirmationWindow, Action.Result);
			Button->SetButtonText(Action.OptionalDisplayText);

			LastButton = Button;
		}
	}

	OnResultCallback = ResultCallback;
}

void ULyraConfirmationScreen::KillDialog()
{
	Super::KillDialog();
}

UWidget* ULyraConfirmationScreen::NativeGetDesiredFocusTarget() const
{
	if (LastButton)
	{
		return LastButton;
	}
	else
	{
		return Super::NativeGetDesiredFocusTarget();
	}
}

void ULyraConfirmationScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Border_TapToCloseZone->OnMouseButtonDownEvent.BindDynamic(this, &ULyraConfirmationScreen::HandleTapToCloseZoneMouseButtonDown);
}

void ULyraConfirmationScreen::CloseConfirmationWindow(ECommonMessagingResult Result)
{
	DeactivateWidget();
	OnResultCallback.ExecuteIfBound(Result);
}

FEventReply ULyraConfirmationScreen::HandleTapToCloseZoneMouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Unhandled();

	if (MouseEvent.IsTouchEvent() || MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		CloseConfirmationWindow(ECommonMessagingResult::Declined);
		Reply.NativeReply = FReply::Handled();
	}

	return Reply;
}

#if WITH_EDITOR
void ULyraConfirmationScreen::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
//	if (CancelAction.IsNull() && !EnhancedCancelAction)
//	{
//		CompileLog.Error(FText::Format(FText::FromString(TEXT("{0} has unset property: CancelAction && EnhancedCancelAction.")), FText::FromString(GetName())));
//	}
}
#endif
