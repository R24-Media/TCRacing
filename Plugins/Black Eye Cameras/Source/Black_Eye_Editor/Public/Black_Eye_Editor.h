// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#pragma once

class FBlack_Eye_EditorModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

private:
    void RegisterMenuExtensions();
};
