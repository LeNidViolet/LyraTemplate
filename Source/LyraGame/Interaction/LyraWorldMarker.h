// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraWorldMarker.generated.h"

#define UE_API LYRAGAME_API


struct FGameplayTag;
struct FOnMarkerDiscoverParameters;
class USphereComponent;



#define WORLD_MARKER_INDEX_INVALID		(int32)(-1)
#define WORLD_MARKER_INDEX_PRREDICTED	(int32)(-2)

UENUM(BlueprintType)
enum class ELyraWorldMarkerType : uint8
{
	None,
	Waypoint,
	Objective,
	Enemy,
	Friendly
};



USTRUCT()
struct FLyraWorldMarkerData
{
	// Marker同步数据

	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APlayerState> OwnerPlayer;

	UPROPERTY()
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::None;

	UPROPERTY()
	FLinearColor Color = FLinearColor::White;

	UPROPERTY()
	int32 MarkerId = WORLD_MARKER_INDEX_INVALID;

	// 本地预测标记点?
	UPROPERTY()
	bool bPredictedMarker = false;
};




UCLASS(MinimalAPI, BlueprintType, Blueprintable)
class ALyraWorldMarker : public AActor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="WorldMarker", meta=(WorldContext="WorldContextObject"))
	static UE_API ALyraWorldMarker* SpawnLyraWorldMarkerActor(
		UObject* WorldContextObject,
		TSubclassOf<ALyraWorldMarker> MarkerClass,
		AActor* TargetActor,
		const FVector& TargetLocation,
		ELyraWorldMarkerType MarkerType,
		APlayerState* OwnerPlayer,
		bool bAsPredictedMarker = false
	);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	static UE_API int32 InvalidId() { return WORLD_MARKER_INDEX_INVALID; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	static UE_API int32 PredictedId() { return WORLD_MARKER_INDEX_PRREDICTED; }

	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API APlayerState* GetOwnerPlayer() const { return MarkerData.OwnerPlayer.Get(); }
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API ELyraWorldMarkerType GetMarkerType() const { return MarkerData.MarkerType; }
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API FLinearColor GetMarkerColor() const { return MarkerData.Color; }
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API int32 GetMarkerId() const { return MarkerData.MarkerId; }
	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API bool IsPredictedMarker() const { return MarkerData.bPredictedMarker; }

protected:
	UE_API ALyraWorldMarker();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// 需要在BeginPlay之前设置一些参数, 因为 SpawnActorDeferred 无法传递额外参数??
	virtual void Initialize(FLyraWorldMarkerData InMarkerData);

	UFUNCTION()
	void OnParentDestroyed(AActor* DestroyedActor);

	UPROPERTY(ReplicatedUsing=OnRep_MarkerData)
	FLyraWorldMarkerData MarkerData;
	UFUNCTION()
	void OnRep_MarkerData();

	UPROPERTY()
	TObjectPtr<USphereComponent> SphereComp;

	// 广播添加/移除标记点的消息
	void Broadcast_MarkerAddedMessage();
	void Broadcast_MarkerRemovedMessage();

	void RegisterMessageHandlers();
	void UnregisterMessageHandlers();
	FGameplayMessageListenerHandle MarkerDiscoverRequestListener;
	void HandleMarkerDiscoverRequest(FGameplayTag Channel, const FOnMarkerDiscoverParameters& Parameters);


	bool bComponentsInitialized = false;

	static int32 NextMarkerId;
};


#undef UE_API