#include "EdgegapSettings.h"
#include "Settings/ProjectPackagingSettings.h"
#include "Misc/ConfigCacheIni.h"

UEdgegapSettings::UEdgegapSettings()
{
	// Migrate any settings from the old ini files if they exist
	UProjectPackagingSettings* ProjectPackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
	if (ProjectPackagingSettings)
	{
		//bEncryptPakIniFiles = ProjectPackagingSettings->bEncryptIniFiles_DEPRECATED;
		//bEncryptPakIndex = ProjectPackagingSettings->bEncryptPakIndex_DEPRECATED;
	}
}

void UEdgegapSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetName() == TEXT("SecondaryEncryptionKeys"))
	{
		//if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd)
		//{
		//	int32 Index = PropertyChangedEvent.GetArrayIndex(TEXT("SecondaryEncryptionKeys"));
		//	CryptoKeysHelpers::GenerateEncryptionKey(SecondaryEncryptionKeys[Index].Key);

		//	int32 Number = 1;
		//	FString NewName = FString::Printf(TEXT("New Encryption Key %d"), Number++);
		//	while (SecondaryEncryptionKeys.FindByPredicate([NewName](const FCryptoEncryptionKey& Key) { return Key.Name == NewName; }) != nullptr)
		//	{
		//		NewName = FString::Printf(TEXT("New Encryption Key %d"), Number++);
		//	}

		//	SecondaryEncryptionKeys[Index].Name = NewName;
		//	SecondaryEncryptionKeys[Index].Guid = FGuid::NewGuid();
		//}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}