# NPPM Demo Operations Guide

## Project Status

Current baseline status:
- the project builds successfully on this machine
- the main app launches successfully on this machine
- the browser control panel works for recipe selection and demo control
- `.cast` and `.yaml` recipes are accepted by `x_mainmoil`
- `.script` is retained as a legacy migration format and is planned for removal after YAML migration
- metadata, assets, and mapping cache locations are external to the repository

## Current Architecture Snapshot

Current responsibilities:
- `apps/nppm/cast.cpp` resolves recipe paths, metadata roots, and runtime symlinks
- `apps/nppm/nppm.cpp` initializes mapping caches and runs the display/UI loop
- `tools/nppm_control_server.py` provides browser-based recipe and demo control
- `.nppm-meta/defaultS` contains recipe-control files

## External Runtime Layout

Current machine layout:
- metadata root: `/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta`
- preferred active recipe state file: `/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta/defaultS/active_recipes.yaml`
- compatibility active recipe file: `/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta/defaultS/active_recipe.txt`
- legacy recipe list: `/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta/defaultS/2Demo.txt`
- active recipe inventory for the browser UI comes from real `*.cast`, `*.script`, and `*.yaml` files under `defaultS/`
- external assets are stored under `/home/cj/usb_1T_disk/Dropbox/...`

Notes:
- the code can still fall back to legacy `/home/cj/Dropbox/...` locations where needed
- recipes may use `@DROPBOX_ROOT@/...` for Dropbox-backed assets instead of hardcoded `/home/cj/...` paths
- mapping cache paths come from recipe/config values, not from the repository root itself

## Build

Verify OpenCV CMake config exists:

```bash
ls /home/cj/cv-2026/lib/cmake/opencv4/OpenCVConfig.cmake
```

Configure and build:

```bash
cmake -S . -B build -DOpenCV_DIR=/home/cj/cv-2026/lib/cmake/opencv4
cmake --build build -j2
```

Expected main binary:

```text
build/apps/nppm/x_mainmoil
```

## Run From Terminal

Normal launch:

```bash
DISPLAY=:0 ./build/apps/nppm/x_mainmoil
```

Explicit metadata root:

```bash
DISPLAY=:0 CJ_NPPM_META=/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta ./build/apps/nppm/x_mainmoil
```

Explicit Dropbox asset root:

```bash
DISPLAY=:0 CJ_NPPM_DROPBOX_ROOT=/home/cj/usb_1T_disk/Dropbox ./build/apps/nppm/x_mainmoil
```

Explicit recipe file:

```bash
DISPLAY=:0 CJ_NPPM_RECIPE=/absolute/path/to/recipe.cast ./build/apps/nppm/x_mainmoil
```

Or:

```bash
DISPLAY=:0 ./build/apps/nppm/x_mainmoil /absolute/path/to/recipe.cast
```

`.script` files are no longer runnable by `x_mainmoil`. Convert them first:

```bash
python3 tools/convert_recipe_to_yaml.py /absolute/path/to/recipe.script /absolute/path/to/output.yaml
DISPLAY=:0 ./build/apps/nppm/x_mainmoil /absolute/path/to/output.yaml
```

`.yaml` files are also supported:

```bash
DISPLAY=:0 ./build/apps/nppm/x_mainmoil /absolute/path/to/recipe.yaml
```

## Run From CLion

Use CLion as a CMake project, not as a single-file compile.

Required CLion setup:
- target: `x_mainmoil`
- CMake option:

```text
-DOpenCV_DIR=/home/cj/cv-2026/lib/cmake/opencv4
```

Important:
- set `OpenCV_DIR` in CLion CMake settings
- do not pass `-DOpenCV_DIR=...` as a build argument to `cmake --build`

Recommended environment variables for the CLion run configuration:

```text
DISPLAY=:0;CJ_NPPM_META=/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta;CJ_NPPM_DROPBOX_ROOT=/home/cj/usb_1T_disk/Dropbox
```

## Run Python Control Server

Launch:

```bash
python3 tools/nppm_control_server.py
```

Open:

```text
http://127.0.0.1:8080
```

If port `8080` is already in use:

```bash
NPPM_CONTROL_PORT=8081 python3 tools/nppm_control_server.py
```

Browser UI features:
- per-type active recipe selection stored in `active_recipes.yaml`
- one selected list type at a time:
  - `cast`
  - `script (legacy)`
  - `yaml`
