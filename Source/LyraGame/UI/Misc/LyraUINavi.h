// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraUINavi.generated.h"


USTRUCT(BlueprintType)
struct FLyraUINaviFocus
{
	GENERATED_BODY()

public:
	/** The Widget to be Focus **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi")
	TObjectPtr<UUserWidget> WidgetFocus;

	/** Widget identify **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi")
	FString WidgetId;
};


UCLASS(Blueprintable, BlueprintType, Abstract, meta = (DisableNativeTick))
class LYRAGAME_API ALyraUINavi : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALyraUINavi();

	/** Channel to Send/Recv Message **/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Navi", meta=(Categories = "UI.Layer"))
	FGameplayTag MessageChannel;

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintNativeEvent, Category="UI Navi")
	void OnAboutToReceiveFocus(const FGameplayTag& Channel, const FLyraUINaviFocus& Payload);
	virtual void OnAboutToReceiveFocus_Implementation(const FGameplayTag& Channel, const FLyraUINaviFocus& Payload) {}

private:
	void HandleAboutToReceiveFocus(FGameplayTag Channel, const FLyraUINaviFocus& Payload);

	FGameplayMessageListenerHandle AboutToReceiveFocusListener;
};
