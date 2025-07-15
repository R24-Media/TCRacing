// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "BlackEyePersistChangesSubsystem.h"
#include "CoreMinimal.h"

#include "Editor.h"
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#include "Runtime/Engine/Public/EngineUtils.h"

#include "Actors/BlackEyeCameraRigBase.h"
#include "Components/LookAtComponent.h"

#define COMPONENT_POD_DATA_COPY(property, runtimeComponent, EditorComponent, Type) \
    if (F##Type##Property* __##Type##__ = CastField<F##Type##Property>(property))\
    {\
        UE_LOG(LogTemp, Display, TEXT("BlackEye SIP: Saved '" #Type "' in editor actor"));\
        __##Type##__->SetPropertyValue_InContainer(EditorComponent, __##Type##__->GetPropertyValue_InContainer(runtimeComponent));\
    }

#define COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(UScriptStruct, Type) UScriptStruct == TBaseStructure<Type>::Get()

static TSet<FName> kBlacklistedPropertyNames = {
    FName(TEXT("RelativeScale3D"))
};

static bool IsPropertyBlacklisted(const FName& MemberPropertyName)
{
    return kBlacklistedPropertyNames.Contains(MemberPropertyName);
}

void UBlackEyePersistChangesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    PIEActorToEditorActorMap = TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<AActor>>();

    BeginPIEDelegateHandle = FEditorDelegates::BeginPIE.AddUObject(this, &UBlackEyePersistChangesSubsystem::OnPIEChangedEvent);
    EndPIEDelegateHandle =  FEditorDelegates::EndPIE.AddUObject(this, &UBlackEyePersistChangesSubsystem::OnPIEChangedEvent);

    EditPropertyDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UBlackEyePersistChangesSubsystem::OnPostObjectEditProperty);
}

void UBlackEyePersistChangesSubsystem::Deinitialize()
{
    if (BeginPIEDelegateHandle.IsValid())
    {
        FEditorDelegates::BeginPIE.Remove(BeginPIEDelegateHandle);
        BeginPIEDelegateHandle.Reset();
    }

    if (EndPIEDelegateHandle.IsValid())
    {
        FEditorDelegates::EndPIE.Remove(EndPIEDelegateHandle);
        EndPIEDelegateHandle.Reset();
    }

    if (EditPropertyDelegateHandle.IsValid())
    {
        FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(EditPropertyDelegateHandle);
        EditPropertyDelegateHandle.Reset();
    }
}

void UBlackEyePersistChangesSubsystem::SetSaveInPlayState(bool Enabled)
{
    bSaveInPlayEnabled = Enabled;
}

AActor* UBlackEyePersistChangesSubsystem::TryGetMatchingEditorActor(AActor* PieActor, UWorld* EditorWorld)
{
    if (PieActor && EditorWorld)
    {
        if (PIEActorToEditorActorMap.Contains(PieActor))
        {
            TWeakObjectPtr<AActor> EditorActor = PIEActorToEditorActorMap[PieActor];

            if (EditorActor.IsValid())
            {
                return EditorActor.Get();
            }
            else
            {
                PIEActorToEditorActorMap.Remove(PieActor);
            }
        }

        FString ActorName = PieActor->GetName();
        for (TActorIterator<AActor> ActorItr(EditorWorld); ActorItr; ++ActorItr)
        {
            AActor* EditorActor = *ActorItr;
            if (EditorActor->GetName() == ActorName)
            {
                PIEActorToEditorActorMap.Add((TWeakObjectPtr<AActor>(PieActor), TWeakObjectPtr<AActor>(EditorActor)));
                return EditorActor;
            }
        }
    }
    return nullptr;
}

void UBlackEyePersistChangesSubsystem::CopyBlackEyeTargetProperty(FStructProperty* BETTargetProperty,
                                                                    const FBlackEyeSimpleTarget* SceneTarget,
                                                                    FBlackEyeSimpleTarget* EditorTarget,
                                                                    UWorld* EditorWorld)
{
    if (!BETTargetProperty) return;

    TObjectPtr<UScriptStruct> StructReflectionData = BETTargetProperty->Struct;
    if (COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(StructReflectionData, FBlackEyeTarget))
    {
        // Do specific copies for this sub class
        const FBlackEyeTarget* SceneTargetFull = (FBlackEyeTarget*)(SceneTarget);
        FBlackEyeTarget* EditorTargetFull = (FBlackEyeTarget*)(EditorTarget);

        EditorTargetFull->bAutoSize = SceneTargetFull->bAutoSize;
        EditorTargetFull->BoundingRadius = SceneTargetFull->BoundingRadius;
    }

    const FSoftComponentReference& SceneComponentRef = SceneTarget->Target;
    FSoftComponentReference& EditorComponentRef = EditorTarget->Target;

    AActor* EditorActor = TryGetMatchingEditorActor(SceneComponentRef.OtherActor.Get(), EditorWorld);

    EditorComponentRef.OtherActor = EditorActor;
    EditorComponentRef.ComponentProperty = SceneComponentRef.ComponentProperty;
    EditorComponentRef.PathToComponent = SceneComponentRef.PathToComponent;

    EditorTarget->BoneName = SceneTarget->BoneName;
}

void UBlackEyePersistChangesSubsystem::OnPIEChangedEvent(bool bIsSimulating)
{
    //Ensure no stale data leaves or enters this PIE session
    PIEActorToEditorActorMap.Reset();
}

void UBlackEyePersistChangesSubsystem::OnPostObjectEditProperty(UObject* ObjectBeingModified, FPropertyChangedEvent& InPropertyChangedEvent)
{
    // Only  SIP when it is enabled
    if (!IsSaveInPlayEnabled()) return;

    UActorComponent* ActorComponent = Cast<UActorComponent>(ObjectBeingModified);
    if (!ActorComponent) return;

    AActor* SceneActor = ActorComponent->GetOwner();
    // Only allow scene comps under a BET CameraRig to be a part of this
    if (ActorComponent && Cast<ABlackEyeCameraRigBase>(SceneActor))
    {
        // See if we want to persist changes in the editor world
        FName MemberPropertyName = InPropertyChangedEvent.GetMemberPropertyName();
        if (!IsPropertyBlacklisted(MemberPropertyName))
        {
            UWorld* World = ObjectBeingModified->GetWorld();
            bool IsPlayingSession = GEditor->IsPlayingSessionInEditor();
            bool IsInPIEWorld = World && World->IsGameWorld();
            if (IsPlayingSession && IsInPIEWorld)
            {
                FString ActorName = SceneActor->GetName();
                UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
                UE_LOG(LogTemp, Display, TEXT("(%s:%s): '%s' changed. Checking if it should be SIP."), *ActorName,
                                                *ActorComponent->GetName(),
                                                *MemberPropertyName.ToString());

                const AActor* EditorActor = TryGetMatchingEditorActor(SceneActor, EditorWorld);

                // TODO: Do we generalize SIP for any actor classes?
                if (Cast<ABlackEyeCameraRigBase>(EditorActor))
                {
                    // Find the component of matching name and class within the editor actor
                    UClass* SceneComponentClass = ActorComponent->GetClass();
                    FString SceneComponentName = ActorComponent->GetName();
                    const TSet<UActorComponent*>& EditorActorcomponents = EditorActor->GetComponents();
                    UActorComponent* EditorComponent = nullptr;

                    for (UActorComponent* EditorActorComponent : EditorActorcomponents)
                    {
                        if (EditorActorComponent->GetClass() == SceneComponentClass
                            && SceneComponentName == EditorActorComponent->GetName())
                        {
                            EditorComponent = EditorActorComponent;
                            break;
                        }
                    }

                    //Only allow for single object selection
                    if (EditorComponent && (InPropertyChangedEvent.ObjectIteratorIndex == -1 || InPropertyChangedEvent.ObjectIteratorIndex == 0))
                    {
                        FProperty* EditorComponentProperty = EditorComponent->GetClass()->FindPropertyByName(InPropertyChangedEvent.GetMemberPropertyName());
                            
                        UWorld* PrevWorld = GWorld;
                        bool PrevPlayInEditorWorld = GIsPlayInEditorWorld;
                        GIsPlayInEditorWorld = false;
                        GWorld = EditorWorld;
                        if (EditorComponentProperty)
                        {
                            if (FStructProperty* StructProp = CastField<FStructProperty>(EditorComponentProperty))
                            {
                                //White list only specific struct types for now
#if ENGINE_MINOR_VERSION < 3
                                UScriptStruct* StructReflectionData = StructProp->Struct;
#else
                                UScriptStruct* StructReflectionData = StructProp->Struct.Get();
#endif

                                if (COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(StructReflectionData, FVector)
                                    || COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(StructReflectionData, FVector2D))
                                {
                                    GEditor->BeginTransaction(FText::FromString("BlackEye Save In Play"));

                                    UE_LOG(LogTemp, Display, TEXT("BlackEye SIP: STRUCT saved in editor actor"));
                                    void* Container = StructProp->ContainerPtrToValuePtr<void>(ActorComponent);
                                    StructProp->SetValue_InContainer(EditorComponent, Container);

                                    EditorComponent->Modify(true);
                                    GEditor->EndTransaction();
                                }
                                else if (COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(StructReflectionData, FBlackEyeTarget)
                                    || COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(StructReflectionData, FBlackEyeSimpleTarget))
                                {
                                    GEditor->BeginTransaction(FText::FromString("BlackEye Save In Play"));

                                    const FBlackEyeSimpleTarget* SceneTarget = StructProp->ContainerPtrToValuePtr<FBlackEyeSimpleTarget>(ActorComponent);
                                    FBlackEyeSimpleTarget* EditorTarget = StructProp->ContainerPtrToValuePtr<FBlackEyeSimpleTarget>(EditorComponent);
                                    CopyBlackEyeTargetProperty(StructProp, SceneTarget, EditorTarget, EditorWorld);

                                    UE_LOG(LogTemp, Display, TEXT("BlackEye SIP: BlackEyeTarget saved in editor actor"));

                                    EditorComponent->Modify(true);
                                    GEditor->EndTransaction();
                                }
                            }
                            else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(EditorComponentProperty))
                            {
                                FStructProperty* InnerStructProp = CastField<FStructProperty>(ArrayProp->Inner);
                                if (InnerStructProp && COMPONENT_STRUCT_DATA_ALLOWED_UNREAL_TYPE(InnerStructProp->Struct, FBlackEyeTarget))
                                {
                                    const TArray<FBlackEyeTarget>& SceneTargetsArray = *ArrayProp->ContainerPtrToValuePtr<TArray<FBlackEyeTarget>>(ActorComponent);
                                    TArray<FBlackEyeTarget>& EditorTargetsArray = *ArrayProp->ContainerPtrToValuePtr<TArray<FBlackEyeTarget>>(EditorComponent);

                                    GEditor->BeginTransaction(FText::FromString("BlackEye Save In Play"));
                                    EditorTargetsArray.Reset(SceneTargetsArray.Num());
                                    for (int i = 0; i < SceneTargetsArray.Num(); ++i)
                                    {
                                        const FBlackEyeTarget& SourceElement = SceneTargetsArray[i];
                                        FBlackEyeTarget TargetElement = FBlackEyeTarget();

                                        CopyBlackEyeTargetProperty(InnerStructProp, &SourceElement, &TargetElement, EditorWorld);
                                        EditorTargetsArray.Add(TargetElement);
                                    }

                                    EditorComponent->Modify(true);
                                    GEditor->EndTransaction();

                                    UE_LOG(LogTemp, Display, TEXT("BlackEye SIP:BlackEyeTarget array copy. Size: %i!"), SceneTargetsArray.Num());
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Display, TEXT("BlackEye SIP:Arrays are unsupported atm. Sorry!"));
                                }
                            }
                            else
                            {
                                GEditor->BeginTransaction(FText::FromString("BlackEye Save In Play"));

                                COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Name)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Bool)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Float)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Double)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Byte)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Int8)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Int16)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Int)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, Int64)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, UInt16)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, UInt32)
                                else COMPONENT_POD_DATA_COPY(EditorComponentProperty, ActorComponent, EditorComponent, UInt64)

                                if (FEnumProperty* EnumProp = CastField<FEnumProperty>(EditorComponentProperty))
                                {
                                    UE_LOG(LogTemp, Display, TEXT("BlackEye SIP: ENUM saved in editor actor"));
                                    void* Container = EnumProp->ContainerPtrToValuePtr<void>(ActorComponent);
                                    EnumProp->SetValue_InContainer(EditorComponent, Container);
                                }

                                EditorComponent->Modify(true);
                                GEditor->EndTransaction();
                            }

                        }
                        GIsPlayInEditorWorld = PrevPlayInEditorWorld;
                        GWorld = PrevWorld;
                    }
                }
            }
        }
    }
}
