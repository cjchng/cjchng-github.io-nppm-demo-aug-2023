#include "recipe_legacy_loader.h"

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace {

constexpr const char *kDropboxToken = "@DROPBOX_ROOT@";
constexpr const char *kLegacyDropboxToken = "@DROPBOX@";
constexpr const char *kSkipPrefixes = "#*{}$";

string trim_copy(const string &value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == string::npos) {
        return "";
    }
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

string normalize_path_token(string value) {
    if (value.rfind(kLegacyDropboxToken, 0) == 0) {
        value.replace(0, string(kLegacyDropboxToken).size(), kDropboxToken);
    }
    return value;
}

vector<string> read_relevant_lines(const string &recipe_path) {
    ifstream input(recipe_path);
    if (!input.is_open()) {
        cerr << "Cannot open legacy recipe file: [" << recipe_path << "]" << endl;
        exit(1);
    }

    vector<string> lines;
    string raw_line;
    while (getline(input, raw_line)) {
        const string line = trim_copy(raw_line);
        if (line.empty()) {
            continue;
        }
        if (string(kSkipPrefixes).find(line.front()) != string::npos) {
            continue;
        }
        lines.push_back(line);
    }
    return lines;
}

class LegacyParser {
public:
    explicit LegacyParser(vector<string> lines)
        : lines_(std::move(lines)) {}

    string next_line() {
        if (index_ >= lines_.size()) {
            cerr << "Unexpected end of legacy recipe while parsing." << endl;
            exit(1);
        }
        return lines_[index_++];
    }

    string expect(const string &token) {
        const string actual = next_line();
        if (actual != token) {
            cerr << "Expected token [" << token << "], found [" << actual << "]" << endl;
            exit(1);
        }
        return next_line();
    }

private:
    vector<string> lines_;
    size_t index_ = 0;
};

RecipeMediaType classify_legacy_media(const string &mode, bool usb_camera, const string &source) {
    if (mode == "image") {
        return RecipeMediaType::IMAGE;
    }
    if (mode == "video") {
        return source.find("://") != string::npos || source.rfind("//", 0) == 0
               ? RecipeMediaType::STREAM_HTTP
               : RecipeMediaType::VIDEO_FILE;
    }
    if (mode == "camera" && usb_camera) {
        return RecipeMediaType::CAMERA_USB;
    }
    return RecipeMediaType::UNSUPPORTED_LEGACY_RPI;
}

} // namespace

bool is_legacy_recipe_path(const string &recipe_path) {
    const string extension = filesystem::path(recipe_path).extension().string();
    return extension == ".cast";
}

