#include "recipe_yaml.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

namespace {

string trim_copy(const string &value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

string strip_comment(const string &line) {
    bool in_single = false;
    bool in_double = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (c == '\'' && !in_double) {
            in_single = !in_single;
        } else if (c == '"' && !in_single) {
            in_double = !in_double;
        } else if (c == '#' && !in_single && !in_double) {
            return line.substr(0, i);
        }
    }
    return line;
}

string unquote_scalar(string value) {
    value = trim_copy(value);
    if (value.size() >= 2 &&
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

bool looks_like_url(const string &value) {
    return value.find("://") != string::npos || value.rfind("//", 0) == 0;
}

bool is_filesystem_absolute_path(const string &value) {
    return !value.empty() && value[0] == '/' && !looks_like_url(value);
}

double parse_double_or_exit(const string &value, const string &field_name) {
    try {
        size_t used = 0;
        const double parsed = stod(value, &used);
        if (used != value.size()) {
            throw invalid_argument("trailing characters");
        }
        return parsed;
    } catch (const std::exception &) {
        cerr << "Invalid numeric value for " << field_name << ": [" << value << "]" << endl;
        exit(1);
    }
}

int parse_int_or_exit(const string &value, const string &field_name) {
    try {
        size_t used = 0;
        const int parsed = stoi(value, &used);
        if (used != value.size()) {
            throw invalid_argument("trailing characters");
        }
        return parsed;
    } catch (const std::exception &) {
        cerr << "Invalid integer value for " << field_name << ": [" << value << "]" << endl;
        exit(1);
    }
}

bool parse_bool_or_exit(string value, const string &field_name) {
    transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
    if (value == "true" || value == "1" || value == "yes") {
        return true;
    }
    if (value == "false" || value == "0" || value == "no") {
        return false;
    }
    cerr << "Invalid boolean value for " << field_name << ": [" << value << "]" << endl;
    exit(1);
}

vector<double> parse_inline_double_list_or_exit(string value, const string &field_name) {
    value = trim_copy(value);
    if (value.size() < 2 || value.front() != '[' || value.back() != ']') {
        cerr << "Expected inline list syntax for " << field_name << ": [" << value << "]" << endl;
        exit(1);
    }

    value = value.substr(1, value.size() - 2);
    vector<double> parsed;
    string item;
    stringstream ss(value);
    while (getline(ss, item, ',')) {
        item = trim_copy(item);
        if (!item.empty()) {
            parsed.push_back(parse_double_or_exit(item, field_name));
        }
    }
    return parsed;
}

string require_scalar(const map<string, string> &scalars, const string &field_name) {
    const auto it = scalars.find(field_name);
    if (it == scalars.end() || it->second.empty()) {
        cerr << "Missing required YAML field: " << field_name << endl;
        exit(1);
    }
    return it->second;
}

vector<double> require_array(const map<string, vector<double>> &arrays, const string &field_name, size_t expected_size) {
    const auto it = arrays.find(field_name);
    if (it == arrays.end()) {
        cerr << "Missing required YAML array field: " << field_name << endl;
        exit(1);
    }
    if (it->second.size() != expected_size) {
        cerr << "YAML array " << field_name << " must contain " << expected_size
             << " values, but found " << it->second.size() << endl;
        exit(1);
    }
    return it->second;
}

void validate_yaml_paths_or_exit(const RecipeConfig &config) {
    const auto forbid_absolute = [](const string &value, const string &field_name, bool allow_dropbox_root, bool allow_url) {
        if (value.empty()) {
            return;
        }
        if (allow_dropbox_root && value.rfind("@DROPBOX_ROOT@/", 0) == 0) {
            return;
        }
        if (allow_url && looks_like_url(value)) {
            return;
        }
        if (is_filesystem_absolute_path(value)) {
            cerr << field_name << " uses a hardcoded absolute path: [" << value << "]" << endl;
            cerr << "Use @DROPBOX_ROOT@/... or a relative path instead." << endl;
            exit(1);
        }
    };

    forbid_absolute(config.paths.working_root, "paths.working_root", true, false);
    forbid_absolute(config.paths.image_folder, "paths.image_folder", false, false);
    forbid_absolute(config.paths.image_file, "paths.image_file", false, false);
    forbid_absolute(config.paths.mapper_folder, "paths.mapper_folder", false, false);
    forbid_absolute(config.media.source, "media.source", true, true);
}

}

bool is_yaml_recipe_path(const std::string &recipe_path) {
    const auto dot = recipe_path.find_last_of('.');
    if (dot == string::npos) {
        return false;
    }
    string suffix = recipe_path.substr(dot);
    transform(suffix.begin(), suffix.end(), suffix.begin(), [](unsigned char c) { return static_cast<char>(tolower(c)); });
    return suffix == ".yml" || suffix == ".yaml";
}

RecipeConfig load_yaml_recipe_or_exit(const string &recipe_path) {
    ifstream input(recipe_path);
    if (!input.is_open()) {
        cerr << "Cannot open YAML recipe: [" << recipe_path << "]" << endl;
        exit(1);
    }

    map<string, string> scalars;
    map<string, vector<double>> arrays;
    vector<array<double, 3>> pyz_rows;
    vector<pair<int, string>> sections;

    string raw_line;
    int line_number = 0;
    while (getline(input, raw_line)) {
        ++line_number;
        const string without_comment = strip_comment(raw_line);
        const string trimmed = trim_copy(without_comment);
        if (trimmed.empty()) {
            continue;
        }

        const size_t indent = without_comment.find_first_not_of(' ');
        const int indent_width = indent == string::npos ? 0 : static_cast<int>(indent);

        if (trimmed.rfind("- ", 0) == 0) {
            if (sections.empty() || sections.back().second != "views.pyz") {
                cerr << "Unexpected YAML list item at line " << line_number << ": " << trimmed << endl;
                exit(1);
            }
            const vector<double> row = parse_inline_double_list_or_exit(trimmed.substr(2), "views.pyz");
            if (row.size() != 3) {
                cerr << "Each views.pyz entry must contain exactly 3 values." << endl;
                exit(1);
            }
            pyz_rows.push_back({row[0], row[1], row[2]});
            continue;
        }

        const size_t colon = trimmed.find(':');
        if (colon == string::npos) {
            cerr << "Malformed YAML line " << line_number << ": " << trimmed << endl;
            exit(1);
        }

        while (!sections.empty() && indent_width <= sections.back().first) {
            sections.pop_back();
        }

        const string key = trim_copy(trimmed.substr(0, colon));
        const string value = trim_copy(trimmed.substr(colon + 1));
        const string full_key = sections.empty() ? key : sections.back().second + "." + key;

        if (value.empty()) {
            sections.push_back({indent_width, full_key});
            continue;
        }

        const string normalized_value = unquote_scalar(value);
        if (!normalized_value.empty() && normalized_value.front() == '[' && normalized_value.back() == ']') {
            arrays[full_key] = parse_inline_double_list_or_exit(normalized_value, full_key);
        } else {
            scalars[full_key] = normalized_value;
        }
    }

    RecipeConfig config;
    config.version = parse_int_or_exit(require_scalar(scalars, "version"), "version");
    if (config.version != 1) {
        cerr << "Unsupported YAML recipe version: " << config.version << endl;
        exit(1);
    }

    config.camera.name = require_scalar(scalars, "camera.name");
    config.camera.sensor_width = parse_double_or_exit(require_scalar(scalars, "camera.sensor.width"), "camera.sensor.width");
    config.camera.sensor_height = parse_double_or_exit(require_scalar(scalars, "camera.sensor.height"), "camera.sensor.height");
    config.camera.icx = parse_double_or_exit(require_scalar(scalars, "camera.intrinsics.icx"), "camera.intrinsics.icx");
    config.camera.icy = parse_double_or_exit(require_scalar(scalars, "camera.intrinsics.icy"), "camera.intrinsics.icy");
    config.camera.ratio = parse_double_or_exit(require_scalar(scalars, "camera.intrinsics.ratio"), "camera.intrinsics.ratio");
    config.camera.image_width = parse_double_or_exit(require_scalar(scalars, "camera.intrinsics.image_width"), "camera.intrinsics.image_width");
    config.camera.image_height = parse_double_or_exit(require_scalar(scalars, "camera.intrinsics.image_height"), "camera.intrinsics.image_height");
    config.camera.calibration_ratio = parse_double_or_exit(require_scalar(scalars, "camera.intrinsics.calibration_ratio"), "camera.intrinsics.calibration_ratio");
    {
        const vector<double> para = require_array(arrays, "camera.intrinsics.para", 6);
        copy(para.begin(), para.end(), config.camera.para.begin());
    }

    config.paths.working_root = require_scalar(scalars, "paths.working_root");
    config.paths.camera_folder = require_scalar(scalars, "paths.camera_folder");
    config.paths.image_folder = require_scalar(scalars, "paths.image_folder");
    config.paths.image_file = require_scalar(scalars, "paths.image_file");
    config.paths.mapper_folder = require_scalar(scalars, "paths.mapper_folder");

    const string media_type = require_scalar(scalars, "media.type");
    if (media_type == "image") {
        config.media.type = RecipeMediaType::IMAGE;
    } else if (media_type == "video_file") {
        config.media.type = RecipeMediaType::VIDEO_FILE;
    } else if (media_type == "camera_usb") {
        config.media.type = RecipeMediaType::CAMERA_USB;
    } else if (media_type == "stream_http") {
        config.media.type = RecipeMediaType::STREAM_HTTP;
    } else {
        cerr << "Unsupported YAML media.type: [" << media_type << "]" << endl;
        exit(1);
    }
    config.media.usb_camera = parse_bool_or_exit(require_scalar(scalars, "media.usb_camera"), "media.usb_camera");
    const auto media_source = scalars.find("media.source");
    config.media.source = media_source == scalars.end() ? "" : media_source->second;

    config.display.title = require_scalar(scalars, "display.title");
    config.display.cols = parse_int_or_exit(require_scalar(scalars, "display.cols"), "display.cols");
    config.display.rows = parse_int_or_exit(require_scalar(scalars, "display.rows"), "display.rows");
    config.display.frame_aspect = parse_double_or_exit(require_scalar(scalars, "display.frame_aspect"), "display.frame_aspect");
    config.display.virtual_origin_left = parse_int_or_exit(require_scalar(scalars, "display.virtual_origin_left"), "display.virtual_origin_left");
    config.display.virtual_origin_top = parse_int_or_exit(require_scalar(scalars, "display.virtual_origin_top"), "display.virtual_origin_top");

    config.projection.mode = require_scalar(scalars, "projection.mode");
    config.projection.ic_alpha_degree = parse_double_or_exit(require_scalar(scalars, "projection.ic_alpha_degree"), "projection.ic_alpha_degree");
    config.projection.ic_beta_degree = parse_double_or_exit(require_scalar(scalars, "projection.ic_beta_degree"), "projection.ic_beta_degree");

    config.roi.car_pano_left = parse_double_or_exit(require_scalar(scalars, "roi.car_pano_left"), "roi.car_pano_left");
    config.roi.car_pano_top = parse_double_or_exit(require_scalar(scalars, "roi.car_pano_top"), "roi.car_pano_top");
    config.roi.car_pano_cols = parse_double_or_exit(require_scalar(scalars, "roi.car_pano_cols"), "roi.car_pano_cols");
    config.roi.car_pano_rows = parse_double_or_exit(require_scalar(scalars, "roi.car_pano_rows"), "roi.car_pano_rows");
    config.roi.display_aspect = parse_double_or_exit(require_scalar(scalars, "roi.display_aspect"), "roi.display_aspect");
    config.roi.rt_alpha_from = parse_double_or_exit(require_scalar(scalars, "roi.rt_alpha_from"), "roi.rt_alpha_from");
    config.roi.rt_alpha_to = parse_double_or_exit(require_scalar(scalars, "roi.rt_alpha_to"), "roi.rt_alpha_to");

    config.tuning.alpha_range_vendor = parse_double_or_exit(require_scalar(scalars, "tuning.alpha_range_vendor"), "tuning.alpha_range_vendor");
    config.tuning.alpha_pano_min = parse_double_or_exit(require_scalar(scalars, "tuning.alpha_pano_min"), "tuning.alpha_pano_min");
    config.tuning.alpha_pano_max = parse_double_or_exit(require_scalar(scalars, "tuning.alpha_pano_max"), "tuning.alpha_pano_max");
    config.tuning.beta_pano_begin = parse_double_or_exit(require_scalar(scalars, "tuning.beta_pano_begin"), "tuning.beta_pano_begin");
    config.tuning.beta_pano_range = parse_double_or_exit(require_scalar(scalars, "tuning.beta_pano_range"), "tuning.beta_pano_range");
    config.tuning.alpha_pano_min_init = parse_double_or_exit(require_scalar(scalars, "tuning.alpha_pano_min_init"), "tuning.alpha_pano_min_init");
    config.tuning.alpha_pano_max_init = parse_double_or_exit(require_scalar(scalars, "tuning.alpha_pano_max_init"), "tuning.alpha_pano_max_init");
    config.tuning.alpha_pipe_init = parse_double_or_exit(require_scalar(scalars, "tuning.alpha_pipe_init"), "tuning.alpha_pipe_init");
    config.tuning.beta_pipe_init = parse_double_or_exit(require_scalar(scalars, "tuning.beta_pipe_init"), "tuning.beta_pipe_init");
    config.tuning.zoom_pipe_init = parse_double_or_exit(require_scalar(scalars, "tuning.zoom_pipe_init"), "tuning.zoom_pipe_init");
    config.tuning.pitch_car_init = parse_double_or_exit(require_scalar(scalars, "tuning.pitch_car_init"), "tuning.pitch_car_init");
    config.tuning.yaw_car_init = parse_double_or_exit(require_scalar(scalars, "tuning.yaw_car_init"), "tuning.yaw_car_init");
    config.tuning.zoom_car_init = parse_double_or_exit(require_scalar(scalars, "tuning.zoom_car_init"), "tuning.zoom_car_init");

    if (pyz_rows.size() != RecipeConfig::Views::kItems) {
        cerr << "views.pyz must contain exactly " << RecipeConfig::Views::kItems
             << " rows, but found " << pyz_rows.size() << endl;
        exit(1);
    }
    for (size_t i = 0; i < pyz_rows.size(); ++i) {
        config.views.pyz[i] = pyz_rows[i];
    }

    if (config.display.cols <= 0 || config.display.rows <= 0) {
        cerr << "display.cols and display.rows must both be positive." << endl;
        exit(1);
    }
    if (config.projection.mode != "car_pano" && config.projection.mode != "tube_pano") {
        cerr << "projection.mode must be car_pano or tube_pano." << endl;
        exit(1);
    }
    if (config.media.type == RecipeMediaType::VIDEO_FILE && config.media.source.empty()) {
        cerr << "media.source is required for media.type=video_file." << endl;
        exit(1);
    }
    if (config.media.type == RecipeMediaType::STREAM_HTTP && config.media.source.empty()) {
        cerr << "media.source is required for media.type=stream_http." << endl;
        exit(1);
    }

    validate_yaml_paths_or_exit(config);
    return config;
}
