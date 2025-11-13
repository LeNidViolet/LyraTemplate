// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSPlayerState.h"
#include "Interaction/LyraWorldMarker.h"
#include "Net/UnrealNetwork.h"

#include  UE_INLINE_GENERATED_CPP_BY_NAME(LyraDSPlayerState)

ALyraDSPlayerState::ALyraDSPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ALyraDSPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void ALyraDSPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 销毁所有标记点 服务端销毁的是权威标记点 客户端销毁的是预测标记点
	for (ALyraWorldMarker* MarkerActor : MarkerList)
	{
		if (IsValid(MarkerActor))
		{
			MarkerActor->Destroy();
		}
	}
	MarkerList.Empty();

	Super::EndPlay(EndPlayReason);
}




void ALyraDSPlayerState::AddWorldMarkerToCache(ALyraWorldMarker* MarkerActor)
{
	if (MarkerActor)
	{
		if (!HasWorldMarkerInCache(MarkerActor->GetMarkerId()))
		{
			MarkerList.Add(MarkerActor);
		}
	}
}

void ALyraDSPlayerState::RemoveWorldMarkerFromCache(ALyraWorldMarker* MarkerActor)
{
	if (MarkerActor)
	{
		RemoveWorldMarkerFromCache(MarkerActor->GetMarkerId());
	}
}

void ALyraDSPlayerState::RemoveWorldMarkerFromCache(int32 MarkerId)
{
	int32 Index = MarkerList.IndexOfByPredicate([MarkerId](const TObjectPtr<ALyraWorldMarker>& MarkerActor)
	{
		return MarkerActor && MarkerId == MarkerActor->GetMarkerId();
	});

	if (MarkerList.IsValidIndex(Index))
	{
		TObjectPtr<ALyraWorldMarker>& MarkerActor = MarkerList[Index];
		if (IsValid(MarkerActor))
		{
			MarkerActor->Destroy();
		}
		MarkerList.RemoveAt(Index);
	}
}

void ALyraDSPlayerState::RemoveWorldMarkerFromCache(ELyraWorldMarkerType MarkerType)
{
	for (int32 i = MarkerList.Num() - 1; i >= 0; --i)
	{
		TObjectPtr<ALyraWorldMarker>& MarkerActor = MarkerList[i];
		if (IsValid(MarkerActor.Get()) && MarkerActor->GetMarkerType() == MarkerType)
		{
			MarkerActor->Destroy();
			MarkerList.RemoveAt(i);
		}
	}
}

void ALyraDSPlayerState::RemoveAllWorldMarkers()
{
	for (TObjectPtr<ALyraWorldMarker>& MarkerActor : MarkerList)
	{
		if (IsValid(MarkerActor.Get()))
		{
			MarkerActor->Destroy();
		}
	}
	MarkerList.Empty();
}

bool ALyraDSPlayerState::HasWorldMarkerInCache(int32 MarkerId) const
{
	int32 Index = MarkerList.IndexOfByPredicate([MarkerId](const TObjectPtr<ALyraWorldMarker>& MarkerActor)
	{
		return MarkerId == MarkerActor->GetMarkerId();
	});

	return MarkerList.IsValidIndex(Index);
}

ALyraWorldMarker* ALyraDSPlayerState::GetWorldMarkerForId(int32 MarkerId) const
{
	int32 Index = MarkerList.IndexOfByPredicate([MarkerId](const TObjectPtr<ALyraWorldMarker>& MarkerActor)
	{
		return IsValid(MarkerActor.Get()) && MarkerId == MarkerActor->GetMarkerId();
	});

	if (MarkerList.IsValidIndex(Index))
	{
		return MarkerList[Index].Get();
	}
	return nullptr;
}


void ALyraDSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALyraDSPlayerState, PlayerColor);
}

bool ALyraDSPlayerState::RemoveWorldMarkerForId(int32 MarkerId)
{
	if (HasWorldMarkerInCache(MarkerId))
	{
		RemoveWorldMarkerFromCache(MarkerId);
		return true;
	}
	return false;
}

void ALyraDSPlayerState::OnRep_PlayerColor()
{
	OnPlayerColorChanged.Broadcast(PlayerColor);
}

