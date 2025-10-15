// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraMarkerManagerComponent.generated.h"

#define UE_API LYRAGAME_API

USTRUCT(BlueprintType)
struct FLyraMessageMarkerToggle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
	bool bAddMarker = true;

	// used for add
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
	FVector Location = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
	TObjectPtr<AActor> Actor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;

	// used for remove
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Marker")
	TObjectPtr<UIndicatorDescriptor> DescriptorObject;
};



USTRUCT()
struct FLyraMarkerInstance
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;
	UPROPERTY()
	TWeakObjectPtr<UIndicatorDescriptor> DescriptorObject;
};



UCLASS(MinimalAPI)
class ULyraMarkerManagerComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UE_API ULyraMarkerManagerComponent(const FObjectInitializer& ObjectInitializer);

	static UE_API ULyraMarkerManagerComponent* GetComponent(AController* Controller);

	UE_API TSharedPtr<FLyraMarkerInstance> GetMarkerInstance();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Returns the distance, in meters.
	UFUNCTION(BlueprintCallable, Category="MarkerInfo")
	int32 GetDistanceToLocation(UIndicatorDescriptor* DescriptorObject, const FVector& TargetLocation);

private:
	bool Initialize();
	void Deinitialize();

	FGameplayMessageListenerHandle MarkerToggleEventListener;
	void HandleMarkerToggleEvent(FGameplayTag Channel, const FLyraMessageMarkerToggle& Payload);

	void HandleMarkerAdd(const FLyraMessageMarkerToggle& Payload);
	void HandleMarkerRemove(const FLyraMessageMarkerToggle& Payload);
	void RemoveExistingMarker();

	TSharedPtr<FLyraMarkerInstance> MarkerInstance;
};

#undef UE_API