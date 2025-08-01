// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Editor/PropertyEditor/Public/IDetailCustomization.h"

/**
 * 
 */
class BLACK_EYE_EDITOR_API FBlackEyeLookAtCustomization : public IDetailCustomization
{
public: 
    /** Makes a new instance of this detail layout class for a specific detail view requesting it */ 
    static TSharedRef<IDetailCustomization> MakeInstance();
    /** IDetailCustomization interface */ 
    virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;
};
