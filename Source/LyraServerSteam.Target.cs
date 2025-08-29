// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraServerSteamTarget : LyraServerTarget
{
	public LyraServerSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
	}
}
