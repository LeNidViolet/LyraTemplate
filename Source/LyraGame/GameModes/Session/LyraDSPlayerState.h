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

	void AddWorldMarkerToCache(ALyraWorldMarker* MarkerActor);
	void RemoveWorldMarkerFromCache(ALyraWorldMarker* MarkerActor);
	void RemoveWorldMarkerFromCache(int32 MarkerId);
	void RemoveWorldMarkerFromCache(ELyraWorldMarkerType MarkerType);
	void RemoveAllWorldMarkers();
	bool HasWorldMarkerInCache(int32 MarkerId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="WorldMarker")
	ALyraWorldMarker* GetWorldMarkerForId(int32 MarkerId) const;

	UFUNCTION(BlueprintCallable, Category="WorldMarker")
	bool RemoveWorldMarkerForId(int32 MarkerId);

protected:
	UFUNCTION()
	void OnRep_PlayerColor();

private:

	// 缓存生成的标记点列表 只存在于服务端, 不要复制, 否则产生不必要的网络开销
	// 服务端在生成标记时添加
	UPROPERTY()
	TArray<TObjectPtr<ALyraWorldMarker>> MarkerList;
};

#undef UE_API