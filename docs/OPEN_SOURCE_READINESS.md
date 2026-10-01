# Open-source readiness: maintainer checklist

This document tracks verifiable release requirements. A checked item should have evidence (for example, a license file, clean-environment validation record, or published release); do not check items based only on intention.

## Rights and provenance

- [ ] Identify the copyright holder and origin of every source file.
- [ ] Identify the origin and license/terms for all plugins, Marketplace content, maps, meshes, materials, textures, sounds, fonts, and other binary assets.
- [ ] Confirm rights to publish, modify, and redistribute each included item.
- [ ] Separate third-party components that cannot be redistributed; retain required notices.
- [ ] Confirm contributor ownership and obtain appropriate contribution terms if accepting outside contributions.
- [ ] Select and add a license only for content the maintainer is authorized to license.
- [ ] Confirm the license and third-party notices are consistent across the repository and release packages.

## Portability and security

- [ ] Identify and resolve machine-specific paths and external plugin dependencies.
- [ ] Search tracked files and repository history for credentials, tokens, private endpoints, and personal data.
- [ ] Revoke or rotate any exposed credential; removing it from the latest file does not remove it from Git history.
- [ ] Review enabled editor/remote-control/network services and document safe defaults.
- [ ] Verify that a clean checkout does not require private files or unlicensed assets.

## Build, testing, and documentation

- [ ] Verify project opening in a clean Unreal Engine 5.6 environment.
- [ ] Record exact platform, engine build, plugins, and setup steps used for validation.
- [ ] Document a reproducible build/package or validation procedure where applicable.
- [ ] Add test or validation coverage for original source code.
- [ ] Document project scope, limitations, known issues, and contribution workflow.
- [ ] Add versioned releases only after validating the included content and rights.

## Maintainer activity and program application

- [ ] Confirm the applicant is a primary or core maintainer with repository write access.
- [ ] Record authentic maintenance activity: reviewed PRs, issue triage, releases, and ongoing support.
- [ ] Gather verifiable usage/adoption signals (stars, downloads, dependents, deployments, or ecosystem role); do not estimate or inflate them.
- [ ] Explain the project's specific value to the software ecosystem, with evidence.
- [ ] Describe concrete Codex/API-credit workflows that support ongoing OSS maintenance.
- [ ] Verify current program terms and eligibility immediately before applying.

## Current known blockers

- The repository has no declared open-source license.
- The `Plugins/Cinematographer` source is attributed to lumines_labs; redistribution rights are not verified.
- Other plugins and binary Unreal assets need provenance and license review.
- `TCRacing.uproject` contains a machine-specific additional plugin directory.
- The Unreal project has not been validated here in a clean Unreal Engine installation.
- A token-like value was present in the tracked Android File Server configuration. It has been blanked on this readiness branch, but any exposed credential must be treated as compromised and rotated; Git history may still contain it.
- Repository visibility and documentation improvements alone do not establish eligibility for an open-source maintainer program.
