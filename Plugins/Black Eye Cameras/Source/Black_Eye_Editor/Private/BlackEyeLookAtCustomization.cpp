// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.


#include "BlackEyeLookAtCustomization.h"

#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/IDetailGroup.h"
#include "Editor/PropertyEditor/Public/IDetailChildrenBuilder.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "Editor/PropertyEditor/Private/UserInterface/PropertyEditor/SPropertyEditorInteractiveActorPicker.h"

#include "Kismet2/ComponentEditorUtils.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateIconFinder.h"

#include "Components/LookAtComponent.h"
#include "Utility/BlackEyeWeightedTarget.h"

#define LOCTEXT_NAMESPACE "Black_Eye_EditorModule"

/** Makes a new instance of this detail layout class for a specific detail view requesting it */
TSharedRef<IDetailCustomization> FBlackEyeLookAtCustomization::MakeInstance() { return MakeShareable(new FBlackEyeLookAtCustomization()); }

/** IDetailCustomization interface */
void FBlackEyeLookAtCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    FText CategoryLocString = FText::FromString(TEXT("Composition"));
    FText LookCategoryLocString = FText::FromString(TEXT("Look"));

    IDetailCategoryBuilder& CompositionCategory = DetailBuilder.EditCategory("Composition", CategoryLocString, ECategoryPriority::Default);
    IDetailCategoryBuilder& LookCategory = DetailBuilder.EditCategory("Look", LookCategoryLocString, ECategoryPriority::Important);

    FText PresetText = FText::FromString(TEXT("Presets"));
    FText PresetRowName = FText::FromString(TEXT("Composition Preset (Rule of 3rd)"));

    FText TopLeftText = FText::FromString(TEXT("Top Left"));
    FText TopRightText = FText::FromString(TEXT("Top Right"));
    FText CenterText = FText::FromString(TEXT("Center"));
    FText CottomLeftText = FText::FromString(TEXT("Bottom Left"));
    FText BottomRightText = FText::FromString(TEXT("Bottom Right"));

    TSharedRef<IPropertyHandle> TargetScreenPositionProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_STRING_CHECKED(ULookAtComponent, ScreenPosition));

    IDetailGroup& PresetGroup = CompositionCategory.AddGroup(TEXT("Composition Presets"), PresetRowName);
    PresetGroup.AddWidgetRow()
        .NameContent() [ 
            SNew(STextBlock) 
                .Text(FText::GetEmpty())
                .Font(IDetailLayoutBuilder::GetDetailFont())
        ] 
        .ValueContent()
        .MinDesiredWidth(500) [ 
            SNew(SVerticalBox)
                + SVerticalBox::Slot()[
                    SNew(SVerticalBox)
                ]
                + SVerticalBox::Slot()
                .AutoHeight() [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(0.5f)
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        .VAlign(VAlign_Center)[
                            SNew(SButton)
                                .HAlign(HAlign_Center)
                                .Text(TopLeftText)
                                .OnClicked_Lambda([TargetScreenPositionProperty]() {
                                        TargetScreenPositionProperty.Get().SetValue(ULookAtComponent::GetLookAtViewportPosition(EViewLookPosition::VLP_TopLeft), EPropertyValueSetFlags::InteractiveChange);
                                        return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        .VAlign(VAlign_Center)[
                            SNew(SButton)
                                .HAlign(HAlign_Center)
                                .Text(TopRightText)
                                .OnClicked_Lambda([TargetScreenPositionProperty]() {
                                        TargetScreenPositionProperty.Get().SetValue(ULookAtComponent::GetLookAtViewportPosition(EViewLookPosition::VLP_TopRight), EPropertyValueSetFlags::InteractiveChange);
                                        return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(0.5f)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()[
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        + SHorizontalBox::Slot()
                        .FillWidth(0.5f)
                        .VAlign(VAlign_Center)[
                            SNew(SButton)
                                .HAlign(HAlign_Center)
                                .Text(CenterText)
                                .OnClicked_Lambda([TargetScreenPositionProperty]() {
                                        TargetScreenPositionProperty.Get().SetValue(ULookAtComponent::GetLookAtViewportPosition(EViewLookPosition::VLP_Center), EPropertyValueSetFlags::InteractiveChange);
                                        return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()[
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(0.5f)
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        .VAlign(VAlign_Center)[
                            SNew(SButton)
                                .HAlign(HAlign_Center)
                                .Text(CottomLeftText)
                                .OnClicked_Lambda([TargetScreenPositionProperty]() {
                                        TargetScreenPositionProperty.Get().SetValue(ULookAtComponent::GetLookAtViewportPosition(EViewLookPosition::VLP_BottomLeft), EPropertyValueSetFlags::InteractiveChange);
                                        return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        .VAlign(VAlign_Center)[
                            SNew(SButton)
                                .HAlign(HAlign_Center)
                                .Text(BottomRightText)
                                .OnClicked_Lambda([TargetScreenPositionProperty]() {
                                        TargetScreenPositionProperty.Get().SetValue(ULookAtComponent::GetLookAtViewportPosition(EViewLookPosition::VLP_BottomRight), EPropertyValueSetFlags::InteractiveChange);
                                        return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(0.5f)
                ]
                + SVerticalBox::Slot()
                [
                    SNew(SVerticalBox)
                ]
        ]; 
}

#undef LOCTEXT_NAMESPACE
