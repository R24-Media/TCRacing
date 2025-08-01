// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class FBlackEyeUICommands final : public TCommands<FBlackEyeUICommands>
{
public:
    FBlackEyeUICommands();

    TSharedPtr<FUICommandInfo> SaveInPlayToggleButton;

    void RegisterCommands() override;
};
