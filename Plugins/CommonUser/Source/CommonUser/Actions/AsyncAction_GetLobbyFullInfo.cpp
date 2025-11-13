// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_GetLobbyFullInfo.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_Helper.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_GetLobbyFullInfo* UAsyncAction_GetLobbyFullInfo::GetLobbyFullInfo(
	UObject* WorldContextObject,
	APlayerController* Player)
{
	if (!Player || !WorldContextObject)
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_GetLobbyFullInfo::GetLobbyFullInfo: Invalid parameters"));
		return nullptr;
	}

	UAsyncAction_GetLobbyFullInfo* Action = NewObject<UAsyncAction_GetLobbyFullInfo>();
	Action->WorldContextObject = WorldContextObject;
	Action->Player = Player;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_GetLobbyFullInfo::Activate()
{
	Execute_GetLobbyFullInfo();
	Super::Activate();
}

void UAsyncAction_GetLobbyFullInfo::Execute_GetLobbyFullInfo()
{
	if (!WorldContextObject.IsValid() || !Player.IsValid())
	{
		auto HandleFailure = [this]()
		{
			OnFailure.Broadcast(FLobbyFullInfo());
			SetReadyToDestroy();
		};

		if (UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr)
		{
			World->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateLambda(HandleFailure));
		}
		else
		{
			HandleFailure();
		}
		return;
	}

	bool bSuccess = false;
	if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
	{
		if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			FNamedOnlineSession* NamedSession = SessionPtr->GetNamedSession(NAME_GameSession);
			if (NamedSession)
			{
				Result.LobbyId = NamedSession->GetSessionIdStr();
				Result.NumOpenPublicConnections = NamedSession->NumOpenPublicConnections;
				Result.NumPublicConnections = NamedSession->SessionSettings.NumPublicConnections;
				if (NamedSession->OwningUserId.IsValid())
				{
					Result.OwnerEpicId = AsyncAction_Helper::GetEpicID(NamedSession->OwningUserId);
					Result.OwnerProductId = AsyncAction_Helper::GetProductUserID(NamedSession->OwningUserId);
					Result.OwnerNetId = FUniqueNetIdRepl(NamedSession->OwningUserId);
				}
				for (const TPair<FName, FOnlineSessionSetting>& Pair : NamedSession->SessionSettings.Settings)
				{
					FEIKAttribute Attribute(Pair.Value.Data);
					Result.Attributes.Add(Pair.Key, Attribute);
				}

				IOnlineUserPtr UserPtr = OnlineSubsystem->GetUserInterface();
				if (UserPtr)
				{
					for (const auto & Pair : NamedSession->SessionSettings.MemberSettings)
					{
						FLobbyMemberFullInfo MemberInfo;

						FUniqueNetIdPtr UniqueNetId = Pair.Key;
						FString EpicId = AsyncAction_Helper::GetEpicID(UniqueNetId);
						FString ProductId = AsyncAction_Helper::GetProductUserID(UniqueNetId);

						MemberInfo.EpicId = EpicId;
						MemberInfo.ProductId = ProductId;
						MemberInfo.NetId = FUniqueNetIdRepl(UniqueNetId);

						if (MemberInfo.EpicId == Result.OwnerEpicId && MemberInfo.ProductId == Result.OwnerProductId)
						{
							MemberInfo.bIsOwner = true;
						}
						else
						{
							MemberInfo.bIsOwner = false;
						}

						for (const TTuple<FName, FOnlineSessionSetting>& Entry : Pair.Value)
						{
							MemberInfo.Attributes.Add(Entry.Key, FEIKAttribute(Entry.Value.Data));
						}

						Result.Members.Add(MemberInfo);
					}

					bSuccess = true;
				}
			}
		}
	}


	auto HandleResult = [this, bSuccess]()
	{
		if (bSuccess)
		{
			OnSuccess.Broadcast(Result);
		}
		else
		{
			OnFailure.Broadcast(FLobbyFullInfo());
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

void UAsyncAction_GetLobbyFullInfo::Cancel()
{
	bIsCancelled = true;

	// 这里的操作是'同步'的 没有什么需要取消的

	Super::Cancel();
}
