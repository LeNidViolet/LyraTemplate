// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraUINavi.h"

ALyraUINavi::ALyraUINavi()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ALyraUINavi::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		UGameplayMessageSubsystem & MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		AboutToReceiveFocusListener = MessageSubsystem.RegisterListener<FLyraUINaviFocus>(
			MessageChannel,
			this,
			&ThisClass::HandleAboutToReceiveFocus
		);
	}
}

void ALyraUINavi::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		if (AboutToReceiveFocusListener.IsValid())
		{
			MessageSubsystem.UnregisterListener(AboutToReceiveFocusListener);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ALyraUINavi::HandleAboutToReceiveFocus(FGameplayTag Channel, const FLyraUINaviFocus& Payload)
{
	OnAboutToReceiveFocus(Channel, Payload);
}





