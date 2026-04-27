//also updated 17aug2023
//updated 14May2022

#include <memory>
#include <ostream>

#include "cast.h"
#include "recipe_legacy_loader.h"
#include "recipe_resolver.h"
#include "recipe_validator.h"
#include "recipe_yaml.h"
#include "tools.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>
#include <stdlib.h>

//#include <json-c/json.h>
//#include <boost/filesystem/path.hpp>
//using namespace boost::filesystem;

/*#if __cplusplus < 201703L // If the version of C++ is less than 17
#include <experimental/filesystem>
// It was still in the experimental:: namespace
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif*/

//using namespace std;
extern double my_global_alpha_range_vender;

extern struct running_mode el_state;
extern VideoCapture *pVC;
extern VideoCapture camera_capture;
extern VideoCapture narl_capture;   //constructor init cannot be shown here

extern cv::VideoCapture worked_cap;;
/**
 *
 */
//global 
Cast *ing_cast_ptr;

namespace {
const string kLegacyDropboxPrefix = "/home/cj/Dropbox/";
const string kUsbDropboxPrefix = "/home/cj/usb_1T_disk/Dropbox/";
const string kDropboxRootToken = "@DROPBOX_ROOT@";

enum class MediaInputKind {
    IMAGE,
    VIDEO_FILE,
    CAMERA_USB,
    STREAM_HTTP,
    UNSUPPORTED_LEGACY_RPI
};

string remapLegacyDropboxPath(const string &path, bool require_regular_file = false, bool require_directory = false) {
    namespace fs = std::filesystem;

    if (path.empty()) {
        return path;
    }

    const fs::path original(path);
    if (require_regular_file && fs::is_regular_file(original)) {
        return path;
    }
    if (require_directory && fs::is_directory(original)) {
        return path;
    }
    if (!require_regular_file && !require_directory && fs::exists(original)) {
        return path;
    }

    if (path.rfind(kLegacyDropboxPrefix, 0) != 0) {
        return path;
    }

    const string candidate = kUsbDropboxPrefix + path.substr(kLegacyDropboxPrefix.size());
    const fs::path remapped(candidate);
    if (require_regular_file && fs::is_regular_file(remapped)) {
        return candidate;
    }
    if (require_directory && fs::is_directory(remapped)) {
        return candidate;
    }
    if (!require_regular_file && !require_directory && fs::exists(remapped)) {
        return candidate;
    }

    return path;
}

string ensureTrailingSlash(string path) {
    if (!path.empty() && path.back() != '/') {
        path.push_back('/');
    }
    return path;
}

string detectDropboxRoot() {
    namespace fs = std::filesystem;

    const char *env_root = getenv("CJ_NPPM_DROPBOX_ROOT");
    if (env_root != nullptr && fs::exists(fs::path(env_root))) {
        return ensureTrailingSlash(env_root);
    }

    const fs::path usb_dropbox_root = "/home/cj/usb_1T_disk/Dropbox";
    if (fs::exists(usb_dropbox_root)) {
        return ensureTrailingSlash(usb_dropbox_root.string());
    }

    const fs::path legacy_dropbox_root = "/home/cj/Dropbox";
    if (fs::exists(legacy_dropbox_root)) {
        return ensureTrailingSlash(legacy_dropbox_root.string());
    }

    return "";
}

string expandDropboxRootToken(const string &path) {
    if (path.rfind(kDropboxRootToken, 0) != 0) {
        return path;
    }

    const string dropbox_root = detectDropboxRoot();
    if (dropbox_root.empty()) {
        return path;
    }

    string suffix = path.substr(kDropboxRootToken.size());
    if (!suffix.empty() && suffix.front() == '/') {
        suffix.erase(0, 1);
    }
    return dropbox_root + suffix;
}

string resolveConfiguredPath(const string &path,
                            bool require_regular_file = false,
                            bool require_directory = false) {
    return remapLegacyDropboxPath(
            expandDropboxRootToken(path),
            require_regular_file,
            require_directory);
}

bool looksLikeStreamUri(const string &value) {
    return value.find("://") != string::npos;
}

MediaInputKind classifyMediaInput(const Cast &cast_config) {
    if (cast_config.la_is_camera_ready == "image") {
        return MediaInputKind::IMAGE;
    }

    if (cast_config.la_is_camera_ready == "video") {
        if (looksLikeStreamUri(cast_config.la_video_stream_uri)) {
            return MediaInputKind::STREAM_HTTP;
        }
        return MediaInputKind::VIDEO_FILE;
    }

    if (cast_config.la_is_camera_ready == "camera") {
        if (cast_config.md->cfg->getCameraName() == "narl") {
            return MediaInputKind::UNSUPPORTED_LEGACY_RPI;
        }
        return cast_config.la_is_usb_camera ? MediaInputKind::CAMERA_USB
                                            : MediaInputKind::STREAM_HTTP;
    }

    cerr << "Invalid @la_is_camera_ready value: [" << cast_config.la_is_camera_ready << "]" << endl;
    cerr << "Expected one of: camera, video, image." << endl;
    exit(1);
}

void validateParsedCastConfigurationOrExit(Cast &cast_config) {
    namespace fs = std::filesystem;

    if (cast_config.get_la_display_cols() <= 0 || cast_config.get_la_display_rows() <= 0) {
        cerr << "Invalid display size in recipe. "
             << "@la_display_cols and @la_display_rows must both be positive." << endl;
        exit(1);
    }

    if (cast_config.la_car_pano_or_tube_tube_pano != "car_pano" &&
        cast_config.la_car_pano_or_tube_tube_pano != "tube_pano") {
        cerr << "Invalid @la_car_pano_or_tube_tube_pano value: ["
             << cast_config.la_car_pano_or_tube_tube_pano << "]" << endl;
        cerr << "Expected either car_pano or tube_pano." << endl;
        exit(1);
    }

    if (!cast_config.using_mappers_folder.empty() &&
        !fs::is_directory(fs::path(cast_config.using_mappers_folder))) {
        cerr << "Mapper folder is not available after setup: ["
             << cast_config.using_mappers_folder << "]" << endl;
        exit(1);
    }

    switch (classifyMediaInput(cast_config)) {
        case MediaInputKind::IMAGE: {
            const string image_path = resolveConfiguredPath(
                    cast_config.la_image_inst_folder + cast_config.la_image_inst_short_file,
                    true,
                    false);
            if (!utl::doesFileExist(image_path)) {
                cerr << "Recipe references an image that does not exist: [" << image_path << "]" << endl;
                cerr << "Check @la_using_image_short_folder and @la_image_inst_short_file." << endl;
                exit(1);
            }
            break;
        }
        case MediaInputKind::VIDEO_FILE: {
            if (cast_config.la_video_stream_uri.empty()) {
                cerr << "Recipe media type is video_file, but @la_video_stream_uri is empty." << endl;
                exit(1);
            }
            if (!utl::doesFileExist(cast_config.la_video_stream_uri)) {
                cerr << "Recipe video source does not exist: [" << cast_config.la_video_stream_uri << "]" << endl;
                cerr << "Check @la_video_stream_uri in the recipe." << endl;
                exit(1);
            }
            break;
        }
        case MediaInputKind::STREAM_HTTP: {
            if (cast_config.la_video_stream_uri.empty()) {
                cerr << "Recipe network stream URI is empty." << endl;
                cerr << "Check @la_video_stream_uri in the recipe." << endl;
                exit(1);
            }
            break;
        }
        case MediaInputKind::CAMERA_USB:
            break;
        case MediaInputKind::UNSUPPORTED_LEGACY_RPI:
            cerr << "Unsupported media input: legacy Raspberry Pi camera capture." << endl;
            cerr << "This build does not support shell-based curl capture from a fixed Pi address." << endl;
            cerr << "Do not use @la_is_camera_ready=camera with cameraName=narl." << endl;
            cerr << "Use one of these supported alternatives instead:" << endl;
            cerr << "  - @la_is_camera_ready=image with a local image file" << endl;
            cerr << "  - @la_is_camera_ready=video with a local video file" << endl;
            cerr << "  - @la_is_camera_ready=video with a direct HTTP stream URL in @la_video_stream_uri" << endl;
            exit(1);
    }
}
}

