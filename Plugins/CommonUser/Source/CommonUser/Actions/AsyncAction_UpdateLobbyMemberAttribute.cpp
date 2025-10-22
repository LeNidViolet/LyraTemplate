// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_UpdateLobbyMemberAttribute.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "AsyncAction_Helper.h"
#include "AsyncAction_LogChannel.h"


UAsyncAction_UpdateLobbyMemberAttribute* UAsyncAction_UpdateLobbyMemberAttribute::UpdateLobbyMemberAttribute(
	UObject* WorldContextObject, APlayerController* Player, TMap<FName, FEIKAttribute> MemberSettings)
{
	if (!WorldContextObject || !Player || MemberSettings.Num() == 0)
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_UpdateLobbyMemberAttribute::UpdateLobbyMemberAttribute: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_UpdateLobbyMemberAttribute* Action = NewObject<UAsyncAction_UpdateLobbyMemberAttribute>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->MemberSettings = MemberSettings;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_UpdateLobbyMemberAttribute::Activate()
{
	Execute_UpdateLobbyMemberAttribute();
	Super::Activate();
}

void UAsyncAction_UpdateLobbyMemberAttribute::Execute_UpdateLobbyMemberAttribute()
{
	bool bSuccess = false;
	if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
	{
		if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			if (MemberSettings.Num() > 0)
			{
				bool bOk = AsyncAction_Helper::UpdateMemberAttributes(
					WorldContextObject.Get(),
					Player->GetLocalPlayer()->GetLocalPlayerIndex(),
					NAME_GameSession,
					MemberSettings);
				if (!bOk)
				{
					UE_LOG(LogCommonSessionAsyncAction, Warning, TEXT("UAsyncAction_CreateLobby::OnCreateLobbyCompleted: Failed to update member attributes"));
				}
				else
				{
					bSuccess = true;
				}
			}
		}
	}



	auto HandleResult = [this, bSuccess]()
	{
		if (bSuccess)
		{
			OnSuccess.Broadcast();
		}
		else
		{
			OnFailure.Broadcast();
		}

		SetReadyToDestroy();
	};

	UWorld* World = WorldContextObject->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(HandleResult));
	}
	else
	{
		HandleResult();
	}
}
