# TCRacing

TCRacing is an Unreal Engine project for real-time automotive visualization and virtual-production experimentation, maintained by R24-Media.

> **Repository status: licensing and third-party-content review in progress.** This repository is public, but it does not currently declare an open-source license. Public visibility alone does not grant permission to reuse, modify, or redistribute its code or assets. Until the review is complete and a suitable license is added, please do not treat the project as an approved open-source release.

## Project information

- **Engine:** Unreal Engine 5.6 (see `TCRacing.uproject`)
- **Primary technology:** Unreal Engine project content, configuration, and C++
- **Maintainer:** R24-Media
- **Repository:** https://github.com/R24-Media/TCRacing

The project configuration enables Unreal Engine tools related to Sequencer scripting, Python, virtual cameras, Live Link, Composure, media I/O, Movie Render Pipeline, camera calibration, and virtual production. Availability of these engine features depends on the installed Unreal Engine version, platform support, and any required plugins or licenses.

## Important third-party content notice

The repository currently includes the `Cinematographer PRO` plugin. Its manifest attributes the plugin to **lumines_labs** and references its Marketplace listing and documentation. Its source files also contain lumines_labs copyright notices. This plugin is not represented as original R24-Media code, and its presence here does not establish permission to redistribute it independently.

The project also enables other Unreal Engine and Marketplace plugins and contains binary Unreal assets. Their respective terms and redistribution rights must be reviewed separately. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for the initial inventory and review status.

## Getting started

1. Install Unreal Engine **5.6** using an Epic Games account and the applicable Epic Games terms.
2. Clone the repository only if you have the rights to access and use its included content.
3. Open `TCRacing.uproject` with Unreal Engine 5.6.
4. Install or enable any required plugins through the appropriate authorized source.
5. Generate project files if your platform or workflow requires them, then open the project in the Unreal Editor.

This is a preliminary outline, not a verified clean-machine setup guide. The project references an additional plugin directory at `../../../Desktop/R25TV/UE_Resources/PostProcess/BlackEyeCameras`, which is specific to a local filesystem and is not guaranteed to exist for other users. A portable setup requires that dependency to be identified and resolved.

## Development and validation

No automated build or test procedure is currently documented. The project should be opened and validated in a clean Unreal Engine 5.6 environment before a reproducible release is claimed. Contributions, builds, and redistribution remain subject to the licensing review above.

## Contribution status

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before proposing changes. Until the licensing and asset review is complete, this repository should be treated as a public development snapshot, not a fully cleared open-source distribution.

## License

**No open-source license is currently declared for this repository.** Do not assume that any part of the project is available under an open-source license. A license will be added only after the maintainer has verified ownership and redistribution rights for the material it covers.
