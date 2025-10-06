// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraToastMessageSubsystem.h"

#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"


UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_TOAST_MESSAGE, "ToastMessage")


void ULyraToastMessageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	ToastMessageListener = MessageSystem.RegisterListener<FLyraToastMessage>(
		TAG_TOAST_MESSAGE,
		[this](FGameplayTag Channel, const FLyraToastMessage& Payload)
		{
			if (this)
			{
				this->HandleToastMessage(Channel, Payload);
			}
		},
		EGameplayMessageMatch::PartialMatch
		);
}

void ULyraToastMessageSubsystem::Deinitialize()
{
	if (ToastMessageListener.IsValid())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSystem.UnregisterListener(ToastMessageListener);
	}

	Super::Deinitialize();
}

void ULyraToastMessageSubsystem::HandleToastMessage(FGameplayTag Channel, const FLyraToastMessage& Payload)
{
	OnToastMessageReceived.Broadcast(Channel, Payload);
}
