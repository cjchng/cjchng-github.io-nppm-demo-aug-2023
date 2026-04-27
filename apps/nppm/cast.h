#include <memory>
#include "../../dev/x_moildev.h"
//#include "../util/tools.h"
#include <opencv2/opencv.hpp>
#include "recipe_config.h"

//using namespace cv;
using namespace std;

//namespace ct {

    class Cast {
    private:
        int la_display_cols;
        int la_display_rows;
        double la_vir_frame_asp;
        int VIRTUAL_ORGIN_LEFT;
    public:
        void setVirtualOrginLeft(int virtualOrginLeft);
        void setVirtualOrginTop(int virtualOrginTop);

    private:
        int VIRTUAL_ORGIN_TOP;

    public:
        int get_la_display_cols() {return la_display_cols ;}
        int get_la_display_rows() {return la_display_rows ;}
        //int get_la_display_asp() {return la_display_asp;}
        int get_VIRTUAL_ORGIN_LEFT () {return VIRTUAL_ORGIN_LEFT;}
        void set_VIRTUAL_ORGIN_LEFT (int left){ VIRTUAL_ORGIN_LEFT = left; }
        int get_VIRTUAL_ORGIN_TOP () {return VIRTUAL_ORGIN_TOP;}
        void set_VIRTUAL_ORGIN_TOP (int top){ VIRTUAL_ORGIN_TOP = top; }

    public:
        double la_display_asp;
        //char img_path_be_processed[512];
        //const int la_respberry_width = 2592;
        //const int la_respberry_height = 1944;

        cv::Mat la_using_image;
        string la_nppm_ing_abs_folder;
        string la_camera_name_as_inst_short_folder;  //using resultd for each camera
        string la_using_image_short_folder;
        string la_image_inst_folder;
        string la_image_inst_short_file;

        Moildev *md ;
        //std:auto_ptr<Moildev> md(new Moildev() );

        bool open_cv_camera();
        string la_screen_title{"Title is not yet defined"};

        string la_cast_version ;
        double alpha_range_vender;
        double alpha_pano_min, alpha_pano_max, beta_pano_begin, beta_pano_range;
        double alpha_pano_min_init, alpha_pano_max_init;
        double alpha_pipe_init, beta_pipe_init, zoom_pipe_init;
        double pitch_car_init, yaw_car_init, zoom_car_init;

        string la_car_pano_or_tube_tube_pano;
        string la_mappers_folder_short;
        string using_mappers_folder;
        string la_is_camera_ready;
        bool la_is_usb_camera;
        string la_video_stream_uri;
        //////////////////////////////////////
        //double pitch[6];
        //double yaw[6];
        //double zoom[6] ;
        static const int la_pyz_items = 6;
        double pyz[la_pyz_items][4];    //for roll extension of <CarAnyPoint>
        double la_iC_alpha_degree;
        double la_iC_beta_degree;
        double la_roi_car_pano_left;
        double la_roi_car_pano_top;
        double la_roi_car_pano_cols;
        double la_roi_car_pano_rows;

        double la_rt_alpha_from;
        double la_rt_alpha_to;

    private:
        string create_folder_if_not_exist(const string &the_folder) const;
        bool using_image_inst_check();
        void initialize_media_from_recipe();
        void apply_recipe_config(const RecipeConfig &config);

    public:
//void read_in_script();
        Cast(int argc, char *argv[]) ;
        void recipe_name(int argc, char *const *argv, string &ing_recipe_name, string &ing_short_name) const;
    };
//}
