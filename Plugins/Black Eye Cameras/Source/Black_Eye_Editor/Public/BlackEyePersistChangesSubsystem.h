// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "BlackEyePersistChangesSubsystem.generated.h"

/**
 * Subsystem which is used to persist changes between the player and editor worlds when modifying black eye camera components
 */
UCLASS()
class BLACK_EYE_EDITOR_API UBlackEyePersistChangesSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    bool IsSaveInPlayEnabled() const { return bSaveInPlayEnabled; }
    void SetSaveInPlayState(bool Enabled);

private:
    class AActor* TryGetMatchingEditorActor(class AActor* PieActor, class UWorld* EditorWorld);

    void CopyBlackEyeTargetProperty(class FStructProperty* BETTargetProperty, 
                                    const struct FBlackEyeSimpleTarget* SceneTarget, 
                                    struct FBlackEyeSimpleTarget* EditorTarget,
                                    class UWorld* EditorWorld);

    void OnPostObjectEditProperty(class UObject* ObjectBeingModified, FPropertyChangedEvent& InPropertyChangedEvent);
    void OnPIEChangedEvent(bool bIsSimulating);

    FDelegateHandle EditPropertyDelegateHandle;

    FDelegateHandle BeginPIEDelegateHandle;
    FDelegateHandle EndPIEDelegateHandle;

    TMap<TWeakObjectPtr<class AActor>, TWeakObjectPtr<class AActor>> PIEActorToEditorActorMap;
    bool bSaveInPlayEnabled;
};
