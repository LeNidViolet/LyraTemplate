// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "OnlineSessionSettings.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_FindLobby.generated.h"

#define UE_API COMMONUSER_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindLobby_Delegate, const TArray<FString>&, Lobbies);


/**
 *
 */
UCLASS(MinimalAPI)
class UAsyncAction_FindLobby : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "LobbySettings", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_FindLobby* FindLobby(
		UObject* WorldContextObject,
		APlayerController* Player,
		TMap<FName, FEIKAttribute> LobbySettings,
		int32 MaxResults = 10
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindLobby_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindLobby_Delegate OnFailure;

	UE_API UAsyncAction_FindLobby()
	{
		SessionSearch = MakeShared<FOnlineSessionSearch>();
	}

protected:
	FDelegateHandle FindLobbyDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TMap<FName, FEIKAttribute> LobbySettings;
	int32 MaxResults = 10;
	bool bIsCancelled = false;
	bool bIsSearching = false;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UE_API virtual void Activate() override;
	UE_API void Execute_FindLobby();
	UE_API void OnFindLobbyCompleted(bool bWasSuccessful);
	UE_API virtual void Cancel() override;
};

#undef UE_API
