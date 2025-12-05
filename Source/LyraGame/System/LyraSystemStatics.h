// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"
#include "CommonSessionSubsystem.h"
#include "GameModes/LyraUserFacingExperienceDefinition.h"
#include "LyraSystemStatics.generated.h"

template <typename T> class TSubclassOf;

class AActor;
class UActorComponent;
class UObject;
struct FFrame;

UCLASS()
class ULyraSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Returns the soft object reference associated with a Primary Asset Id, this works even if the asset is not loaded */
	UFUNCTION(BlueprintPure, Category = "AssetManager", meta=(DeterminesOutputType=ExpectedAssetType))
	static TSoftObjectPtr<UObject> GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType);

	UFUNCTION(BlueprintCallable)
	static FPrimaryAssetId GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Lyra", meta = (WorldContext = "WorldContextObject"))
	static void PlayNextGame(const UObject* WorldContextObject);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors = true);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors = true);

	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors = true);

	// Gets all the components that inherit from the given class
	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DefaultToSelf="TargetActor", ComponentClass="/Script/Engine.ActorComponent", DeterminesOutputType="ComponentClass"))
	static TArray<UActorComponent*> FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors = true);

	// Transferred to the world represented by the FacingExperience object.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Experience", meta = (WorldContext = "WorldContextObject"))
	static bool TravelExperience(const UObject* WorldContextObject, ULyraUserFacingExperienceDefinition* FacingExperience,	TMap<FString, FString> ExtraArgs, bool bAbsolute = true, ECommonSessionOnlineMode OnlineMode=ECommonSessionOnlineMode::Offline);

	// Transferred to the world represented by the FacingExperience name.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Experience", meta = (WorldContext = "WorldContextObject"))
	static bool TravelExperienceWithName(const UObject* WorldContextObject, FName FacingExperience, TMap<FString, FString> ExtraArgs, bool bAbsolute = true, ECommonSessionOnlineMode OnlineMode=ECommonSessionOnlineMode::Offline);


	/**
	 * 在 Actor 前方扇形区域寻找一个合法的放置点
	 * @param OriginActor 参考 Actor (提供位置和朝向)
	 * @param MinRadius 最小距离 (避免生成在脚下)
	 * @param MaxRadius 最大距离 (扇形纵深)
	 * @param ConeHalfAngle 扇形半角 (度数, e.g. 45度)
	 * @param OutLocation [Out] 找到的位置
	 * @return 是否成功找到
	 */
	UFUNCTION(BlueprintCallable, Category="Lyra|Gameplay")
	static bool FindValidSpawnLocationInCone(
		FVector& OutLocation,
		AActor* OriginActor,
		float MinRadius=50.f,
		float MaxRadius=200.f,
		float ConeHalfAngle=60.f);


	static float SampleCurveValue(const UCurveFloat* Curve, float X, float DefaultValue = 0.0f);

	/**
	 * 使用曲线计算丢弃数量
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static int32 CalculateDropCount(int32 CurrentStackCount, UCurveFloat* DropCurve);
};
