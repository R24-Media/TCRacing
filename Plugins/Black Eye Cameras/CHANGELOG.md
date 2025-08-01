# Black Eye Technologies Camera Plugin CHANGELOG

## 1.0.0 (2024-11-12)
### Known Issues
  - When showing black eye camera gizmos in the viewport with a UE version below 5.5 there is an issue when using a camera with `Constrain Aspect Ratio` enabled. When enabled, the viewport will add 50% extra letterbox and Canvas based gizmos will not draw correctly. This is an Unreal Editor bug which we cannot solve from our plugin.
    - You may workaround this by resizing your level viewport to the aspect ratio requested by the active camera (i.e. do not let it draw letterbox bars)
  - Using spin boxes (UI sliders) does not consistently live update the viewport and camera evaluations due to an engine bug dating back to 4.27.
  - Level Sequences + Black Eye Camera components in non-C++ actors (blueprints)
    - When using blueprints and spawnables in level sequencer, due to limitations of sequencer serialization, some properties will not save with the sequence
    - To work around this, please convert the camera actor BP to a C++ actor class and it will behave as expected

### Added:
 - Support for UE 5.1, and 5.2
  
### Changed:
  - Subjects' USTRUCT() now use soft object pointers in C++ code instead of weak object pointers for both `Look At` and `Follow` components

### Fixed:
  - Level Sequences + Black Eye Camera component: changes to the components in our look at actor do not save when spawned via a level sequence
    - This was fixed by moving our look at Blue Print to a C++ class (see known issues for details)

### Removed:

## 0.9.0 (2024-11-05)
### Known Issues
  - When showing black eye camera gizmos in the viewport with a UE version below 5.5 there is an issue when using a camera with `Constrain Aspect Ratio` enabled. When enabled, the viewport will add 50% extra letterbox and Canvas based gizmos will not draw correctly. This is an Unreal Editor bug which we cannot solve from our plugin.
    - You may workaround this by resizing your level viewport to the aspect ratio requested by the active camera (i.e. do not let it draw letterbox bars)

### Added:
  - Translating a Black Eye Camera rig actor with a `Follow` component as its root component will automatically use the translation to offset its position relative to its subjects
    - Note that this is a feature of the `Black Eye Camera Rig` Actor, and any Black Eye components not under this root actor type do not gain this benefit.
  - New CVAR for drawing camera names while editing is `BlackEye.ShowCameraNames` and defaults to `1`
  - Debug drawing now toggled via the `Show` menu in the level editor viewport `Show Black Eye Cameras`
    - Use the CVAR `ShowFlag.BlackEyeCameras 1` to enable in gameplay
  - CVAR to disable all smooth motion damping `BlackEye.DisableDamping 1`
  - Look At components now have variables to set values for camera Pedestal and Plate dimensions to give look tracking authentic feel for larger camera rigs
  - Look at component now has two extra variables to set the pedestal height, and plate distance which will offset the camera component and rotate it as if it were a physically mounted camera
  - Follow component now has a radius you may set which can be used to ignore changes in its position by a given radius. This effecively acts like a spatial dead zone for movements of its follow subjects
  - `Calculated FoV` added to dynamic FoV category in `Look At` component to give insight to the FoV being calculated based on requested subject viewport size.
  
### Changed:
  - `FollowOffset` in `Follow` component is not interpolable, allowing it to be keyframed in sequencer.
  - `bSetFocalDistance` in `LookAt` component now defaults to `true`
  - All debug drawing has been refactored to share a single drawing entrypoint
  - All blueprint cameras were renamed from `BET_Basic_Follow` (etc) to `Black_Eye_Basic_Follow`, `Black_Eye_Basic_LookAt`
    - When updating to this package, please add the appropriate `ClassRedirect` for these BP classes (i.e. `+ActiveClassRedirects=(OldClassName=”UMyClass”,NewClassName=”UMyNewClass”)`) in your `Engine.ini`
  - `ACameraRig` actor has been renamed to `ABlackEyeCameraRigBase`. Please update your core redirects to preserve data
    - Add `+ActiveClassRedirects=(OldClassName="/Script/Black_Eye.CameraRig",NewClassName="/Script/Black_Eye.BlackEyeCameraRigBase")` to your `[/Script/Engine.Engine]` section of your `Engine.ini`
  - Changed BP API for LookAt to include ability to very target count, add/remove targets, or change an existing target
  - Do not draw debug gizmos for camera actors hidden in the level editor

