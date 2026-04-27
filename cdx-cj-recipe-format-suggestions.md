# Recipe Format Suggestions

## Goal

Reduce recipe-loading errors and make recipe configuration easier to validate and maintain.

## Recommended Path

### 1. Keep `.cast`, add validation first

This is the lowest-risk move.

Add a preflight validator that checks before startup:
- required tokens exist
- referenced image and video files exist
- mapper folder exists or is creatable
- numeric fields parse correctly
- camera name is valid
- no duplicate required keys
- paths are normalized
- clear error messages include line numbers

This gives immediate value without breaking old recipes.

### 2. Add a strict parser layer

Current loading is too stream-driven and implicit. Improve it by:
- reading the whole file first
- parsing into a structured `RecipeConfig`
- validating that struct
- only then starting the app

This prevents half-loaded state.

### 3. Add a recipe doctor tool

Examples:

```bash
./build/apps/nppm/x_mainmoil --check-recipe /path/to/file.cast
```

or:

```bash
tools/check_recipe.sh /path/to/file.cast
```

Output should say:
- `PASS`
- or exact failures with fixes

### 4. Prefer `active_recipe.txt` only for selection

Use:
- `active_recipe.txt` = which recipe to run
- recipe file = actual configuration

Do not keep expanding `2Demo.txt` beyond legacy fallback.

### 5. Migrate from `.cast` to YAML gradually

The current `.cast` format is fragile enough that replacing it long-term is reasonable.

## Recommendation

For this repo:
- keep recipe files themselves in **YAML**
- keep the schema constrained and versioned

Reason:
- the current recipes are operator-edited configuration files
- YAML is easier to read, comment, and maintain than the current token stream
- the same internal validation rules can still be enforced after loading a YAML document into a structured config object

## Best Migration Path

Do not replace `.cast` in one step.

Use this sequence:

1. Define `RecipeConfig` in C++
2. Implement loader interface:
   - `load_cast_recipe(...)`
   - `load_yaml_recipe(...)`
3. Convert both into the same internal struct
4. Add validation on the struct
5. Prefer `.yml` for new recipes
6. Keep `.cast` support temporarily
7. Add a converter:
   - `.cast` -> `.yml`

This avoids breaking all existing recipes at once.

## Example YAML Shape

```yaml
version: 1

camera:
  name: narl

paths:
  working_root: "@DROPBOX_ROOT@/fic-092021/nppm@1T/nppm/"
  image_folder: image/
  image_file: sample.png
  mapper_folder: mapper/

media:
  type: image
  usb_camera: false
  source: ""

display:
  title: NPPM
  cols: 1920
  rows: 1080

car_view:
  pitch: 0
  yaw: 0
  zoom: 1
```

Then validate:
- required keys present
- types correct
- files exist
- unsupported keys rejected

## Immediate Priority

Do not start with a full format switch.

First:
- add validation and error reporting to the current `.cast` loader

Second:
- introduce YAML recipe support beside `.cast`

This gives safety now and modernization next.

## Bottom Line

- yes, moving away from `.cast` is reasonable
- but the right first step is not abandonment
- the right first step is:
  - structured internal config
  - validation
  - preflight checks
  - then gradual YAML migration
