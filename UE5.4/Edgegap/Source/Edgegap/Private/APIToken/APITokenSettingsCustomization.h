#pragma once

#include "IPropertyTypeCustomization.h"

class FAPITokenSettingsCustomization;

class EDGEGAP_API FAPITokenSettingsCustomization : public IPropertyTypeCustomization
{
	FAPITokenSettingsCustomization();

public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

};