RecipeConfig load_legacy_recipe_or_exit(const string &recipe_path) {
    if (!is_legacy_recipe_path(recipe_path)) {
        cerr << "Unsupported legacy recipe extension: [" << recipe_path << "]" << endl;
        exit(1);
    }

    LegacyParser parser(read_relevant_lines(recipe_path));
    RecipeConfig config;

    const string cast_version = parser.expect("@cast_version");
    if (cast_version != "JUN-2022") {
        cerr << "Unsupported legacy recipe version: [" << cast_version << "]" << endl;
        exit(1);
    }

    config.camera.name = parser.expect("@md->cfg->cameraName");

    {
        const string camera_values_line = parser.next_line();
        vector<double> camera_values;
        size_t start = 0;
        while (start < camera_values_line.size()) {
            const size_t end = camera_values_line.find_first_of(" \t", start);
            const string token = camera_values_line.substr(start, end - start);
            if (!token.empty()) {
                camera_values.push_back(stod(token));
            }
            if (end == string::npos) {
                break;
            }
            start = camera_values_line.find_first_not_of(" \t", end);
            if (start == string::npos) {
                break;
            }
        }
        if (camera_values.size() != 14) {
            cerr << "Expected 14 camera values, found [" << camera_values.size() << "] in [" << recipe_path << "]" << endl;
            exit(1);
        }

        config.camera.sensor_width = camera_values[0];
        config.camera.sensor_height = camera_values[1];
        config.camera.icx = camera_values[2];
        config.camera.icy = camera_values[3];
        config.camera.ratio = camera_values[4];
        config.camera.image_width = camera_values[5];
        config.camera.image_height = camera_values[6];
        config.camera.calibration_ratio = camera_values[7];
        for (size_t i = 0; i < config.camera.para.size(); ++i) {
            config.camera.para[i] = camera_values[8 + i];
        }
    }

    {
        const string token = parser.next_line();
        if (token != "@alpha_range_vender" && token != "@alpha_range_vendor") {
            cerr << "Expected @alpha_range_vender or @alpha_range_vendor, found [" << token << "]" << endl;
            exit(1);
        }
        config.tuning.alpha_range_vendor = stod(parser.next_line());
    }

    config.tuning.alpha_pano_min = stod(parser.expect("@alpha_pano_min"));
    config.tuning.alpha_pano_max = stod(parser.expect("@alpha_pano_max"));
    config.tuning.beta_pano_begin = stod(parser.expect("@beta_pano_begin"));
    config.tuning.beta_pano_range = stod(parser.expect("@beta_pano_range"));
    config.tuning.alpha_pano_min_init = stod(parser.expect("@alpha_pano_min_init"));
    config.tuning.alpha_pano_max_init = stod(parser.expect("@alpha_pano_max_init"));
    config.tuning.alpha_pipe_init = stod(parser.expect("@alpha_pipe_init"));
    config.tuning.beta_pipe_init = stod(parser.expect("@beta_pipe_init"));
    config.tuning.zoom_pipe_init = stod(parser.expect("@zoom_pipe_init"));
    config.tuning.pitch_car_init = stod(parser.expect("@pitch_car_init"));
    config.tuning.yaw_car_init = stod(parser.expect("@yaw_car_init"));
    config.tuning.zoom_car_init = stod(parser.expect("@zoom_car_init"));
    config.display.virtual_origin_left = stoi(parser.expect("@VIRTUAL_ORGIN_LEFT"));
    config.display.virtual_origin_top = stoi(parser.expect("@VIRTUAL_ORGIN_TOP"));
    config.paths.working_root = normalize_path_token(parser.expect("@la_nppm_ing_abs_folder"));
    config.paths.camera_folder = parser.expect("@la_camera_name_as_inst_short_folder");
    config.paths.image_folder = parser.expect("@la_using_image_short_folder");
    config.paths.image_file = parser.expect("@la_image_inst_short_file");
    config.projection.mode = parser.expect("@la_car_pano_or_tube_tube_pano");
    config.paths.mapper_folder = parser.expect("@la_mappers_folder_short");
    config.display.title = parser.expect("@la_screen_title");
    config.display.cols = stoi(parser.expect("@la_display_cols"));
    config.display.rows = stoi(parser.expect("@la_display_rows"));
    config.display.frame_aspect = stod(parser.expect("@la_vir_frame_asp"));

    const string media_mode = parser.expect("@la_is_camera_ready");
    config.media.usb_camera = parser.expect("@la_is_usb_camera") == "1";
    config.media.source = normalize_path_token(parser.expect("@la_video_stream_uri"));
    config.media.type = classify_legacy_media(media_mode, config.media.usb_camera, config.media.source);

    for (size_t row = 0; row < RecipeConfig::Views::kItems; ++row) {
        for (size_t col = 0; col < 3; ++col) {
            config.views.pyz[row][col] = stod(parser.expect("@pyz[a_row][a_col]"));
        }
    }

    config.projection.ic_alpha_degree = stod(parser.expect("@la_iC_alpha_degree"));
    config.projection.ic_beta_degree = stod(parser.expect("@la_iC_beta_degree"));
    config.roi.car_pano_left = stod(parser.expect("@la_roi_car_pano_left"));
    config.roi.car_pano_top = stod(parser.expect("@la_roi_car_pano_top"));
    config.roi.car_pano_cols = stod(parser.expect("@la_roi_car_pano_cols"));
    config.roi.car_pano_rows = stod(parser.expect("@la_roi_car_pano_rows"));
    config.roi.display_aspect = stod(parser.expect("@la_display_asp"));
    config.roi.rt_alpha_from = stod(parser.expect("@la_rt_alpha_from"));
    config.roi.rt_alpha_to = stod(parser.expect("@la_rt_alpha_to"));

    return config;
}
