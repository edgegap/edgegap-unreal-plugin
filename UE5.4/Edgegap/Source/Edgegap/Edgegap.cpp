#include "Edgegap.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "APIToken/APITokenSettings.h"
#include "APIToken/APITokenSettingsCustomization.h"
#include "UObject/Package.h"
#include "Features/IModularFeatures.h"
	
IMPLEMENT_MODULE(Edgegap, Edgegap);

void Edgegap::StartupModule()
{
	RegisterSettings();

	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(FAPITokenSettings::StaticStruct()->GetFName(), FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAPITokenSettingsCustomization::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UEdgegapSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FEdgegapSettingsDetails::MakeInstance));

    EdgegapPluginCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);

    PluginCommands->MapAction(
        EdgegapPluginCommands::Get().BuildAndPushCommand,
        FExecuteAction::CreateRaw(this, &Edgegap::Do_BuildAndPush),
        FCanExecuteAction::CreateRaw(this, &Edgegap::Can_BuildAndPush));

    PluginCommands->MapAction(
        EdgegapPluginCommands::Get().SettingsCommand,
        FExecuteAction::CreateRaw(this, &Edgegap::Do_OpenSettings),
        FCanExecuteAction::CreateRaw(this, &Edgegap::Can_OpenSettings));

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &Edgegap::RegisterMenus));

#if WITH_EDITOR
        UEdgegapSettings::OnSettingsChange.AddRaw(this, &Edgegap::OnEdgegapSettingsChanged);
#endif

    if (!StyleSet.IsValid())
    {
        const FVector2D Icon16x16(16.0f, 16.0f);
        const FVector2D Icon64x64(64.0f, 64.0f);

        StyleSet = MakeShareable(new FSlateStyleSet("EdgegapPluginToolsStyle"));

        StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
        StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

        StyleSet->Set("EdgegapIcon.Icon64", new IMAGE_BRUSH("icon128", Icon64x64));
        StyleSet->Set("EdgegapPluginTools.BuildAndPushCommand", new IMAGE_BRUSH("build_and_upload_64", Icon16x16));
        StyleSet->Set("EdgegapPluginTools.SettingsCommand", new IMAGE_BRUSH("settings_64", Icon16x16));

        FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
    }
}

void Edgegap::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();

		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FAPITokenSettings::StaticStruct()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UEdgegapSettings::StaticClass()->GetFName());

        if (StyleSet.IsValid())
        {
            FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
        }

#if WITH_EDITOR
        UEdgegapSettings::OnSettingsChange.RemoveAll(this);
#endif
	}
}

TSharedRef<SWidget> Edgegap::MakeEdgegapMenu()
{
    static const FName MenuName("UnrealEd.EdgegapPluginCommands.EdgegapMenu");

    if (!UToolMenus::Get()->IsMenuRegistered(MenuName))
    {
        UToolMenu* Menu = UToolMenus::Get()->RegisterMenu(MenuName);

        Menu->AddSection("EdgegapWarning", {}, FToolMenuInsert("Insert", EToolMenuInsertType::First));

        FToolMenuSection& MainSection = Menu->AddSection("Edgegap Tools", LOCTEXT("EdgegapTools", "Edgegap Tools"));
        MainSection.AddMenuEntryWithCommandList(
            EdgegapPluginCommands::Get().BuildAndPushCommand,
            PluginCommands);
        MainSection.AddMenuEntryWithCommandList(
            EdgegapPluginCommands::Get().SettingsCommand,
            PluginCommands);
    }

    FToolMenuContext MenuContext;
    return UToolMenus::Get()->GenerateWidget(MenuName, MenuContext);
}

void Edgegap::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);

    FUIAction PlatformMenuShownDelegate;
    PlatformMenuShownDelegate.IsActionVisibleDelegate = FIsActionButtonVisible::CreateLambda(
        []() {
            return true;
        });

    UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
    {
        FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
        {
            FToolMenuEntry Entry = FToolMenuEntry::InitComboButton(
                "EdgegapMenu",
                PlatformMenuShownDelegate,
                FOnGetContent::CreateLambda(
                    [this] {
                        return MakeEdgegapMenu();
                    }),
                        LOCTEXT("EdgegapMenu", "Edgegap"),
                        LOCTEXT("EdgegapMenu_Tooltip", "Edgegap Plugin Tools"),
                        FSlateIcon("EdgegapPluginToolsStyle", "EdgegapIcon.Icon64"),
                        false,
                        "EdgegapMenu");
            Entry.StyleNameOverride = "CalloutToolbar";

            Section.AddEntry(Entry);
        }
    }
}

void Edgegap::OnEdgegapSettingsChanged(UEdgegapSettings const* InSettings)
{
    // Update bCanBuildAndPush
    bCanBuildAndPush = !InSettings->Registry.IsEmpty() && !InSettings->ImageRepository.IsEmpty() && !InSettings->PrivateRegistryToken.IsEmpty() && !InSettings->PrivateRegistryUsername.IsEmpty();

    // Update bCanOpenSettings
    bCanOpenSettings = true;
}

void EdgegapPluginCommands::RegisterCommands()
{
    UI_COMMAND(
        BuildAndPushCommand,
        "Build and Push",
        "Builds and Pushes the game server.",
        EUserInterfaceActionType::Button,
        FInputChord());

    UI_COMMAND(
        SettingsCommand,
        "Settings...",
        "Edgagap Settings",
        EUserInterfaceActionType::Button,
        FInputGesture());
}

void Edgegap::Do_BuildAndPush()
{
    FEdgegapSettingsDetails::PackageProject("linux");
}

bool Edgegap::Can_BuildAndPush()
{
    return bCanBuildAndPush;
}

void Edgegap::Do_OpenSettings()
{
    FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer(FName("Project"), FName("Plugins"), FName("Edgegap"));
}

bool Edgegap::Can_OpenSettings()
{
    return bCanOpenSettings;
}