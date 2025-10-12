// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "OnlineSessionSettings.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_FindLobby.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindLobby_Delegate, const TArray<FString>&, Lobbies);


/**
 *
 */
UCLASS(MinimalAPI)
class UAsyncAction_FindLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "LobbySettings", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_FindLobby* FindLobby(
		UObject* WorldContextObject,
		APlayerController* Player,
		TMap<FName, FEIKAttribute> LobbySettings,
		int32 MaxResults = 10
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindLobby_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindLobby_Delegate OnFailure;

	UAsyncAction_FindLobby()
	{
		SessionSearch = MakeShared<FOnlineSessionSearch>();
	}

protected:
	FDelegateHandle FindLobbyDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TMap<FName, FEIKAttribute> LobbySettings;
	int32 MaxResults = 10;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	virtual void Activate() override;
	void Execute_FindLobby();
	void OnFindLobbyCompleted(bool bWasSuccessful);
};