/**
 * 
 * @param script_lname
 * @param script_dir
 */
void handle_unknow(string script_lname, string script_dir) {

#include <fstream>

    using std::ofstream;

    std::size_t found = script_lname.find_last_of('/');
    string sname(script_lname.substr(found + 1));
    std::cout << " file: " << script_lname.substr(found + 1) << '\n' << sname << endl;

    string script_now(script_dir);
    script_now.append(utl::now_as_string());
    script_now.append("@");
    script_now.append(sname);

    ofstream outfile;
    outfile.open(script_now);
    outfile << "# ";
    outfile << script_lname << endl;
    outfile << "# " << script_now << endl;

    ifstream infile;
    infile.open(script_lname);
    outfile << infile.rdbuf();
    infile.close();
    outfile.close();
    //ln -s <script_now> <script_ln_s>
}


Cast::Cast(int argc, char *argv[]) {

    md = new Moildev();    //Not good, put this statement here
    string ing_recipe_name;
    //2b process in next if..else block
    //short file name stored with prefix _!! NAME
    string ing_recipe_s_name;
    recipe_name(argc, argv, ing_recipe_name, ing_recipe_s_name);
    String long_cast_fname(ing_recipe_name) ;
    tLog("Using local scripts!!", "ing_cast_fname", long_cast_fname);
    system("pwd");
    if (utl::exists_test3(long_cast_fname)) {
        validate_recipe_file_or_exit(long_cast_fname);
        const bool is_yaml = is_yaml_recipe_path(long_cast_fname);
        const RecipeConfig config = is_yaml
                                    ? load_yaml_recipe_or_exit(long_cast_fname)
                                    : load_legacy_recipe_or_exit(long_cast_fname);
        la_cast_version = is_yaml ? "YAML-V1" : "JUN-2022";
        apply_recipe_config(config);
        caster_dump();
        return;
    } else {
        tLog("Local Config Running File does not exist", "long_cast_fname", long_cast_fname);
        exit(0);
    }
}

