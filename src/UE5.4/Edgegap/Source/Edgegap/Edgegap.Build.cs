using UnrealBuildTool;
using System.IO;



public class Edgegap : ModuleRules
{
	
	private string ModulePath
	{

        get
        {
            return System.IO.Path.GetFullPath(System.IO.Path.Combine(ModuleDirectory, "../../"));
        }
	}


	
	public Edgegap(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "ApplicationCore",
                "Engine",
                "EngineSettings",
                "InputCore",
                "RHI",
                "RenderCore",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "SourceControl",
                "SourceControlWindows",
                "TargetPlatform",
                "DesktopPlatform",
                "UnrealEd",
                "WorkspaceMenuStructure",
                "MessageLog",
                "UATHelper",
                "TranslationEditor",
                "Projects",
                "DeviceProfileEditor",
                "UndoHistory",
                "Analytics",
                "ToolMenus",
                "LauncherServices",
                "DerivedDataCache",
                "HTTP",
                "Json",
                "JsonUtilities",
                "ImageWrapper",
                "SharedSettingsWidgets",
                "ApplicationCore",
                "DeveloperToolSettings",
                "EngineSettings",
                "InputCore",
                "RHI",
                "RenderCore",
                "Slate",
                "SlateCore",
                "TargetPlatform",
                "DesktopPlatform",
                "WorkspaceMenuStructure",
                "MessageLog",
                 "Projects",
                 "ToolMenus",
                 "LauncherServices",
                "SourceControl",
                "EditorStyle",
                "TurnkeyIO",
                "UnrealEd",
                "UATHelper",
                "SettingsEditor",
                "Zen",
                "DeveloperSettings",
                "DesktopWidgets"
            }
        );

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "DesktopPlatform",
                "GameProjectGeneration",
                "ProjectTargetPlatformEditor",
                "LevelEditor",
                "Settings",
                "SourceCodeAccess",
                "LocalizationDashboard",
                "UCMDHelper",
                "Projects"
            }
        );

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                    "AssetTools",
                    "DesktopPlatform",
                    "GameProjectGeneration",
                    "ProjectTargetPlatformEditor",
                    "LevelEditor",
                    "Settings",
                    "SourceCodeAccess",
                    "LocalizationDashboard",
                     "MainFrame",
            }
        );

        System.Console.WriteLine("ModulePath here: ");
        System.Console.WriteLine(ModulePath);
        PublicDefinitions.Add("EDGEGAP_MODULE_PATH=\""+ ModulePath.Replace("\\", "/")+"\"");
    }
}
