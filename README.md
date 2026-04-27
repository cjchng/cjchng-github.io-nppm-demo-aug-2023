# NPPM Demo

This repository contains the `x_mainmoil` application and a small browser control panel for recipe and demo management.

GitHub Pages site: https://cjchng.github.io/

Detailed operations, troubleshooting, and architecture notes are in [cdx-cj-README.md](cdx-cj-README.md).

## Overview

Main components:
- `build/apps/nppm/x_mainmoil`
- `tools/nppm_control_server.py`

Recipe file types:
- `.cast`
- `.script` (legacy migration format)
- `.yaml`

## Prerequisites

Current machine assumptions:
- OpenCV CMake package: `/home/cj/cv-2026/lib/cmake/opencv4`
- metadata root: `/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta`

## Build

```bash
cmake -S . -B build -DOpenCV_DIR=/home/cj/cv-2026/lib/cmake/opencv4
cmake --build build -j2
```

## Run

```bash
DISPLAY=:0 ./build/apps/nppm/x_mainmoil
```

If you want to force the metadata root:

```bash
DISPLAY=:0 CJ_NPPM_META=/home/cj/usb_1T_disk/Dropbox/19-22-ing/.nppm-meta ./build/apps/nppm/x_mainmoil
```

If you want to force the Dropbox asset root used by recipe placeholders:

```bash
DISPLAY=:0 CJ_NPPM_DROPBOX_ROOT=/home/cj/usb_1T_disk/Dropbox ./build/apps/nppm/x_mainmoil
```

## Browser Control Panel

```bash
python3 tools/nppm_control_server.py
```

Open:

```text
http://127.0.0.1:8080
```

Current browser UI features:
- per-type active recipe selection stored in `defaultS/active_recipes.yaml`
- one-type-at-a-time list filter:
  - `cast`
  - `script (legacy)`
  - `yaml`
- per-recipe Demo checkboxes
- Demo `All` toggle
- demo timeout
- demo countdown
- `Run App`, `Check Recipe`, and `Open Editor`
- `script (legacy)` is visible for editing and YAML conversion, but not runnable from the control server

## Convert Legacy Recipes

Use the converter to turn a legacy `.cast` or `.script` recipe into YAML:

```bash
python3 tools/convert_recipe_to_yaml.py /path/to/recipe.cast
python3 tools/convert_recipe_to_yaml.py /path/to/recipe.script /path/to/output.yaml
```

Then validate the converted file:

```bash
./build/apps/nppm/x_mainmoil --check-recipe /path/to/output.yaml
```

## Notes

- preferred recipe source: `defaultS/active_recipes.yaml`
- compatibility fallback: `defaultS/active_recipe.txt`
- legacy fallback: `defaultS/2Demo.txt`
- legacy note: [docs/2Demo-legacy-note.md](docs/2Demo-legacy-note.md)
- `x_mainmoil` accepts `.cast` and `.yaml` recipe files at runtime
- `.script` is now a legacy migration format and should be converted to `.yaml` where possible
- `.script` remains supported by the converter and editor for migration only
- the control server does not run `.script` directly anymore; use `Transfer to YAML` first
- `active_recipes.yaml` stores:
  - `active.cast`
  - `active.yaml`
  - `default_kind`
- `active_recipe.txt` is still written as a legacy mirror of the current `default_kind` selection
- recipes may use `@DROPBOX_ROOT@/...` instead of machine-specific absolute Dropbox paths
- `CJ_NPPM_DROPBOX_ROOT` overrides the detected Dropbox root at runtime
- YAML recipe design: [docs/yaml-recipe-v1-design.md](docs/yaml-recipe-v1-design.md)
- runtime symlinks are refreshed automatically:
  - `running_recipe@...`
  - `apps/nppm/ln-nppm-meta`
  - `apps/nppm/ln-defaultS`
  - `apps/nppm/ln-active_recipes.yaml`
  - `apps/nppm/ln-active_recipe.txt`
  - `apps/nppm/ln-2Demo.txt`
- supported media inputs:
  - local image
  - local video file
  - direct HTTP stream URL handled by OpenCV
- unsupported media input:
  - legacy Raspberry Pi shell-based `curl` capture path
- supported exit methods:
  - `Esc`
  - in-app `Exit`
- the window title-bar close button is not the supported exit path

Example `defaultS/active_recipes.yaml`:

```yaml
active:
  cast: "leo-fisheye@neihu-mp4-115.cast"
  yaml: "leo-fisheye@lab103.yaml"
default_kind: yaml
```
