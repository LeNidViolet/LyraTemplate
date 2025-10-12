// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_Types.h"
#include "AsyncAction_JoinLobby.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJoinLobby_Delegate);



/**
 *
 */
UCLASS(MinimalAPI)
class UAsyncAction_JoinLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_JoinLobby* JoinLobby(
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

	virtual void Activate() override;
	void Execute_JoinLobby();
	void OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
