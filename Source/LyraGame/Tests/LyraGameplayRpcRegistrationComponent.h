// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ExternalRpcRegistrationComponent.h"
#include "Serialization/JsonSerializer.h"

#include "Dom/JsonObject.h"
#include "LyraGameplayRpcRegistrationComponent.generated.h"

#define UE_API LYRAGAME_API


UCLASS(MinimalAPI)
class ULyraGameplayRpcRegistrationComponent : public UExternalRpcRegistrationComponent
{
	GENERATED_BODY()
protected:
	static UE_API ULyraGameplayRpcRegistrationComponent* ObjectInstance;

public:
	static UE_API ULyraGameplayRpcRegistrationComponent* GetInstance();


#if WITH_RPC_REGISTRY
	/**
	* Basic function that converts any request into a proper Json body.
	*/
	UE_API TSharedPtr<FJsonObject> GetJsonObjectFromRequestBody(TArray<uint8> InRequestBody);

	UE_API virtual void DeregisterHttpCallbacks() override;


// These are RPCs that should always be enabled, no matter what state the game is in. They 

	UE_API virtual void RegisterAlwaysOnHttpCallbacks() override;
	/** 
	* This is an example RPC that shows how to interact with request bodies and gather passed in values for use in game code.
	*/
	UE_API bool HttpExecuteCheatCommand(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);


// These are RPCs that should only be enabled while we are in the frontend.

	UE_API virtual void RegisterFrontendHttpCallbacks();
	//bool HttpSetMatchType(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

// These are RPCs that should only be enabled while we are in a match

	UE_API virtual void RegisterInMatchHttpCallbacks();
	/**
	 * This is an example RPC that shows how you can use requests to cause in-game input.
	 */
	UE_API bool HttpFireOnceCommand(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	/**
	 * This is an example RPC that shows how you can put together a response for consumption.
	 */
	UE_API bool HttpGetPlayerVitalsCommand(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

#endif

};

#undef UE_API
