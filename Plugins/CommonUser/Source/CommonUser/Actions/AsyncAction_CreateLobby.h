// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_CreateLobby.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateLobby_Delegate, const FString&, LobbyId);

UCLASS(MinimalAPI)
class UAsyncAction_CreateLobby : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "LobbySettings,MemberSettings", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_CreateLobby* CreateLobby(
		UObject* WorldContextObject,
		APlayerController* Player,
		const TMap<FName, FEIKAttribute> LobbySettings,
		const TMap<FName, FEIKAttribute> MemberSettings,
		int32 NumberOfPublicConnections=10
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FCreateLobby_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FCreateLobby_Delegate OnFailure;

protected:
	FDelegateHandle CreateLobbyDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	int32 NumberOfPublicConnections;
	TMap<FName, FEIKAttribute> LobbySettings;
	TMap<FName, FEIKAttribute> MemberSettings;
	bool bDelegateCalled = false;

	virtual void Activate() override;
	void Execute_CreateLobby();
	void OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful);
};
