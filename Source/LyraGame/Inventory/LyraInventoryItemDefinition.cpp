// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraInventoryItemDefinition.h"

#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryItemDefinition)

//////////////////////////////////////////////////////////////////////
// ULyraInventoryItemDefinition

ULyraInventoryItemDefinition::ULyraInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const ULyraInventoryItemFragment* ULyraInventoryItemDefinition::FindFragmentByClass(TSubclassOf<ULyraInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (ULyraInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

void ULyraInventoryItemDefinition::PostInitProperties()
{
	// 进行一些必要的数据修正

	if (!bAllowStacking) MaxStackCount = 1;
	else if (MaxStackCount < 1) MaxStackCount = 1;

	Super::PostInitProperties();
}
