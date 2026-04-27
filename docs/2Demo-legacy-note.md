# 2Demo.txt Legacy Note

`2Demo.txt` is a legacy recipe-list file under `defaultS/`.

## Current Status

Current project behavior:
- the Python control server does not use `2Demo.txt` as the source of the recipe list
- the browser UI lists real `*.cast`, `*.script`, and `*.yaml` files directly from `defaultS/`
- `x_mainmoil` prefers `active_recipes.yaml`
- `x_mainmoil` falls back to `active_recipe.txt`
- `x_mainmoil` still keeps `2Demo.txt` as a final legacy fallback if neither active file is available

## Preferred Mechanism

The preferred recipe selector is:

```text
active_recipes.yaml
```

That file stores per-type active state plus `default_kind`, for example:

```yaml
active:
  cast: "leo-fisheye@neihu-mp4-115.cast"
  yaml: "leo-fisheye@lab103.yaml"
default_kind: yaml
```

`active_recipe.txt` is still kept as a compatibility mirror, for example:

```text
leo-fisheye@neihu-mp4-115.cast
```

or:

```text
rasp+entaniya@oil_front(v2_image).script
```

## Recommendation

For current operation:
- use `active_recipes.yaml` to store the active recipe state
- allow `active_recipe.txt` to remain as the compatibility mirror
- use the Python control server to browse and switch recipes
- do not rely on `2Demo.txt` for normal day-to-day selection

## Removal Plan

`2Demo.txt` can be removed only after:
- all launch paths use `active_recipes.yaml`, `active_recipe.txt`, or explicit recipe selection
- the legacy fallback is removed from `apps/nppm/recipe_resolver.cpp`
- documentation is updated to remove the `2Demo.txt` fallback note

## Summary

`2Demo.txt` is now a compatibility file, not the main control mechanism.
