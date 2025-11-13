// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_CreateLobby.generated.h"

#define UE_API COMMONUSER_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCreateLobby_Delegate, const FString&, LobbyId);

UCLASS(MinimalAPI)
class UAsyncAction_CreateLobby : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "LobbySettings,MemberSettings", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_CreateLobby* CreateLobby(
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
	bool bIsCancelled = false;
	bool bIsCreating = false;

	UE_API virtual void Activate() override;
	UE_API void Execute_CreateLobby();
	UE_API void OnCreateLobbyCompleted(FName SessionName, bool bWasSuccessful);
	UE_API virtual void Cancel() override;
};

#undef UE_API