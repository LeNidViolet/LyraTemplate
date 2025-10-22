// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IndicatorDescriptor.h"
#include "Components/ControllerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraMarkerManagerComponent.generated.h"

#define UE_API LYRAGAME_API

struct FOnPlaceMarkerParameters;
struct FOnRemoveMarkerParameters;


USTRUCT()
struct FLyraMarkerInstance
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	UPROPERTY()
	FGuid MarkerId;
	UPROPERTY()
	TObjectPtr<APlayerState> PlayerState;
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

	UE_API TSharedPtr<FLyraMarkerInstance> GetMarkerInstance(APlayerState* PlayerState);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Returns the distance, in meters.
	UFUNCTION(BlueprintCallable, Category="Marker")
	int32 GetDistanceToLocation(UIndicatorDescriptor* DescriptorObject, const FVector& TargetLocation);


	UFUNCTION(BlueprintCallable, Category="Marker")
	FGuid GetMarkerId(UIndicatorDescriptor* DescriptorObject);

	UFUNCTION(BlueprintCallable, Category="Marker")
	APlayerState* GetMarkerPlayerState(UIndicatorDescriptor* DescriptorObject);

private:
	bool Initialize();
	void Deinitialize();

	FGameplayMessageListenerHandle MarkerAddEventListener;
	void HandlePlaceMarkerEvent(FGameplayTag Channel, const FOnPlaceMarkerParameters& Parameters);
	FGameplayMessageListenerHandle MarkerRemoveEventListener;
	void HandleRemoveMarkerEvent(FGameplayTag Channel, const FOnRemoveMarkerParameters& Parameters);

	UPROPERTY(EditDefaultsOnly, Category="Marker")
	TSubclassOf<UIndicatorDescriptor> DescriptorClass;

	TArray<TSharedPtr<FLyraMarkerInstance>> MarkerList;
};

#undef UE_API