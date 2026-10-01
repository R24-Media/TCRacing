# Third-party notices and content inventory

This file records an initial repository-level review. It is not a complete software bill of materials, legal opinion, or grant of rights. Entries must be verified against the applicable licenses and acquisition records before redistribution.

## Identified third-party plugin

### Cinematographer PRO

- **Repository path:** `Plugins/Cinematographer/`
- **Manifest attribution:** `lumines_labs`
- **Manifest version:** 1.2
- **Engine version stated in manifest:** Unreal Engine 5.6.0
- **References in manifest:**
  - Website: https://lumines-labs.com
  - Documentation: https://lumines-labs.com/Cinematographer-Documentation.html
  - Marketplace product identifier: `223cd74a5e0a42eca98e585f25b0b729`
- **Source attribution:** plugin source files include lumines_labs copyright notices.
- **Review status:** third-party; redistribution and source-publication rights not verified.

Do not relicense this plugin, remove its attribution, or represent it as original R24-Media work. Confirm the applicable Epic Marketplace and publisher terms and whether this repository's current distribution is authorized. If redistribution is not permitted, remove it from any future public source distribution and document an authorized installation method instead.

## Unreal Engine and Marketplace dependencies

`TCRacing.uproject` enables several Unreal Engine plugins and identifies Marketplace references for AJA Media, Blackmagic Media, and Cinematographer PRO. The project also contains binary Unreal assets (`.uasset` and `.umap`) whose origins and redistribution rights have not yet been fully inventoried.

- **Review status:** incomplete.
- **Required action:** classify each plugin and asset as R24-Media original, Epic-provided, Marketplace/third-party, or otherwise licensed; record the applicable terms and confirm redistribution permissions before selecting a repository-wide license.

## Local dependency reference

The project file includes the additional plugin directory:

`../../../Desktop/R25TV/UE_Resources/PostProcess/BlackEyeCameras`

This is a machine-specific path. Its contents, ownership, license, and role in project loading must be identified before a portable release.

## Licensing policy

No license in this file grants rights to third-party code, plugins, or assets. Add a project license only after the maintainer has confirmed that all material covered by it can legally be licensed under those terms. Keep third-party notices and required attributions intact.