### Fixed:
  - Assigning a subject in `Look Component` when it is the root component of an actor would cause UE to crash
  - Crash when using a skeletal mesh component without a mesh asset assigned as a subject in Follow or LookAt

### Removed:
  - All `bet.*` CVARS have been removed in favour of the toggle in the level editor viewport + show flags CVAR. See manual for current CVARS available.

## 0.0.9 (2024-09-20)

Added:
  - API to `ACameraRig`: `SnapComponentsToTargets()` which signals to all look and follow components that they should snap to their respective targets as configured on the next frame. This method is BlueprintCallable.
  - API to `IBlackEyeHasFollow`: `SnapToTargets()` which is meant to signal to concrete implementations that they should snap to their respective targets on the next frame
  - API to `IBlackEyeHasLookAt`: `SnapToTargets()` which is meant to signal to concrete implementations that they should snap to their respective targets on the next frame
  
Changed:
  - 
  
Fixed:
  - Changed the look at component to function better when no damping is used but a follow component with rotation and translation damping is applied
  - CVAR for positional gizmos was not actually doing anything: now functions as intended
  - Renamed `Camera Rig` actor to `Black Eye Camera Rig`
  
## 0.0.8 (2024-09-16)

Added:
  - CVAR for camera frustums `` now has 3 states: `0` disables frustum drawing, `1` will draw only active camera, and `2` will draw all frustums
  - Macintosh computer support!
  
Changed:
  - 
  
Fixed:
  - 

## 0.0.7 (2024-09-10)

Added:
  - CVAR for displaying BET camera actor frustums in scene during game play. `bet.ShowCameraFrustums`
  - CVAR for displaying BET camera position gizmos. `bet.ShowCameraPositionGizmos` 
  - Follow offset has been added to the follow component, and will respect the orientation reference mode set when following a single subject. When following multiple subjects, this will always be a world offset.
  - Support for velocity look ahead time when targeting bones in a USkeletalMeshComponent in the look component
  
Changed:
  - Full lint pass on the code base for public API documentation and conforming to UE coding standards
  - Gizmo drawing CVAR now also change drawing behaviour for editor visualizers and drawing methods for Black Eye components
  
Fixed:
  - Follow orientation reference mode was not correctly functioning when set to world space, or heading only when following a single subject
  - Local position offsets when using a bone for the look at target in a skeletal mesh now functions again in the look component

## 0.0.6 (2024-08-21)

Added:
  - Full linux support for the plugin runtime and editor
  
Changed:
  - Updated some tooltips to provide more context for their property's functionality
  
Fixed:
  - 

## 0.0.5 (2024-08-12)

Added:
  - Ability to toggle a multi-target follow mode in the Follow component. Multi-target mode will disable orientation reference and its associated rotation offsets/damping.
  - Targets in Follow component when set to multiple target mode now has a weight value which can be used to change how the target positions are combined
  
Changed:
  - Updated follow component debug visualization to support multiple targets
  - Moved composition preset buttons into a detail panel customizer under the composition category for the look at component
  - Rearranged properties in look at component: this will reset all damping tuning values back to their defaults
  
Fixed:
  - 

## 0.0.4 (2024-08-06)

Added:
  - Links to Black Eye website for support & documentation added
  
Changed:
  - 
  
Fixed:
  - Level Editor module access issues when using MRQ for remote rendering have been resolved.
  -- Make sure that when rendering movies in MRQ, that camera gizmo rendering is disabled via CVAR "bet.ShowCameraGizmos" set to "0"

## 0.0.3 (2024-08-01)

Added:
  - 
  
Changed:
  - 
  
Fixed:
  - Changed module loading code in Black Eye Editor module to use unchecked API so that it won't throw an exception in deployments where the module(s) are not loaded

## 0.0.2 (2024-07-30)

Added:
  - Added blueprint function calls for both Follow and LookAt components to set their target(s)
  
Changed:
  - 
  
Fixed:
  - Changed default value of `Keep On Screen` to `false` in Look at component

## 0.0.1 (2024-07-01)
- Initial release