// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerState.h"
#include "LyraDSPlayerState.generated.h"

#define UE_API LYRAGAME_API

enum class ELyraWorldMarkerType : uint8;
class ALyraWorldMarker;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerColorChanged_Dynamic, FColor, NewColor);

UCLASS(MinimalAPI)
class ALyraDSPlayerState : public ALyraPlayerState
{
	GENERATED_BODY()

public:
	UE_API ALyraDSPlayerState(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category="PlayerState")
	FOnPlayerColorChanged_Dynamic OnPlayerColorChanged;

	UPROPERTY(ReplicatedUsing=OnRep_PlayerColor, BlueprintReadOnly, VisibleAnywhere, Category="PlayerState")
	FColor PlayerColor = FColor::White;

	UE_API void AddWorldMarkerToCache(ALyraWorldMarker* MarkerActor);
	UE_API void RemoveWorldMarkerFromCache(ALyraWorldMarker* MarkerActor);
	UE_API void RemoveWorldMarkerFromCache(int32 MarkerId);
	UE_API void RemoveWorldMarkerFromCache(ELyraWorldMarkerType MarkerType);
	UE_API void RemoveAllWorldMarkers();
	UE_API bool HasWorldMarkerInCache(int32 MarkerId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	UE_API ALyraWorldMarker* GetWorldMarkerForId(int32 MarkerId) const;

	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	UE_API bool RemoveWorldMarkerForId(int32 MarkerId);

protected:
	UFUNCTION()
	void OnRep_PlayerColor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	// 缓存生成的标记点列表 在服务端保存的是权威标记点 在客户端保存的是预测标记点
	UPROPERTY()
	TArray<TObjectPtr<ALyraWorldMarker>> MarkerList;
};

#undef UE_API