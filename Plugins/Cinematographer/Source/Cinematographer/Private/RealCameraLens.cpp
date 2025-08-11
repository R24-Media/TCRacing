// Copyright 2022 lumines_labs. All Rights Reserved.

#include "RealCameraLens.h"

URealCameraLens::URealCameraLens()
{

    if(b_isPrimeLens)
    {
      RealLensSettings.MinFocalLength = FocalLength;
      RealLensSettings.MaxFocalLength = FocalLength;
    }
    else
    {
        RealLensSettings.MinFocalLength = FocalLengthRange.X;
        RealLensSettings.MaxFocalLength = FocalLengthRange.Y;
    }

    RealLensSettings.MinFStop = FStopRange.X;
    RealLensSettings.MaxFStop = FStopRange.Y;
}
