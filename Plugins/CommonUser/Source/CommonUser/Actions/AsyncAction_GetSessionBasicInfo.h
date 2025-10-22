// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_GetSessionBasicInfo.generated.h"

USTRUCT(BlueprintType)
struct FSessionBasicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PingInMs = 0;

	/** The number of publicly available connections that are available (read only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumOpenPublicConnections = 0;

	/** The number of publicly available connections advertised */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumPublicConnections = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FEIKAttribute> Attributes;
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetSessionBasicInfo_Delegate, FSessionBasicInfo, SessioniBasicInfo);


UCLASS(MinimalAPI)
class UAsyncAction_GetSessionBasicInfo : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_GetSessionBasicInfo* GetSessionBasicInfo(
		UObject* WorldContextObject,
		APlayerController* Player,
		const FString& SessionId,
		TArray<FName> ExposeAttributes
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FGetSessionBasicInfo_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FGetSessionBasicInfo_Delegate OnFailure;

protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TArray<FName> ExposeAttributes;
	FString SessionId;

	virtual void Activate() override;
	void Execute_GetSessionBasicInfo();
	void OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);

};
