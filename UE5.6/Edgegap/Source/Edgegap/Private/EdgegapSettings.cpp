#include "EdgegapSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Misc/ConfigCacheIni.h"

UEdgegapSettings::UEdgegapSettings() {}

#if WITH_EDITOR

UEdgegapSettings::FOnUpdateSettings UEdgegapSettings::OnSettingsChange;

void UEdgegapSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	OnSettingsChange.Broadcast(this);

	if (PropertyChangedEvent.Property == nullptr)
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName() == TEXT("APIToken"))
	{
		bIsTokenVerified = false;
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif // WITH_EDITOR
