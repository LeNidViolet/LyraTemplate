// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_Types.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_JoinLobby.generated.h"


#define UE_API COMMONUSER_API


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJoinLobby_Delegate);



/**
 *
 */
UCLASS(MinimalAPI)
class UAsyncAction_JoinLobby : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_JoinLobby* JoinLobby(
		UObject* WorldContextObject,
		APlayerController* Player,
		const FString& LobbyId,
		TMap<FName, FEIKAttribute> MemberSettings
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FJoinLobby_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FJoinLobby_Delegate OnFailure;


protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	FString LobbyId;
	TMap<FName, FEIKAttribute> MemberSettings;
	FDelegateHandle JoinLobbyDelegateHandle;
	bool bIsCancelled = false;
	bool bIsSearching = false;
	bool bIsJoining = false;

	UE_API virtual void Activate() override;
	UE_API void Execute_JoinLobby();
	UE_API void OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);
	UE_API void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UE_API virtual void Cancel() override;
};

#undef UE_API
