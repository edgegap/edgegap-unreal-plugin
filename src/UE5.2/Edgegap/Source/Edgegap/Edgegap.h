#pragma once

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "Engine/EngineTypes.h"
#include "EdgegapSettingsDetails.h"
#include "EdgegapSettings.h"
#include "EditorStyleSet.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "EdgegapModule"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Content(RelativePath, ".png"), __VA_ARGS__)
static FString Content(FString RelativePath, FString Extension)
{
	static FString BaseDir = FString();

	if (BaseDir.IsEmpty())
	{
		auto Plugin = IPluginManager::Get().FindPlugin("Edgegap");

		if (Plugin.IsValid())
		{
			BaseDir = Plugin->GetBaseDir();
		}
		else
		{
			return FString();
		}
	}

	return (BaseDir / "Resources" / RelativePath) + TCHAR_TO_ANSI(*Extension);
}

class Edgegap : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline Edgegap& Get()
	{
		return FModuleManager::LoadModuleChecked<Edgegap>("Edgegap");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Edgegap");
	}

	virtual bool IsGameModule() const override
	{
		return true;
	}


	void RegisterSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "Edgegap",
				LOCTEXT("RuntimeSettingsName", "Edgegap"),
				LOCTEXT("RuntimeSettingsDescription", "Deploy into Edgegap"),
				GetMutableDefault<UEdgegapSettings>());
		}
	}

	void UnregisterSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "Edgegap");
		}
	}

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	void RegisterMenus();

	void Do_BuildAndPush();
	bool Can_BuildAndPush();
	void Do_OpenSettings();
	bool Can_OpenSettings();

	bool bCanBuildAndPush = true;
	bool bCanOpenSettings = true;

	TSharedRef<SWidget> MakeEdgegapMenu();

	class TSharedPtr<FSlateStyleSet> StyleSet;

private:
	void OnEdgegapSettingsChanged(UEdgegapSettings const* InSettings);
};

class EdgegapPluginCommands : public TCommands<EdgegapPluginCommands>
{
public:
	EdgegapPluginCommands()
		: TCommands<EdgegapPluginCommands>("EdgegapPluginTools", LOCTEXT("EdgegapPluginTools", "Edgegap Plugin Tools"), NAME_None, "EdgegapPluginToolsStyle")
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> BuildAndPushCommand;
	TSharedPtr<FUICommandInfo> SettingsCommand;
};
