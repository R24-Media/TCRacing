// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.
#include "BlackEyeTargetRowCustomization.h"

#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/IDetailChildrenBuilder.h"
#include "Editor/PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "Editor/PropertyEditor/Private/UserInterface/PropertyEditor/SPropertyEditorInteractiveActorPicker.h"

#include "Editor.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"

#include "Kismet2/ComponentEditorUtils.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateIconFinder.h"

#include "Utility/BlackEyeSimpleTarget.h"
#include "Utility/BlackEyeTarget.h"

#define LOCTEXT_NAMESPACE "Black_Eye_EditorModule"

TSharedRef<IPropertyTypeCustomization> FBlackEyeTargetRowCustomization::MakeInstance()
{
    return MakeShareable(new FBlackEyeTargetRowCustomization());
}

void FBlackEyeTargetRowCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    HeaderRow
        .NameContent()
        [
            PropertyHandle->CreatePropertyNameWidget()
        ]
        .ValueContent()
        [
            PropertyHandle->CreatePropertyValueWidget()
        ];
}

void FBlackEyeTargetRowCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
    uint32 NumChildren;
    PropertyHandle->GetNumChildren(NumChildren);

    for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
    {
        TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

        FName ChildName = ChildHandle->GetProperty()->GetFName();
        if (ChildName == GET_MEMBER_NAME_STRING_CHECKED(FBlackEyeSimpleTarget, Target)
            || ChildName == GET_MEMBER_NAME_STRING_CHECKED(FBlackEyeTarget, Target))
        {
            bool CanUseEyeDropper = !GEditor->IsPlayingSessionInEditor();

            FText ActorRowOverride = FText::FromString("Actor");
            FText ComponentRowOverride = FText::FromString("Actor Component");

            TSharedRef<SWidget> ActorSelectWidget = SNew(SObjectPropertyEntryBox)
                .AllowedClass(AActor::StaticClass())
                .DisplayThumbnail(true)
                .DisplayUseSelected(CanUseEyeDropper)
                .ObjectPath_Lambda([ChildHandle]() {
                    void* TargetPtr;
                    ChildHandle->GetValueData(TargetPtr);
                    FSoftComponentReference* Reference = ((FSoftComponentReference*)TargetPtr);

                    UActorComponent* ComponentRef = Reference->GetComponent(nullptr);
                    AActor* ActorRef = ComponentRef ? ComponentRef->GetOwner() : Reference->OtherActor.Get();

                    FSoftObjectPath Path(ActorRef);

                    return Path.ToString(); 
                })

                .OnObjectChanged_Lambda([ChildHandle](const FAssetData& data) {
                    void* TargetPtr;
                    ChildHandle->GetValueData(TargetPtr);

                    ChildHandle->NotifyPreChange();

                    FSoftComponentReference* Reference = ((FSoftComponentReference*)TargetPtr);

                    UObject* Asset = data.GetAsset();
                    if (AActor* Actor = Cast<AActor>(Asset))
                    {
                        Reference->OtherActor = Actor;
                        Reference->ComponentProperty = NAME_None;
                    }
                    else
                    {
                        Reference->OtherActor = nullptr;
                        Reference->ComponentProperty = NAME_None;
                    }

                    ChildHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
                    ChildHandle->NotifyFinishedChangingProperties();
                });

            ChildBuilder.AddCustomRow(ActorRowOverride)
                .NameContent()[
                    ChildHandle->CreatePropertyNameWidget(ActorRowOverride)
                ]
                .ValueContent()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Center)[
                        ActorSelectWidget
                    ];

            if (!CanUseEyeDropper)
            {
                // HACK: This is SUUUPER fragile and based on the explicit layout  of the SPropertyEditorInteractiveActorPicker we chose above
                // future versions of UE could change this and utterly break it.
                auto DoTheThing = [](const TSharedRef<SWidget>& Widget, int Depth) {
                        if (Widget->GetWidgetClass().GetWidgetType() == SPropertyEditorInteractiveActorPicker::StaticWidgetClass().GetWidgetType())
                        {
                            FChildren* Children = Widget->GetChildren();
                            if (Children->Num() == 1 && Depth == 10
                                && Children->GetChildAt(0)->GetWidgetClass().GetWidgetType() == SImage::StaticWidgetClass().GetWidgetType())
                            {
                                Widget->SetEnabled(false);
                                TSharedRef<SWidget> ImageWidget = Children->GetChildAt(0);
                                SImage* Image = (SImage*)(&ImageWidget.Get());
                                
                            }
                        }

                        return false;
                    };

                TFunction<bool(const TSharedRef<SWidget>& widget, int)> IterateChildren;
                IterateChildren = [DoTheThing, &IterateChildren](const TSharedRef<SWidget>& widget, int depth)
                    {
                        if (DoTheThing(widget, depth)) return true;

                        FChildren* Children = widget->GetChildren();
                        if (ensure(Children))
                        {
                            for (int32 i = 0; i < Children->Num(); ++i)
                            {
                                TSharedRef<SWidget> Child = Children->GetChildAt(i);
                                if (IterateChildren(Child, depth + 1)) return true;
                            }
                        }

                        return false;
                    };

                IterateChildren(ActorSelectWidget, 0);
            }

            TSharedRef<SVerticalBox> ObjectContent = SNew(SVerticalBox);
            ObjectContent->AddSlot() [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center) [
                            SNew(SImage)
                                .Image_Lambda([ChildHandle]() {
                                    void* TargetPtr;
                                    ChildHandle->GetValueData(TargetPtr);

                                    FSoftComponentReference* Reference = ((FSoftComponentReference*)TargetPtr);
                                    return GetComponentIcon(Reference->GetComponent(nullptr));
                                })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(1)
                        .VAlign(VAlign_Center) [
                            // Show the name of the asset or actor
                            SNew(STextBlock)
                                .Font(IDetailLayoutBuilder::GetDetailFont())
                                .Text_Lambda([ChildHandle]() {
                                        void* TargetPtr;
                                        ChildHandle->GetValueData(TargetPtr);

                                        FSoftComponentReference* Reference = ((FSoftComponentReference*)TargetPtr);
                                        return OnGetComponentName(Reference->GetComponent(nullptr));
                                    })
                        ]
                ];

            TSharedRef<SHorizontalBox> ComboButtonContent = SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(1)
                .VAlign(VAlign_Center) [
                    ObjectContent
                ];

            ChildBuilder.AddCustomRow(ComponentRowOverride).NameContent()[
                ChildHandle->CreatePropertyNameWidget(ComponentRowOverride)
            ].ValueContent()[
                SNew(SComboButton)
                    .HAlign(HAlign_Fill)
                    .ButtonStyle(FAppStyle::Get(), "PropertyEditor.AssetComboStyle")
                    .ForegroundColor(FAppStyle::GetColor("PropertyEditor.AssetName.ColorAndOpacity"))
                    .OnGetMenuContent_Lambda([ChildHandle]() {
                            return FBlackEyeTargetRowCustomization::OnGetComponentContent(ChildHandle);
                        })
                    .IsEnabled(true)
                    .ContentPadding(2.0f)
                    .ButtonContent() [
                        ComboButtonContent
                    ]
            ];
            continue;
        }
        ChildBuilder.AddProperty(ChildHandle);
    }
}

