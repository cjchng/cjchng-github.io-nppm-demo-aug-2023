#pragma once

#include "recipe_config.h"

#include <string>

bool is_yaml_recipe_path(const std::string &recipe_path);
RecipeConfig load_yaml_recipe_or_exit(const std::string &recipe_path);
