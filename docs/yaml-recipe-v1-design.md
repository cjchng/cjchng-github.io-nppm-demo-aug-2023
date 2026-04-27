# YAML Recipe V1 Design

## Purpose

This document defines a practical YAML replacement target for the current `.cast` and `.script` recipe files used by `x_mainmoil`.

Goals:
- replace the flat token/value stream with a structured public format
- preserve the current runtime behavior and field coverage
- make validation clearer and earlier
- avoid hardcoded machine-local paths in recipe files

Non-goals for v1:
- redesigning the rendering model
- removing `.cast` support immediately
- adding dynamic includes or nested recipe inheritance

## File Naming

Preferred new extension:

```text
*.yml
```

Example:

```text
narl@images_lab113_38-18.yml
```

## Top-Level Shape

```yaml
version: 1

camera:
  name: narl
  sensor:
    width: 0
    height: 0
  intrinsics:
    icx: 0
    icy: 0
    ratio: 1
    image_width: 0
    image_height: 0
    calibration_ratio: 0
    para: [0, 0, 0, 0, 0, 0]

paths:
  working_root: "@DROPBOX_ROOT@/fic-092021/nppm@1T/nppm/"
  camera_folder: entaniya-vr220
  image_folder: images/
  image_file: example.png
  mapper_folder: mapper/

media:
  type: image
  usb_camera: false
  source: ""

display:
  title: "NPPM Demo"
  cols: 1920
  rows: 1080
  frame_aspect: 0.75
  virtual_origin_left: 0
  virtual_origin_top: 0

projection:
  mode: car_pano
  ic_alpha_degree: 90
  ic_beta_degree: 30

views:
  pyz:
    - [0, -45, 10]
    - [0, 0, 10]
    - [0, 45, 10]
    - [-60, 0, 10]
    - [-50, -20, 12]
    - [-50, 20, 12]

roi:
  car_pano_left: 0
  car_pano_top: 0
  car_pano_cols: 1920
  car_pano_rows: 1080
  display_aspect: 0.75
  rt_alpha_from: 0
  rt_alpha_to: 1.5

tuning:
  alpha_range_vendor: 110
  alpha_pano_min: 20
  alpha_pano_max: 110
  beta_pano_begin: 90
  beta_pano_range: 360
  alpha_pano_min_init: 30
  alpha_pano_max_init: 90
  alpha_pipe_init: 41.4
  beta_pipe_init: -15.7
  zoom_pipe_init: 0.5
  pitch_car_init: -55.8
  yaw_car_init: 0
  zoom_car_init: 2.2
```

## Field Mapping From Current Tokens

### Camera

Current tokens:
- `@md->cfg->cameraName`
- `@md->cfg->cameraSensorWidth`
- `@md->cfg->ameraSensorHeight`
- `@md->cfg->iCx`
- `@md->cfg->iCy`
- `@md->cfg->i_ratio`
- `@md->cfg->imageWidth`
- `@md->cfg->imageHeight`
- `@md->cfg->calibrationRatio`
- `@md->cfg->para0` .. `@md->cfg->para5`

YAML target:

```yaml
camera:
  name: ...
  sensor:
    width: ...
    height: ...
  intrinsics:
    icx: ...
    icy: ...
    ratio: ...
    image_width: ...
    image_height: ...
    calibration_ratio: ...
    para: [p0, p1, p2, p3, p4, p5]
```

### Paths

Current tokens:
- `@la_nppm_ing_abs_folder`
- `@la_camera_name_as_inst_short_folder`
- `@la_using_image_short_folder`
- `@la_image_inst_short_file`
- `@la_mappers_folder_short`

YAML target:

```yaml
paths:
  working_root: ...
  camera_folder: ...
  image_folder: ...
  image_file: ...
  mapper_folder: ...
```

Rules:
- `working_root` may use `@DROPBOX_ROOT@/...`
- other path fields should normally be relative
- absolute filesystem paths should remain invalid in validation

### Media

Current tokens:
- `@la_is_camera_ready`
- `@la_is_usb_camera`
- `@la_video_stream_uri`

YAML target:

```yaml
media:
  type: image | video_file | camera_usb | stream_http
  usb_camera: false
  source: ...
```

Notes:
- `source` is empty for pure image mode
- `source` is a file path or URL for video/stream modes
- unsupported legacy Raspberry Pi shell-capture mode must not appear in YAML

### Display

Current tokens:
- `@la_screen_title`
- `@la_display_cols`
- `@la_display_rows`
- `@la_vir_frame_asp`
- `@VIRTUAL_ORGIN_LEFT`
- `@VIRTUAL_ORGIN_TOP`

YAML target:

```yaml
display:
  title: ...
  cols: ...
  rows: ...
  frame_aspect: ...
  virtual_origin_left: ...
  virtual_origin_top: ...
```

### Projection

Current tokens:
- `@la_car_pano_or_tube_tube_pano`
- `@la_iC_alpha_degree`
- `@la_iC_beta_degree`

YAML target:

