// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LyraToastMessageSubsystem.generated.h"



USTRUCT(BlueprintType)
struct FLyraToastMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast Message")
	FString StringValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast Message")
	int32 NumberValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast Message")
	bool BooleanValue = false;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnToastMessageReceived, FGameplayTag, Channel, const FLyraToastMessage&, Payload);

UCLASS(MinimalAPI)
class ULyraToastMessageSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category="Toast Message")
	FOnToastMessageReceived OnToastMessageReceived;

	UPROPERTY(BlueprintAssignable, Category="Toast Message")
	FOnToastMessageReceived OnToastLobbyMemberEventReveived;

	UPROPERTY(BlueprintAssignable, Category="Toast Message")
	FOnToastMessageReceived OnToastLobbyCountdownReveived;

private:

	void HandleToastMessage(FGameplayTag Channel, const FLyraToastMessage& Payload);
	FGameplayMessageListenerHandle ToastMessageListener;
};
