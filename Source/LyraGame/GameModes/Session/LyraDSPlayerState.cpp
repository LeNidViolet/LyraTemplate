// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraDSPlayerState.h"
#include "Interaction/LyraWorldMarker.h"
#include "Net/UnrealNetwork.h"


ALyraDSPlayerState::ALyraDSPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ALyraDSPlayerState::AddWorldMarkerToCache(ALyraWorldMarker* MarkerActor)
{
	if (MarkerActor)
	{
		check(!HasWorldMarkerInCache(MarkerActor->GetMarkerId()));
		MarkerList.Add(MarkerActor);
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
		MarkerActor->Destroy();
		MarkerList.RemoveAt(Index);
	}
}

void ALyraDSPlayerState::RemoveWorldMarkerFromCache(ELyraWorldMarkerType MarkerType)
{
	for (int32 i = MarkerList.Num() - 1; i >= 0; --i)
	{
		TObjectPtr<ALyraWorldMarker>& MarkerActor = MarkerList[i];
		if (MarkerActor && MarkerActor->GetMarkerType() == MarkerType)
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
		if (MarkerActor)
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
		return MarkerActor && MarkerId == MarkerActor->GetMarkerId();
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