void Cast::recipe_name(int argc,
                       char *const *argv,
                       string &ing_recipe_name,
                       string &ing_short_name) const {
    ResolvedRecipe resolved = resolve_recipe(argc, argv);
    ing_recipe_name = resolved.recipe_path;
    ing_short_name = resolved.recipe_short_name;
}


bool Cast::using_image_inst_check() {
    string a_using_imgage_inst = resolveConfiguredPath(la_image_inst_folder + la_image_inst_short_file, true, false);
    la_using_image = imread(a_using_imgage_inst);
    if (la_using_image.empty()) {
        cerr << "File of zero length: [" << a_using_imgage_inst << "] or not existed" << endl;
        cerr << "Put image file to the folder [" << a_using_imgage_inst << "]" << endl;
        exit(0);
    } else if ((la_using_image.cols != md->cfg->getImageWidth()) || (la_using_image.rows != md->cfg->getImageHeight())) {
        tLog ("Ing Image is NOT compliance with default camera", "la_using_image.cols", la_using_image.cols);;
        resize(la_using_image, la_using_image, Size(md->cfg->getImageWidth(), md->cfg->getImageHeight()));
    } else {
        tLog ("Ing Image is compliance with default camera", "la_using_image.cols", la_using_image.cols);
    }
    return (!la_using_image.empty());
}


