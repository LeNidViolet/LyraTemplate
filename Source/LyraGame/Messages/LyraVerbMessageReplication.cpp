// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraVerbMessageReplication.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/LyraVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraVerbMessageReplication)

//////////////////////////////////////////////////////////////////////
// FLyraVerbMessageReplicationEntry

FString FLyraVerbMessageReplicationEntry::GetDebugString() const
{
	return Message.ToString();
}

void FLyraVerbMessageReplicationEntry::SetTimeCreated(float InTime)
{
	TimeCreated = InTime;
}

//////////////////////////////////////////////////////////////////////
// FLyraVerbMessageReplication

void FLyraVerbMessageReplication::AddMessage(const FLyraVerbMessage& Message)
{
	CleanupExpiredMessages(10.0f);

	FLyraVerbMessageReplicationEntry& NewStack = CurrentMessages.Emplace_GetRef(Message);

	const float Now = Owner->GetWorld() ? Owner->GetWorld()->GetTimeSeconds() : 0.f;
	NewStack.SetTimeCreated(Now);

	MarkItemDirty(NewStack);
}

void FLyraVerbMessageReplication::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
// 	for (int32 Index : RemovedIndices)
// 	{
// 		const FGameplayTag Tag = CurrentMessages[Index].Tag;
// 		TagToCountMap.Remove(Tag);
// 	}
}

void FLyraVerbMessageReplication::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FLyraVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FLyraVerbMessageReplication::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FLyraVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FLyraVerbMessageReplication::RebroadcastMessage(const FLyraVerbMessage& Message)
{
	check(Owner);
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner);
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}

void FLyraVerbMessageReplication::CleanupExpiredMessages(float MaxAgeSeconds)
{
	check(Owner);

	const float Now = Owner->GetWorld() ? Owner->GetWorld()->GetTimeSeconds() : 0.f;
	bool bRemovedAny = false;

	for (int32 Index = CurrentMessages.Num() - 1; Index >= 0; --Index)
	{
		const FLyraVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		if (Now - Entry.TimeCreated > MaxAgeSeconds)
		{
			CurrentMessages.RemoveAt(Index);
			bRemovedAny = true;
		}
	}

	if (bRemovedAny)
	{
		MarkArrayDirty();
	}
}

