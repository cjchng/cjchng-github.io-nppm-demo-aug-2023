#include "recipe_resolver.h"

#include "tools.h"

#include <cstdlib>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

namespace {

bool is_runtime_recipe_extension(const std::filesystem::path &path) {
    const string extension = path.extension().string();
    return extension == ".cast" || extension == ".yml" || extension == ".yaml";
}

void reject_script_recipe_or_exit(const string &recipe_ref) {
    if (std::filesystem::path(recipe_ref).extension() != ".script") {
        return;
    }
    cerr << "Script recipes are no longer supported by x_mainmoil at runtime: [" << recipe_ref << "]" << endl;
    cerr << "Convert the script to YAML first, then run the generated .yaml recipe." << endl;
    exit(1);
}

/**
 * Normalize a candidate directory path so downstream path joins can assume a
 * trailing slash.
 */
string make_dir_path(string path) {
    if (!path.empty() && path.back() != '/') {
        path.push_back('/');
    }
    return path;
}

string trim_copy(const string &value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

string unquote_copy(const string &value) {
    const string trimmed = trim_copy(value);
    if (trimmed.size() >= 2 &&
        ((trimmed.front() == '"' && trimmed.back() == '"') ||
         (trimmed.front() == '\'' && trimmed.back() == '\''))) {
        return trimmed.substr(1, trimmed.size() - 2);
    }
    return trimmed;
}

/**
 * Resolve one explicit recipe reference.
 *
 * The reference may be:
 * - an absolute recipe path
 * - a repo-relative path
 * - a recipe filename relative to <metadata-root>/defaultS
 */
bool resolve_explicit_recipe_reference(const string &recipe_ref,
                                       const string &meta_root,
                                       ResolvedRecipe &resolved) {
    namespace fs = std::filesystem;

    if (recipe_ref.empty()) {
        return false;
    }

    const fs::path configured_path(recipe_ref);
    reject_script_recipe_or_exit(configured_path.string());
    if (configured_path.is_absolute() && utl::doesFileExist(configured_path.string())) {
        if (!is_runtime_recipe_extension(configured_path)) {
            return false;
        }
        resolved.recipe_path = configured_path.string();
        resolved.recipe_short_name = configured_path.filename().string();
        if (!meta_root.empty()) {
            resolved.metadata_root = make_dir_path(meta_root);
        }
        return true;
    }

    if (utl::doesFileExist(recipe_ref)) {
        const fs::path existing_path(recipe_ref);
        if (!is_runtime_recipe_extension(existing_path)) {
            return false;
        }
        resolved.recipe_path = existing_path.string();
        resolved.recipe_short_name = existing_path.filename().string();
        if (!meta_root.empty()) {
            resolved.metadata_root = make_dir_path(meta_root);
        }
        return true;
    }

    if (!meta_root.empty()) {
        const fs::path candidate =
                fs::path(make_dir_path(meta_root)) / "defaultS" / configured_path.filename();
        if (utl::doesFileExist(candidate.string())) {
            if (!is_runtime_recipe_extension(candidate)) {
                return false;
            }
            resolved.recipe_path = candidate.string();
            resolved.recipe_short_name = candidate.filename().string();
            resolved.metadata_root = make_dir_path(meta_root);
            return true;
        }
    }

    return false;
}

/**
 * Refresh the timestamped symlink that records which recipe file is currently
 * running.
 */
void link_recipe(const string &recipe_path) {
    namespace fs = std::filesystem;

    if (recipe_path.empty()) {
        return;
    }
    try {
        for (const auto &entry : fs::directory_iterator(fs::current_path())) {
            const string name = entry.path().filename().string();
            if (name.rfind("running_recipe@", 0) == 0) {
                fs::remove(entry.path());
            }
        }
        fs::create_symlink(recipe_path, "running_recipe@" + utl::now_as_string());
    } catch (const std::exception &ex) {
        cerr << "Warning: failed to create running_recipe symlink: " << ex.what() << endl;
    }
}

/**
 * Refresh a stable runtime symlink such as ln-defaultS or ln-2Demo.txt so it
 * points at the resolved metadata-root content for this launch.
 */
void update_named_symlink(const std::filesystem::path &target_path, const string &link_name) {
    namespace fs = std::filesystem;

    if (target_path.empty() || !fs::exists(target_path)) {
        return;
    }
    try {
        fs::path link_path = fs::current_path() / "apps/nppm" / link_name;
        if (!fs::exists(link_path.parent_path())) {
            link_path = fs::current_path() / link_name;
        }
        if (fs::exists(link_path) || fs::is_symlink(link_path)) {
            fs::remove(link_path);
        }
        fs::create_symlink(target_path, link_path);
        tLog("Updated runtime symlink", link_name, link_path.string());
    } catch (const std::exception &ex) {
        cerr << "Warning: failed to create " << link_name << " symlink: " << ex.what() << endl;
    }
}

/**
 * Try to resolve a runnable recipe from one metadata root.
 *
 * The preferred source is defaultS/active_recipes.yaml. If that file is absent,
 * the resolver falls back to defaultS/active_recipe.txt and finally the legacy
 * defaultS/2Demo.txt format. Once a root is accepted, this function also
 * refreshes the metadata-related runtime symlinks.
 */
bool select_from_recipe_list(const string &meta_root, ResolvedRecipe &resolved) {
    namespace fs = std::filesystem;

    const string default_dir = make_dir_path(meta_root) + "defaultS/";
    const string active_recipes_file = default_dir + "active_recipes.yaml";
    const string active_recipe_file = default_dir + "active_recipe.txt";
    const string recipe_list = default_dir + "2Demo.txt";
    tLog("Recipe meta root", "meta_root", meta_root);
    tLog("Active recipes file", "active_recipes_file", active_recipes_file);
    tLog("Active recipe file", "active_recipe_file", active_recipe_file);
    tLog("Recipe list", "recipe_list", recipe_list);

    const fs::path meta_root_path(make_dir_path(meta_root));
    if (fs::is_directory(meta_root_path)) {
        update_named_symlink(meta_root_path, "ln-nppm-meta");
    }
    const fs::path default_dir_path(default_dir);
    if (fs::is_directory(default_dir_path)) {
        update_named_symlink(default_dir_path, "ln-defaultS");
    }
    if (utl::doesFileExist(active_recipes_file)) {
        update_named_symlink(fs::path(active_recipes_file), "ln-active_recipes.yaml");
    }
    if (utl::doesFileExist(active_recipe_file)) {
        update_named_symlink(fs::path(active_recipe_file), "ln-active_recipe.txt");
    }
    if (utl::doesFileExist(recipe_list)) {
        update_named_symlink(fs::path(recipe_list), "ln-2Demo.txt");
    }

    if (utl::doesFileExist(active_recipes_file)) {
        ifstream active_state_ifstream(active_recipes_file);
        string line;
        string current_section;
        string default_kind = "yaml";
        string active_cast;
        string active_yaml;

        while (getline(active_state_ifstream, line)) {
            const string trimmed = trim_copy(line);
            if (trimmed.empty() || trimmed[0] == '#') {
                continue;
            }
            if (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
                if (current_section == "active") {
                    const size_t sep = trimmed.find(':');
                    if (sep == string::npos) {
                        continue;
                    }
                    const string key = trim_copy(trimmed.substr(0, sep));
                    const string value = unquote_copy(trimmed.substr(sep + 1));
                    if (key == "cast") {
                        active_cast = value;
                    } else if (key == "yaml") {
                        active_yaml = value;
                    }
                }
                continue;
            }

            const size_t sep = trimmed.find(':');
            if (sep == string::npos) {
                continue;
            }
            const string key = trim_copy(trimmed.substr(0, sep));
            const string value = unquote_copy(trimmed.substr(sep + 1));
            if (key == "active") {
                current_section = "active";
            } else if (key == "default_kind") {
                if (value == "cast" || value == "yaml") {
                    default_kind = value;
                }
                current_section.clear();
            } else {
                current_section.clear();
            }
        }

        const array<pair<string, string>, 2> active_by_kind = {{
            {"cast", active_cast},
            {"yaml", active_yaml},
        }};
        array<string, 2> preferred_order = {default_kind, "cast"};
        if (default_kind == "cast") {
            preferred_order = {"cast", "yaml"};
        } else {
            preferred_order = {"yaml", "cast"};
        }

        for (const string &kind : preferred_order) {
            for (const auto &entry : active_by_kind) {
                if (entry.first != kind || entry.second.empty()) {
                    continue;
                }
                if (!resolve_explicit_recipe_reference(entry.second, meta_root, resolved)) {
                    continue;
                }
                link_recipe(resolved.recipe_path);
                tLog("Using recipe from active_recipes.yaml", "ing_cast_fname", resolved.recipe_path);
                return true;
            }
        }

        cerr << "No runnable recipe entry found in [" << active_recipes_file << "]" << endl;
        exit(1);
    }

    if (utl::doesFileExist(active_recipe_file)) {
        ifstream active_ifstream(active_recipe_file);
        string active_recipe_name;
        while (getline(active_ifstream, active_recipe_name)) {
            active_recipe_name = trim_copy(active_recipe_name);
            if (active_recipe_name.empty() || active_recipe_name[0] == '#') {
                continue;
            }
            if (!resolve_explicit_recipe_reference(active_recipe_name, meta_root, resolved)) {
                cerr << "Active recipe file points to a missing recipe: [" << active_recipe_name << "]" << endl;
                exit(1);
            }
            link_recipe(resolved.recipe_path);
            tLog("Using recipe from active_recipe.txt", "ing_cast_fname", resolved.recipe_path);
            return true;
        }

        cerr << "No runnable recipe entry found in [" << active_recipe_file << "]" << endl;
        exit(1);
    }

    if (!utl::doesFileExist(recipe_list)) {
        return false;
    }

    ifstream a_ifstream(recipe_list);
    string aline;
    while (getline(a_ifstream, aline)) {
        if (aline.empty() || aline[0] != '_') {
            continue;
        }
        resolved.recipe_short_name = aline;
        const size_t aSeq = aline.find_first_of(' ');
        if (aSeq != 3 || aSeq + 1 >= aline.size()) {
            cerr << "Format Error in recipe list [" << recipe_list << "]: " << aline << endl;
            exit(1);
        }

        resolved.recipe_path = default_dir + aline.substr(aSeq + 1);
        resolved.metadata_root = make_dir_path(meta_root);
        if (!utl::doesFileExist(resolved.recipe_path)) {
            cerr << "Listed recipe file does not exist: [" << resolved.recipe_path << "]" << endl;
            exit(1);
        }
        link_recipe(resolved.recipe_path);
        tLog("A recipe file is selected", "ing_cast_fname", resolved.recipe_path);
        return true;
    }
    cerr << "No runnable recipe entry found in [" << recipe_list << "]" << endl;
    exit(1);
}

/**
 * Normalize and probe one metadata-root candidate.
 *
 * Returns true once a recipe has been resolved from that root.
 */
bool try_meta_root(const string &meta_root, ResolvedRecipe &resolved) {
    if (meta_root.empty()) {
        return false;
    }
    const string normalized = make_dir_path(meta_root);
    cout << "Your NPPM_META is: " << normalized << '\n';
    return select_from_recipe_list(normalized, resolved);
}

/**
 * Resolve an explicit environment-driven recipe override.
 *
 * CJ_NPPM_RECIPE is intentionally strict: once set, it must resolve to a valid
 * recipe path. Supported forms are an absolute recipe path, an existing relative
 * path, or a filename that exists under <metadata-root>/defaultS.
 */
bool try_env_recipe(const string &recipe_ref, ResolvedRecipe &resolved) {
    namespace fs = std::filesystem;

    if (recipe_ref.empty()) {
        return false;
    }

    const char *env_meta = getenv("CJ_NPPM_META");
    if (env_meta != nullptr && resolve_explicit_recipe_reference(recipe_ref, env_meta, resolved)) {
        return true;
    }

    const fs::path local_meta = fs::current_path() / ".nppm-meta";
    if (fs::exists(local_meta) && resolve_explicit_recipe_reference(recipe_ref, local_meta.string(), resolved)) {
        return true;
    }

    const char *home = getenv("HOME");
    if (home != nullptr) {
        const fs::path usb_dropbox_meta = fs::path(home) / "usb_1T_disk/Dropbox/19-22-ing/.nppm-meta";
        if (fs::exists(usb_dropbox_meta) &&
            resolve_explicit_recipe_reference(recipe_ref, usb_dropbox_meta.string(), resolved)) {
            return true;
        }
    }

    const fs::path legacy_dropbox_meta = "/home/cj/Dropbox/19-22-ing/.nppm-meta";
    if (fs::exists(legacy_dropbox_meta) &&
        resolve_explicit_recipe_reference(recipe_ref, legacy_dropbox_meta.string(), resolved)) {
        return true;
    }

    return resolve_explicit_recipe_reference(recipe_ref, "", resolved);
}

/**
 * Print guidance for users when CJ_NPPM_RECIPE is set but does not resolve.
 */
void print_env_recipe_help(const string &recipe_ref) {
    namespace fs = std::filesystem;

    cerr << "CJ_NPPM_RECIPE did not resolve to a valid recipe: [" << recipe_ref << "]" << endl;
    cerr << "Use one of these forms:" << endl;
    cerr << "  1. Absolute recipe path (.cast, .yml, or .yaml)" << endl;
    cerr << "     Example: CJ_NPPM_RECIPE=/full/path/to/example.cast" << endl;
    cerr << "     Example: CJ_NPPM_RECIPE=/full/path/to/example.yml" << endl;
    cerr << "  2. Existing relative path from the current working directory" << endl;
    cerr << "     Example: CJ_NPPM_RECIPE=apps/nppm/example.cast" << endl;
    cerr << "     Example: CJ_NPPM_RECIPE=apps/nppm/example.yml" << endl;
    cerr << "  3. Recipe filename under <metadata-root>/defaultS/" << endl;
    cerr << "     Example: CJ_NPPM_RECIPE=narl@images_lab113_38-18.cast" << endl;
    cerr << "     Example: CJ_NPPM_RECIPE=narl@images_lab113_38-18.yml" << endl;
    cerr << "Script recipes are no longer runnable by x_mainmoil. Convert them to YAML first." << endl;
    cerr << endl;

    const char *env_meta = getenv("CJ_NPPM_META");
    if (env_meta != nullptr) {
        cerr << "Current CJ_NPPM_META: " << env_meta << endl;
        cerr << "Expected filename location: " << make_dir_path(env_meta) << "defaultS/" << endl;
    } else {
        cerr << "CJ_NPPM_META is not set." << endl;
        cerr << "The resolver will try these metadata roots for filename-only values:" << endl;

        const fs::path local_meta = fs::current_path() / ".nppm-meta";
        cerr << "  - " << local_meta.string();
        if (!fs::exists(local_meta)) {
            cerr << " (not found)";
        }
        cerr << endl;

        const char *home = getenv("HOME");
        if (home != nullptr) {
            const fs::path usb_dropbox_meta = fs::path(home) / "usb_1T_disk/Dropbox/19-22-ing/.nppm-meta";
            cerr << "  - " << usb_dropbox_meta.string();
            if (!fs::exists(usb_dropbox_meta)) {
                cerr << " (not found)";
            }
            cerr << endl;
        }

        const fs::path legacy_dropbox_meta = "/home/cj/Dropbox/19-22-ing/.nppm-meta";
        cerr << "  - " << legacy_dropbox_meta.string();
        if (!fs::exists(legacy_dropbox_meta)) {
            cerr << " (not found)";
        }
        cerr << endl;
    }
}

}

/**
 * Resolve the recipe used for this launch.
 *
 * Search order:
 * 1. argv[1]
 * 2. CJ_NPPM_RECIPE
 * 3. CJ_NPPM_META
 * 4. repo-local .nppm-meta
 * 5. USB-backed Dropbox metadata root
 * 6. legacy Dropbox metadata root
 */
ResolvedRecipe resolve_recipe(int argc, char *const *argv) {
    namespace fs = std::filesystem;

    ResolvedRecipe resolved;

    if (argc > 1 && argv[1] != nullptr) {
        if (!try_env_recipe(argv[1], resolved)) {
            cerr << "Explicit recipe argument did not resolve to a valid recipe: [" << argv[1] << "]" << endl;
            cerr << "Use an absolute .cast/.yaml recipe path, an existing relative .cast/.yaml path, or a .cast/.yaml filename under defaultS/." << endl;
            exit(1);
        }
        link_recipe(resolved.recipe_path);
        tLog("Using recipe from argv", "ing_cast_fname", resolved.recipe_path);
        return resolved;
    }

    const char *env_recipe = getenv("CJ_NPPM_RECIPE");
    if (env_recipe != nullptr) {
        if (!try_env_recipe(env_recipe, resolved)) {
            print_env_recipe_help(env_recipe);
            exit(1);
        }
        link_recipe(resolved.recipe_path);
        tLog("Using recipe from CJ_NPPM_RECIPE", "ing_cast_fname", resolved.recipe_path);
        return resolved;
    }

    const char *env_meta = getenv("CJ_NPPM_META");
    if (env_meta != nullptr && try_meta_root(env_meta, resolved)) {
        return resolved;
    }

    const fs::path local_meta = fs::current_path() / ".nppm-meta";
    if (fs::exists(local_meta) && try_meta_root(local_meta.string(), resolved)) {
        return resolved;
    }

    const char *home = getenv("HOME");
    if (home != nullptr) {
        const fs::path usb_dropbox_meta = fs::path(home) / "usb_1T_disk/Dropbox/19-22-ing/.nppm-meta";
        if (fs::exists(usb_dropbox_meta) && try_meta_root(usb_dropbox_meta.string(), resolved)) {
            return resolved;
        }
    }

    const fs::path legacy_dropbox_meta = "/home/cj/Dropbox/19-22-ing/.nppm-meta";
    if (fs::exists(legacy_dropbox_meta) && try_meta_root(legacy_dropbox_meta.string(), resolved)) {
        return resolved;
    }

    cerr << "Unable to locate a runtime recipe." << endl;
    cerr << "Provide a recipe file as argv[1], set CJ_NPPM_RECIPE, or set CJ_NPPM_META to a metadata root containing defaultS/active_recipes.yaml, defaultS/active_recipe.txt, or defaultS/2Demo.txt." << endl;
    cerr << "Runnable recipe types are .cast and .yaml." << endl;
    exit(1);
}
