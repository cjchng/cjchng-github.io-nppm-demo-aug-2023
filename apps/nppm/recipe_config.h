
#pragma once

#include <array>
#include <string>

enum class RecipeMediaType {
    IMAGE,
    VIDEO_FILE,
    CAMERA_USB,
    STREAM_HTTP,
    UNSUPPORTED_LEGACY_RPI
};

struct RecipeConfig {
    int version = 1;

    struct Camera {
        std::string name;
        double sensor_width = 0.0;
        double sensor_height = 0.0;
        double icx = 0.0;
        double icy = 0.0;
        double ratio = 0.0;
        double image_width = 0.0;
        double image_height = 0.0;
        double calibration_ratio = 0.0;
        std::array<double, 6> para{};
    } camera;

    struct Paths {
        std::string working_root;
        std::string camera_folder;
        std::string image_folder;
        std::string image_file;
        std::string mapper_folder;
    } paths;

    struct Media {
        RecipeMediaType type = RecipeMediaType::IMAGE;
        bool usb_camera = false;
        std::string source;
    } media;

    struct Display {
        std::string title;
        int cols = 0;
        int rows = 0;
        double frame_aspect = 0.0;
        int virtual_origin_left = 0;
        int virtual_origin_top = 0;
    } display;

    struct Projection {
        std::string mode;
        double ic_alpha_degree = 0.0;
        double ic_beta_degree = 0.0;
    } projection;

    struct Roi {
        double car_pano_left = 0.0;
        double car_pano_top = 0.0;
        double car_pano_cols = 0.0;
        double car_pano_rows = 0.0;
        double display_aspect = 0.0;
        double rt_alpha_from = 0.0;
        double rt_alpha_to = 0.0;
    } roi;

    struct Tuning {
        double alpha_range_vendor = 0.0;
        double alpha_pano_min = 0.0;
        double alpha_pano_max = 0.0;
        double beta_pano_begin = 0.0;
        double beta_pano_range = 0.0;
        double alpha_pano_min_init = 0.0;
        double alpha_pano_max_init = 0.0;
        double alpha_pipe_init = 0.0;
        double beta_pipe_init = 0.0;
        double zoom_pipe_init = 0.0;
        double pitch_car_init = 0.0;
        double yaw_car_init = 0.0;
        double zoom_car_init = 0.0;
    } tuning;

    struct Views {
        static constexpr std::size_t kItems = 6;
        std::array<std::array<double, 3>, kItems> pyz{};
    } views;
};
