// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_LeaveLobby.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLeaveLobby_Delegate);


/**
 *
 */
UCLASS(MinimalAPI)
class UAsyncAction_LeaveLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_LeaveLobby* LeaveLobby(UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FLeaveLobby_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FLeaveLobby_Delegate OnFailure;

protected:

	FDelegateHandle DestroyLobbyDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	virtual void Activate() override;
	void Execute_LeaveLobby();
};
