// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraGameSteamTarget : LyraGameTarget
{
	public LyraGameSteamTarget(TargetInfo Target) : base(Target)
	{
		CustomConfig = "Steam";
	}
}
