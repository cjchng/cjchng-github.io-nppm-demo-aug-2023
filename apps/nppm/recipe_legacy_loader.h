#pragma once

#include "recipe_config.h"

#include <string>

bool is_legacy_recipe_path(const std::string &recipe_path);
RecipeConfig load_legacy_recipe_or_exit(const std::string &recipe_path);
