#pragma once

#include <string>

/**
 * Resolved runtime recipe information returned by the startup resolver.
 */
struct ResolvedRecipe {
    std::string recipe_path;
    std::string recipe_short_name;
    std::string metadata_root;
};

/**
 * Resolve the recipe that should be used for the current launch.
 */
ResolvedRecipe resolve_recipe(int argc, char *const *argv);
