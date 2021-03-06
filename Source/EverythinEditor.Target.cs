// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class EverythinEditorTarget : TargetRules
{
	public EverythinEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "Everythin" ,"TestModule"} );
        // if (Configuration == UnrealTargetConfiguration.Shipping)
        // {
        //     BuildEnvironment = TargetBuildEnvironment.Unique;
        //     bUseChecksInShipping = true;
        //     bUseLoggingInShipping = true;
        // }
    }
}
