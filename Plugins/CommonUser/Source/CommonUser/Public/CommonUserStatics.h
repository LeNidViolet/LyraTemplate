#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonUserStatics.generated.h"

UCLASS()
class UCommonUserStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="CommonUser")
	static COMMONUSER_API bool UniqueNetIdRepl_Equal(const FUniqueNetIdRepl& A, const FUniqueNetIdRepl& B)
	{
		if (!A.IsValid() || !B.IsValid())
		{
			return false;
		}
		return *A == *B;
	}

	UFUNCTION(BlueprintPure, Category="CommonUser")
	static COMMONUSER_API FString UniqueNetIdRepl_ToString(const FUniqueNetIdRepl& ID)
	{
		if (ID.IsValid())
		{
			return ID->ToString();
		}
		return TEXT("Invalid");
	}
};