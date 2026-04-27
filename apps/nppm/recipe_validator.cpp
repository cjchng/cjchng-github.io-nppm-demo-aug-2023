#include "recipe_validator.h"
#include "recipe_yaml.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace {

size_t count_occurrences(const string &text, const string &needle) {
    if (needle.empty()) {
        return 0;
    }

    size_t count = 0;
    size_t pos = 0;
    while ((pos = text.find(needle, pos)) != string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

bool is_skippable_line(const string &line) {
    return line.empty() ||
           line[0] == '#' ||
           line[0] == '*' ||
           line[0] == '{' ||
           line[0] == '}' ||
           line[0] == '$';
}

string trim_copy(const string &value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

bool looks_like_url(const string &value) {
    return value.find("://") != string::npos || value.rfind("//", 0) == 0;
}

bool is_filesystem_absolute_path(const string &value) {
    return !value.empty() && value[0] == '/' && !looks_like_url(value);
}

vector<string> find_hardcoded_path_violations(const vector<string> &lines) {
    const map<string, string> path_fields = {
            {"@la_nppm_ing_abs_folder", "Use @DROPBOX_ROOT@/... or a relative recipe root instead."},
            {"@la_using_image_short_folder", "Use a relative folder such as images/ instead of an absolute path."},
            {"@la_image_inst_short_file", "Keep this as a filename or relative path, not an absolute filesystem path."},
            {"@la_mappers_folder_short", "Use a relative folder such as mapper/ instead of an absolute path."},
            {"@la_video_stream_uri", "Use @DROPBOX_ROOT@/... for local Dropbox-backed files, or use a direct stream URL."}
    };

    vector<string> violations;
    for (size_t i = 0; i < lines.size(); ++i) {
        const string token = trim_copy(lines[i]);
        const auto field = path_fields.find(token);
        if (field == path_fields.end()) {
            continue;
        }

        for (size_t j = i + 1; j < lines.size(); ++j) {
            const string value = trim_copy(lines[j]);
            if (is_skippable_line(value)) {
                continue;
            }
            if (value.rfind("@DROPBOX_ROOT@/", 0) == 0) {
                break;
            }
            if (is_filesystem_absolute_path(value)) {
                ostringstream oss;
                oss << token << " uses a hardcoded absolute path at line " << (j + 1)
                    << ": [" << value << "] "
                    << field->second;
                violations.push_back(oss.str());
            }
            break;
        }
    }

    return violations;
}

}

void validate_recipe_file_or_exit(const string &recipe_path) {
    if (std::filesystem::path(recipe_path).extension() == ".script") {
        cerr << "Script recipes are no longer supported by x_mainmoil at runtime: [" << recipe_path << "]" << endl;
        cerr << "Convert the script to YAML first, then run or check the generated .yaml recipe." << endl;
        exit(1);
    }

    if (is_yaml_recipe_path(recipe_path)) {
        load_yaml_recipe_or_exit(recipe_path);
        return;
    }

    ifstream input(recipe_path);
    if (!input.is_open()) {
        cerr << "Cannot open recipe file for validation: [" << recipe_path << "]" << endl;
        exit(1);
    }

    const string content((istreambuf_iterator<char>(input)), istreambuf_iterator<char>());
    if (content.empty()) {
        cerr << "Recipe file is empty: [" << recipe_path << "]" << endl;
        exit(1);
    }

    vector<string> lines;
    {
        istringstream stream(content);
        string line;
        while (getline(stream, line)) {
            lines.push_back(line);
        }
    }

    const vector<string> required_single_markers = {
            "@cast_version",
            "@md->cfg->cameraName",
            "@alpha_pano_min",
            "@alpha_pano_max",
            "@beta_pano_begin",
            "@beta_pano_range",
            "@alpha_pano_min_init",
            "@alpha_pano_max_init",
            "@alpha_pipe_init",
            "@beta_pipe_init",
            "@zoom_pipe_init",
            "@pitch_car_init",
            "@yaw_car_init",
            "@zoom_car_init",
            "@VIRTUAL_ORGIN_LEFT",
            "@VIRTUAL_ORGIN_TOP",
            "@la_nppm_ing_abs_folder",
            "@la_camera_name_as_inst_short_folder",
            "@la_using_image_short_folder",
            "@la_image_inst_short_file",
            "@la_car_pano_or_tube_tube_pano",
            "@la_mappers_folder_short",
            "@la_screen_title",
            "@la_display_cols",
            "@la_display_rows",
            "@la_vir_frame_asp",
            "@la_is_camera_ready",
            "@la_is_usb_camera",
            "@la_video_stream_uri",
            "@la_iC_alpha_degree",
            "@la_iC_beta_degree",
            "@la_roi_car_pano_left",
            "@la_roi_car_pano_top",
            "@la_roi_car_pano_cols",
            "@la_roi_car_pano_rows",
            "@la_display_asp",
            "@la_rt_alpha_from",
            "@la_rt_alpha_to"
    };

    vector<string> missing_markers;
    for (const string &marker : required_single_markers) {
        if (content.find(marker) == string::npos) {
            missing_markers.push_back(marker);
        }
    }

    if (content.find("@alpha_range_vender") == string::npos &&
        content.find("@alpha_range_vendor") == string::npos) {
        missing_markers.push_back("@alpha_range_vender or @alpha_range_vendor");
    }

    const size_t pyz_count = count_occurrences(content, "@pyz[a_row][a_col]");
    if (pyz_count < 18) {
        ostringstream oss;
        oss << "@pyz[a_row][a_col] expected at least 18 entries but found " << pyz_count;
        missing_markers.push_back(oss.str());
    }

    const vector<string> hardcoded_path_violations = find_hardcoded_path_violations(lines);

    if (!missing_markers.empty() || !hardcoded_path_violations.empty()) {
        cerr << "Recipe preflight validation failed for [" << recipe_path << "]" << endl;
        if (!missing_markers.empty()) {
            cerr << "Missing or incomplete required markers:" << endl;
            for (const string &marker : missing_markers) {
                cerr << "  - " << marker << endl;
            }
        }
        if (!hardcoded_path_violations.empty()) {
            cerr << "Hardcoded filesystem paths are not allowed:" << endl;
            for (const string &violation : hardcoded_path_violations) {
                cerr << "  - " << violation << endl;
            }
            cerr << "Use @DROPBOX_ROOT@/... for Dropbox-backed paths, or use relative paths where possible." << endl;
        }
        cerr << "Fix the recipe file before running the app." << endl;
        exit(1);
    }
}
