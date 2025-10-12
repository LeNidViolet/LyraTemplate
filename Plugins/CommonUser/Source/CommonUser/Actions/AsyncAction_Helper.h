#pragma once

#include "AsyncAction_Types.h"

class AsyncAction_Helper
{
public:
	static bool UpdateMemberAttributes(
		UObject* WorldContextObject,
		int32 LocalUserNum,
		const FName& SessionName,
		const TMap<FName, FEIKAttribute>& MemberSettings
		);

	static FString GetProductUserID(FUniqueNetIdPtr& UniqueNetId);
	static FString GetEpicID(FUniqueNetIdPtr& UniqueNetId);
};
