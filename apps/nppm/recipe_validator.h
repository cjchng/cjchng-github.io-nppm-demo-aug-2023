#pragma once

#include <string>

/**
 * Perform a lightweight preflight check on a runnable recipe file (.cast, .yml, or .yaml) and terminate
 * early with actionable errors when required markers are missing.
 */
void validate_recipe_file_or_exit(const std::string &recipe_path);