- per-recipe Demo enable checkboxes
- Demo `All` toggle
- configurable timeout
- countdown to next recipe switch
- start/stop app controls
- recipe editor
- `Check Recipe`
- detected Dropbox root is shown in the UI status
- `.script` stays visible for editing and conversion, but the control server no longer runs it directly

## Convert Legacy Recipes To YAML

Converter:

```bash
python3 tools/convert_recipe_to_yaml.py /path/to/recipe.cast
python3 tools/convert_recipe_to_yaml.py /path/to/recipe.script /path/to/output.yaml
```

Recommended follow-up validation:

```bash
./build/apps/nppm/x_mainmoil --check-recipe /path/to/output.yaml
```

References:
- YAML recipe design: [docs/yaml-recipe-v1-design.md](/home/cj/CLionProjects/nppm-demo-aug-2023/docs/yaml-recipe-v1-design.md)
- legacy migration note: [cdx-cj-recipe-format-suggestions.md](/home/cj/CLionProjects/nppm-demo-aug-2023/cdx-cj-recipe-format-suggestions.md)

## Recipe Selection Flow

Recipe resolution order:
1. `argv[1]` if it points to a valid recipe file
2. `CJ_NPPM_RECIPE` if it points to a valid recipe file
3. `CJ_NPPM_META`
4. repo-local `.nppm-meta`
5. `/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta`
6. `/home/cj/Dropbox/19-22-ing/.nppm-meta`

Inside a resolved metadata root:
1. `defaultS/active_recipes.yaml`
2. `defaultS/active_recipe.txt`
3. `defaultS/2Demo.txt` as legacy fallback

Notes:
- `active_recipes.yaml` is the preferred mechanism going forward
- `active_recipes.yaml` stores per-type selection:
  - `active.cast`
  - `active.yaml`
  - `default_kind`
- `active_recipe.txt` remains as a compatibility mirror of the selected `default_kind`
- `.script` should be treated as a legacy migration format rather than a long-term peer of `.cast` and `.yaml`
- `x_mainmoil` no longer runs `.script` directly; convert it to YAML first
- the control server no longer uses `.script` for Active, Demo, Run, or Check
- `2Demo.txt` remains supported as a legacy compatibility path
- legacy note: [docs/2Demo-legacy-note.md](/home/cj/CLionProjects/nppm-demo-aug-2023/docs/2Demo-legacy-note.md)
- the browser UI no longer uses `2Demo.txt` as the list source; it lists real files in `defaultS/`
- `@DROPBOX_ROOT@/...` is the preferred recipe form for Dropbox-backed asset paths
- `CJ_NPPM_DROPBOX_ROOT` overrides automatic Dropbox-root detection

Example `active_recipes.yaml`:

```yaml
active:
  cast: "leo-fisheye@neihu-mp4-115.cast"
  yaml: "leo-fisheye@lab103.yaml"
default_kind: yaml
```

## Runtime Symlinks

The app currently maintains these runtime symlinks:
- `running_recipe@<timestamp>` points to the selected recipe file
- `apps/nppm/ln-nppm-meta` points to the resolved metadata root
- `apps/nppm/ln-defaultS` points to the resolved `defaultS/` directory
- `apps/nppm/ln-active_recipes.yaml` points to the resolved `defaultS/active_recipes.yaml`
- `apps/nppm/ln-active_recipe.txt` points to the resolved `defaultS/active_recipe.txt`
- `apps/nppm/ln-2Demo.txt` points to the resolved `defaultS/2Demo.txt`

## Known Issues

- the window title-bar close button is not the supported exit path
- use `Esc` or the in-app `Exit` button to leave the app
- `active_recipe.txt` is now a compatibility file and should eventually be removable after all deployments adopt `active_recipes.yaml`
- `.script` is expected to be removed after YAML migration completes; avoid adding new `.script`-specific workflows
- CLion users must configure `OpenCV_DIR` in CMake settings, not in build command arguments
- the repository still depends on external metadata and asset roots
- unsupported media input:
  - legacy Raspberry Pi shell-based capture path that previously used hardcoded `curl`
- supported media inputs:
  - local image
  - local video file
  - direct HTTP stream URL through `@la_video_stream_uri`

## Pre-Refactor Baseline

This branch captures a working baseline before structural refactoring.

Planned refactor direction:
- start by separating recipe/path resolution from `cast.cpp`
- then isolate symlink management and mapping-cache responsibilities
