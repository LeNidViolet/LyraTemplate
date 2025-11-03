// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LyraWorldMarker.generated.h"

#define UE_API LYRAGAME_API

class USphereComponent;



#define WORLD_MARKER_INDEX_INVALID		(-1)
#define WORLD_MARKER_INDEX_PRREDICTED	(-2)



UENUM(BlueprintType)
enum class ELyraWorldMarkerType : uint8
{
	None,
	Waypoint,
	Objective,
	Enemy,
	Friendly
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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	UE_API APlayerState* GetOwnerPlayer() const { return OwnerPlayer.Get(); }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	UE_API ELyraWorldMarkerType GetMarkerType() const { return MarkerType; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	UE_API FLinearColor GetColor() const { return Color; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	UE_API int32 GetMarkerId() const { return MarkerId; }
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	UE_API bool IsPredictedMarker() const { return bPredictedMarker; }

protected:
	UE_API ALyraWorldMarker();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// 需要在BeginPlay之前设置一些参数, 因为 SpawnActorDeferred 无法传递额外参数??
	UE_API virtual void Initialize(
		APlayerState* InOwnerPlayer,
		ELyraWorldMarkerType InMarkerType,
		const FLinearColor& InColor,
		int32 InMarkerId = InvalidId(),
		bool InPredictedMarker = false);

	UPROPERTY(Replicated)
	TObjectPtr<APlayerState> OwnerPlayer;

	UPROPERTY(Replicated)
	ELyraWorldMarkerType MarkerType = ELyraWorldMarkerType::None;

	UPROPERTY(Replicated)
	FLinearColor Color;

	UPROPERTY(Replicated)
	int32 MarkerId = 0;

	// 本地预测标记点?
	UPROPERTY(Replicated)
	bool bPredictedMarker = false;

	UPROPERTY(Replicated)
	TObjectPtr<USphereComponent> SphereComp;

	// 广播添加/移除标记点的消息
	void Broadcast_MarkerAddedMessage();
	void Broadcast_MarkerRemovedMessage();


	static int32 NextMarkerId;
};


#undef UE_API