string Cast::create_folder_if_not_exist(const string &the_folder) const {
    const string resolved_folder = resolveConfiguredPath(the_folder, false, true);
    if (!utl::exists_test3(resolved_folder.c_str())) {
        if (resolved_folder.back() != '/') {
            cerr << "NPPM::MSG(Foledr shoud end with /) " << endl;
            cerr << resolved_folder << endl;
            exit(0);
        }
        struct stat info{};
        if (stat(resolved_folder.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)) { // S_ISDIR() doesn't exist on my windows
            cout << "# [cout-log] [" << resolved_folder << "] is existed and a good la_results_folder." << endl;
        } else {
            cout << "# [cout-log] Cannot access [" << resolved_folder << "], and will create it." << endl;
            // ASSUMED INCLUDES
            // #include <string> // required for std::string
            // #include <sys/types.h> // required for stat.h
            // #include <sys/stat.h> // no clue why required -- man pages say so
            string sPath = resolved_folder;
            mode_t nMode = 0733; // UNIX style permissions
            int nError = 0;
#if defined(_WIN32)
            nError = _mkdir(sPath.c_str()); // can be used on Windows
#else
            nError = mkdir(sPath.c_str(), nMode); // can be used on non-Windows
#endif
            if (nError != 0) {
                cerr << "Cannot create folder: " << resolved_folder << endl;
                cerr << "Please create manually the folder: " << resolved_folder << endl;
                exit(0);
            }
        }
        cout << "# [cout-log] *** Folder Created: [" << resolved_folder << "]." << endl;
    }
    return resolved_folder;
}

void Cast::initialize_media_from_recipe() {
    if (la_is_camera_ready == "camera") {
        tLog(287, "@la_is_camera_ready is as camera", la_is_camera_ready);
        open_cv_camera();
        if (md->cfg->getCameraName() != "narl") {
            camera_capture >> la_using_image;
        }
        el_state.util_media_type = ING_STAT::MediaType::CAMERA;
    } else if (la_is_camera_ready == "video") {
        worked_cap.open(la_video_stream_uri);
        if (!worked_cap.isOpened()) {
            tLog("<NEW> Error opening video file", "la_video_stream_uri", la_video_stream_uri);
            exit(0);
        }
        worked_cap >> la_using_image;
        el_state.util_media_type = ING_STAT::MediaType::VIDEO;
    } else if (la_is_camera_ready == "image") {
        if (!using_image_inst_check()) {
            cerr << " #No image found, Prepare an an image manually." << endl;
            exit(0);
        }
        el_state.util_media_type = ING_STAT::MediaType::IMAGE_FILE;
    } else {
        cerr << "Error media type" << endl;
        exit(0);
    }
}

