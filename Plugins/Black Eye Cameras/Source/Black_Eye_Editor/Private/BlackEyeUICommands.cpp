// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "BlackEyeUICommands.h"

#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "Black_Eye_EditorModule"

FBlackEyeUICommands::FBlackEyeUICommands()
    : TCommands<FBlackEyeUICommands>(
        // Unique name of the commands set
        "BlackEyeUICommands",

        // Human readable name (will be displayed in the editor preferences window)
        LOCTEXT("BlackEyeUICommandsName", "Black Eye Commands"),

        // Name of the parent commands set this one is extending (if any)
        NAME_None,

        // Name of the style set from which command icons should be loaded (if any)
        FAppStyle::GetAppStyleSetName()
    )
{}


void FBlackEyeUICommands::RegisterCommands()
{
    //UI_COMMAND
    //(
    //    MyCustomButton,

    //    // Label
    //    "My custom button",

    //    // Tooltip
    //    "Tooltip for my custom button",

    //    // UI representation (when used to dynamically build toolbars and menus)
    //    EUserInterfaceActionType::Button,

    //    // Default keyboard shortcut (can be empty)
    //    FInputChord(EKeys::C, EModifierKey::Shift | EModifierKey::Alt)
    //);

    //UI_COMMAND(MyCustomEntry, "My custom entry", "Tooltip for my custom entry",
    //    EUserInterfaceActionType::Button, FInputChord());

    UI_COMMAND(SaveInPlayToggleButton, "Save In Play", "Toggles save in play for Black Eye cameras",
        EUserInterfaceActionType::ToggleButton, FInputChord());

    
}

#undef LOCTEXT_NAMESPACE
