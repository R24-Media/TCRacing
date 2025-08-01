// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Editor/PropertyEditor/Public/IPropertyTypeCustomization.h"

/**
 * 
 */
class BLACK_EYE_EDITOR_API FBlackEyeTargetRowCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
    static const struct FSlateBrush* GetComponentIcon(const class UActorComponent* ActorComponent);
    static FText OnGetComponentName(const class UActorComponent* ActorComponent);

    static TSharedRef<class SWidget> OnGetComponentContent(TSharedRef<class IPropertyHandle> PropertyHandle);
};
