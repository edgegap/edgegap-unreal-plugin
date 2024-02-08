#include "Edgegap.h"
#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"
#include "Features/IModularFeatures.h"
	
IMPLEMENT_MODULE(Edgegap, Edgegap);

#define LOCTEXT_NAMESPACE "Edgegap"


void Edgegap::StartupModule()
{
	RegisterSettings();

	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UEdgegapSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FEdgegapSettingsDetails::MakeInstance));
}

void Edgegap::ShutdownModule()
{
	if (UObjectInitialized())
	{
		UnregisterSettings();

		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UEdgegapSettings::StaticClass()->GetFName());
	}
}
