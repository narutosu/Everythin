// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EverythinTarget : TargetRules
{
	public EverythinTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "Everythin"  ,"TestModule"} );
		// if(Configuration==UnrealTargetConfiguration.Shipping)
        // {
		// 	BuildEnvironment = TargetBuildEnvironment.Unique;
		// 	bUseChecksInShipping = true;
		// 	bUseLoggingInShipping = true;
        // }
	}
}