TSharedRef<SWidget> FBlackEyeTargetRowCustomization::OnGetComponentContent(TSharedRef<class IPropertyHandle> PropertyHandle)
{
    void* TargetPtr;
    PropertyHandle->GetValueData(TargetPtr);

    FSoftComponentReference* Reference = ((FSoftComponentReference*)TargetPtr);
    UActorComponent* componentRef = Reference->GetComponent(nullptr);
    TSoftObjectPtr<AActor> PropertyActor = componentRef ? componentRef->GetOwner() : Reference->OtherActor;

    return PropertyCustomizationHelpers::MakeComponentPickerWithMenu(Reference->GetComponent(nullptr),
        false,
        FOnShouldFilterActor::CreateLambda([PropertyActor](const AActor* const Actor) {
            return Actor == PropertyActor;
            }),
        FOnShouldFilterComponent::CreateLambda([](const UActorComponent* Comp) {
            return !Comp->IsEditorOnly();
            }),
        FOnComponentSelected::CreateLambda([Reference, PropertyHandle](const UActorComponent* InComponent) {
            PropertyHandle->NotifyPreChange();

            void* PropertyPtr;
            PropertyHandle->GetValueData(PropertyPtr);
            FSoftComponentReference* PropertyReference = ((FSoftComponentReference*)PropertyPtr);

            if (InComponent)
            {
                FComponentReference NewRef = FComponentEditorUtils::MakeComponentReference(InComponent->GetOwner(), InComponent);

                PropertyReference->ComponentProperty = NewRef.ComponentProperty;
                PropertyReference->PathToComponent = NewRef.PathToComponent;
                PropertyReference->OtherActor = InComponent->GetOwner();

            }
            else
            {
                PropertyReference->ComponentProperty = NAME_None;
                PropertyReference->PathToComponent.Empty();
            }

            PropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
            PropertyHandle->NotifyFinishedChangingProperties();

            }),
        FSimpleDelegate::CreateLambda([]() {}));
}

const FSlateBrush* FBlackEyeTargetRowCustomization::GetComponentIcon(const UActorComponent* ActorComponent)
{
    if (ActorComponent)
    {
        return FSlateIconFinder::FindIconBrushForClass(ActorComponent->GetClass());
    }
    return FSlateIconFinder::FindIconBrushForClass(UActorComponent::StaticClass());
}

FText FBlackEyeTargetRowCustomization::OnGetComponentName(const UActorComponent* ActorComponent)
{
    if (ActorComponent)
    {
        const FName ComponentName = FComponentEditorUtils::FindVariableNameGivenComponentInstance(ActorComponent);
        const bool bIsArrayVariable = !ComponentName.IsNone() && ActorComponent->GetOwner() != nullptr && FindFProperty<FArrayProperty>(ActorComponent->GetOwner()->GetClass(), ComponentName);

        if (!ComponentName.IsNone() && !bIsArrayVariable)
        {
            return FText::FromName(ComponentName);
        }
        return FText::AsCultureInvariant(ActorComponent->GetName());
    }
    return LOCTEXT("NoComponent", "None");
}

#undef LOCTEXT_NAMESPACE