```yaml
projection:
  mode: car_pano | tube_pano
  ic_alpha_degree: ...
  ic_beta_degree: ...
```

### View Presets

Current token:
- `@pyz[a_row][a_col]`

YAML target:

```yaml
views:
  pyz:
    - [pitch, yaw, zoom]
```

Rules:
- keep exactly 6 rows in v1
- keep exactly 3 values per row
- preserve the current minimum total count expected by validation

### ROI

Current tokens:
- `@la_roi_car_pano_left`
- `@la_roi_car_pano_top`
- `@la_roi_car_pano_cols`
- `@la_roi_car_pano_rows`
- `@la_display_asp`
- `@la_rt_alpha_from`
- `@la_rt_alpha_to`

YAML target:

```yaml
roi:
  car_pano_left: ...
  car_pano_top: ...
  car_pano_cols: ...
  car_pano_rows: ...
  display_aspect: ...
  rt_alpha_from: ...
  rt_alpha_to: ...
```

### Tuning

Current tokens:
- `@alpha_range_vender` or `@alpha_range_vendor`
- `@alpha_pano_min`
- `@alpha_pano_max`
- `@beta_pano_begin`
- `@beta_pano_range`
- `@alpha_pano_min_init`
- `@alpha_pano_max_init`
- `@alpha_pipe_init`
- `@beta_pipe_init`
- `@zoom_pipe_init`
- `@pitch_car_init`
- `@yaw_car_init`
- `@zoom_car_init`

YAML target:

```yaml
tuning:
  alpha_range_vendor: ...
  alpha_pano_min: ...
  alpha_pano_max: ...
  beta_pano_begin: ...
  beta_pano_range: ...
  alpha_pano_min_init: ...
  alpha_pano_max_init: ...
  alpha_pipe_init: ...
  beta_pipe_init: ...
  zoom_pipe_init: ...
  pitch_car_init: ...
  yaw_car_init: ...
  zoom_car_init: ...
```

Rule:
- YAML uses the normalized spelling `alpha_range_vendor`

## Validation Rules For YAML V1

Required:
- `version`
- `camera.name`
- `camera.sensor.*`
- `camera.intrinsics.*`
- `paths.working_root`
- `paths.camera_folder`
- `paths.image_folder`
- `paths.image_file`
- `paths.mapper_folder`
- `media.type`
- `display.*`
- `projection.mode`
- `views.pyz`
- `roi.*`
- `tuning.*`

Additional rules:
- `version` must be `1`
- `display.cols` and `display.rows` must be positive
- `projection.mode` must be `car_pano` or `tube_pano`
- `media.type` must be one of:
  - `image`
  - `video_file`
  - `camera_usb`
  - `stream_http`
- `views.pyz` must contain 6 rows
- each `views.pyz` row must contain 3 numeric values
- absolute filesystem paths remain invalid except where explicitly allowed by placeholder expansion
- `@DROPBOX_ROOT@/...` remains allowed in path-bearing fields

## Path Policy

The YAML format should continue the current no-hardcoded-path direction.

Recommended policy:
- allow `@DROPBOX_ROOT@/...` for Dropbox-backed asset locations
- allow relative paths for folder and file fields where possible
- reject raw absolute filesystem paths such as `/home/cj/...`

Preferred examples:

```yaml
paths:
  working_root: "@DROPBOX_ROOT@/fic-092021/nppm@1T/nppm/"
  image_folder: entaniya-vr220/images/
  image_file: 20210909_dji_inspire2_lower.png
  mapper_folder: mapper/
```

```yaml
media:
  type: video_file
  source: "@DROPBOX_ROOT@/fic-092021/nppm@1T/nppm/entaniya-vr220/mp4/20210909_dji_inspire2_lower.mp4"
```

## Loader Plan

Recommended C++ interface:

```cpp
struct RecipeConfig;

RecipeConfig load_cast_recipe(const std::filesystem::path& path);
RecipeConfig load_yaml_recipe(const std::filesystem::path& path);
void validate_recipe_config_or_exit(const RecipeConfig& config,
                                    const std::filesystem::path& source_path);
```

Migration direction:
1. define `RecipeConfig`
2. make the current `.cast/.script` loader populate it
3. add YAML loader that populates the same struct
4. move validation from token-level parsing to struct-level validation
5. support `.yml` in `recipe_resolver` and the Python control server

## Converter Plan

Recommended first converter:

```text
.cast/.script -> .yml
```

Converter behavior:
- preserve field values exactly where possible
- normalize `@alpha_range_vender` to `alpha_range_vendor`
- convert `@pyz[a_row][a_col]` into `views.pyz`
- preserve `@DROPBOX_ROOT@/...`
- reject unsupported legacy Raspberry Pi media mode instead of writing bad YAML

## First Implementation Slice

The safest next coding step after this design is:

1. add `RecipeConfig`
2. refactor the current `.cast` parser to populate it
3. move current recipe validation to `RecipeConfig`
4. add a YAML parser behind a new `load_yaml_recipe(...)`

That keeps the migration incremental and reversible.