void Cast::apply_recipe_config(const RecipeConfig &config) {
    md->Config(config.camera.name,
               config.camera.sensor_width,
               config.camera.sensor_height,
               config.camera.icx,
               config.camera.icy,
               config.camera.ratio,
               config.camera.image_width,
               config.camera.image_height,
               config.camera.calibration_ratio,
               config.camera.para[0],
               config.camera.para[1],
               config.camera.para[2],
               config.camera.para[3],
               config.camera.para[4],
               config.camera.para[5]);

    alpha_range_vender = config.tuning.alpha_range_vendor;
    my_global_alpha_range_vender = alpha_range_vender;
    alpha_pano_min = config.tuning.alpha_pano_min;
    alpha_pano_max = config.tuning.alpha_pano_max;
    beta_pano_begin = config.tuning.beta_pano_begin;
    beta_pano_range = config.tuning.beta_pano_range;
    alpha_pano_min_init = config.tuning.alpha_pano_min_init;
    alpha_pano_max_init = config.tuning.alpha_pano_max_init;
    alpha_pipe_init = config.tuning.alpha_pipe_init;
    beta_pipe_init = config.tuning.beta_pipe_init;
    zoom_pipe_init = config.tuning.zoom_pipe_init;
    pitch_car_init = config.tuning.pitch_car_init;
    yaw_car_init = config.tuning.yaw_car_init;
    zoom_car_init = config.tuning.zoom_car_init;

    VIRTUAL_ORGIN_LEFT = config.display.virtual_origin_left;
    VIRTUAL_ORGIN_TOP = config.display.virtual_origin_top;

    la_nppm_ing_abs_folder = resolveConfiguredPath(config.paths.working_root, false, true);
    create_folder_if_not_exist(la_nppm_ing_abs_folder);

    la_camera_name_as_inst_short_folder = config.paths.camera_folder;
    const string a_using_case = la_nppm_ing_abs_folder + md->cfg->getCameraName() + "/";
    create_folder_if_not_exist(a_using_case);

    la_using_image_short_folder = config.paths.image_folder;
    la_image_inst_folder = create_folder_if_not_exist(a_using_case + la_using_image_short_folder);
    la_image_inst_short_file = config.paths.image_file;

    la_car_pano_or_tube_tube_pano = config.projection.mode;
    la_mappers_folder_short = config.paths.mapper_folder;
    la_screen_title = config.display.title;
    la_display_cols = config.display.cols;
    la_display_rows = config.display.rows;
    la_vir_frame_asp = config.display.frame_aspect;
    using_mappers_folder = create_folder_if_not_exist(a_using_case + la_mappers_folder_short);

    switch (config.media.type) {
        case RecipeMediaType::IMAGE:
            la_is_camera_ready = "image";
            break;
        case RecipeMediaType::VIDEO_FILE:
        case RecipeMediaType::STREAM_HTTP:
            la_is_camera_ready = "video";
            break;
        case RecipeMediaType::CAMERA_USB:
        case RecipeMediaType::UNSUPPORTED_LEGACY_RPI:
            la_is_camera_ready = "camera";
            break;
    }
    la_is_usb_camera = config.media.usb_camera;
    la_video_stream_uri = resolveConfiguredPath(config.media.source, true, false);

    for (int row = 0; row < la_pyz_items; ++row) {
        pyz[row][0] = config.views.pyz[row][0];
        pyz[row][1] = config.views.pyz[row][1];
        pyz[row][2] = config.views.pyz[row][2];
    }

    la_iC_alpha_degree = config.projection.ic_alpha_degree;
    la_iC_beta_degree = config.projection.ic_beta_degree;
    la_roi_car_pano_left = config.roi.car_pano_left;
    la_roi_car_pano_top = config.roi.car_pano_top;
    la_roi_car_pano_cols = config.roi.car_pano_cols;
    la_roi_car_pano_rows = config.roi.car_pano_rows;
    la_display_asp = config.roi.display_aspect;
    la_rt_alpha_from = config.roi.rt_alpha_from;
    la_rt_alpha_to = config.roi.rt_alpha_to;

    validateParsedCastConfigurationOrExit(*this);
    initialize_media_from_recipe();
}

/// This camera will be access by different places
/// \param pVC
/// \return


bool Cast::open_cv_camera() {
    tLog(447, "ing_cast->md->cfg->getCameraName()", md->cfg->getCameraName());
    if (md->cfg->getCameraName() == "narl") {
        if (!using_image_inst_check()) {
            cerr << " #No AUX media, Prepare an an image manually." << endl;
            exit(0);
        }
        return true;
    }

    //if(el_state.util_media_type == ING_STAT::MediaType::VIDEO)

    //camera_capture.release();
    if (la_is_usb_camera) { //open usb
        for (auto port = 0; port < 16; port++) {
            cout << "*** Check connected USB Port: " << port << endl;
            camera_capture.open(port);
            waitKey(1000);
            if (camera_capture.isOpened())
                break;
            if (port == 15) {
                cerr << "Can not Open USB Camera" << endl
                     << endl;
                exit(0);
            }
        }
        tLog(458, "cast:: ", "USB Camera is Opened!!!");
    } else { //open Gige

        //camera_capture.open(la_video_stream_uri);
        //assert(camera_capture.isOpened());
    }
    //set camera  resolution of the openCV
    //camera_capture.set(CAP_PROP_FRAME_WIDTH, md->cfg->getImageWidth());
    //camera_capture.set(CAP_PROP_FRAME_HEIGHT, md->cfg->imageHeight);
    //el_state.util_media_type = ING_STAT::MediaType::CAMERA;
    return true;
}

void Cast::setVirtualOrginLeft(int virtualOrginLeft) {
    VIRTUAL_ORGIN_LEFT = virtualOrginLeft;
}

void Cast::setVirtualOrginTop(int virtualOrginTop) {
    VIRTUAL_ORGIN_TOP = virtualOrginTop;
}
