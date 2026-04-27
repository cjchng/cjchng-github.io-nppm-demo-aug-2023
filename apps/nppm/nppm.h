#ifndef XANYPOINT_H
#define XANYPOINT_H

#include "../../dev/x_moildev.h"
#include "tools.h"
//#include "Geo.h"
//#include "cast.h"

#include <stdio.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>
#include <time.h>
#include <iostream>
#include <fstream>
#include <termios.h>

#include <array>
#include <unistd.h>
#include <sys/stat.h>

//#include <unistd.h>
//template<typename T>
//stringstream tLog(int idx, const string &var_token, T var) {
//    string part_1;
//    part_1.append(to_string(idx)).append(" < ").append(var_token).append(" > is : [");
//    stringstream buffer;
//    buffer << part_1 << var << "]" << endl;
//    LOG(buffer.str());
//    buffer.str("");
//    buffer.clear();
//    return buffer;
//}

//namespace demo {
class Nppm {

    public:
        Nppm(int argc, char *argv[]);
        ~Nppm();
        void run();


    private:
        //vp::Change modified;

        //Mat la_xpn_image_input_alt;
        Mat la_xpn_image_rt;
        Mat la_xpn_image_pano;
        Mat la_xpn_image_pnhl;
        Mat la_xpn_image_car;

        Mat la_2bd_stch_image_result;
        Mat la_xpn_image_result_resized;

        Mat la_xpn_pano_map_x;
        Mat la_xpn_pano_map_y;
        Mat la_xpn_rt_map_x;
        Mat la_xpn_rt_map_y;

        Mat la_xpn_pnhl_map_x;
        Mat la_xpn_pnhl_map_y;
        Mat la_xpn_car_map_x;
        Mat la_xpn_car_map_y;

#define CAST_SIX 6
        Mat la_car_map[CAST_SIX][2];

        Mat la_xpn_image0;
        Mat la_xpn_frame_cvui;

        double la_m_ratio;

        int la_scrn_virtual_top = 100;
        int la_scrn_virtual_left = 100;

        //clock_t la_start;
        clock_t la_start_overall;
        double la_time_clock;
        bool CaptureState = false;

        void la_xpn_MatWrite(const string &filename, const Mat &mat);

        Mat la_xpn_MatRead(const string &filename);

        void la_xpn_DisplayCh();

        string saveImage(Mat img);

        void freeMemory();

        void btn_camera();

        string get_mapping_id(const string &pitch, const string &yaw, const string &zoom, const string &cameraID,
                              const string &imageID) const;
        //bool ui_tube_pano_handler(int &g_1_upper_counter, double &g_1_left_track, double &g_1_right_track, int &g_1_lower_counter,
        //                     int &g_2_upper_counter, double &g_2_left_track, double &g_2_right_track, int &g_2_lower_counter
        //) ;


        bool
        multi_tracks(int multi, string name[], int left, int top, int unit_cols, double min[], double max[], double v[],
                     double size, int color);

        void tube_pano_or_car_pano(
                string the_mode,
                double a_ui_pitch_cur_bottom,
                double a_ui_pitch_cur_top,
                double a_ui_beta_from,
                double a_ui_beta_span
        );

        void pano_car_mode(double ic_alpha_degree, //a_80,
                           double ic_beta_degree,  //a_minus_5,
                           double aroi_car_pano_left,
                           double roi_car_pano_top,
                           double roi_car_pano_cols,
                           double roi_car_pano_rows) {};


private:
    struct cj_wd_2{
            string the_var_as_name;
            double *the_var;
            double the_max;
            double the_min ;
            double delta_u;
            double delta_l;
            int precision ;
            cv::Mat frame_cvui;
            int left;
            int top;
            int top2top;
            int l, t, c, r, b;
        };

    private:
        void roi_car_pano_auto_resize(double left_head, double top_head, double cols, double rows);
        void refresh_nppm_viewports();
        void refresh_car_6views();
        void ui_tube_pano();
        void help_dump_Rt_() const;
        bool counter_in(double &the_var, float the_delta, int fmt_precision, cv::Mat &frame_cvui, const int pv_left,
                        const int pv_top, int &left, int &top, int &cols, int &rows) const;

        VideoWriter videoWriter;
private:
    cv::Mat la_xpn_frame_scaled;
public:
    const Mat &getLaXpnFrameScaled() const;

    void setLaXpnFrameScaled(const Mat &laXpnFrameScaled);

private:
    cv::Mat la_xpn_image_input_alt;

        bool
        counters_paired(string the_var_str, double *the_updated, double the_max, double the_min,
                        float the_delta_upper, float the_delta_lower, int the_precision, double cur_on_counter,
                        Mat &frame_cvui, const int pv_left, const int pv_top, int top2top, int &left, int &top,
                        int &cols, int &rows, int &between) const;
        bool
        counters_paired_2(cj_wd_2 &) ;

        stringstream iLog(const string &var_token, int var) const;
        //template<typename T>
        //stringstream tLog(const string &var_token, T var) const;
    static void resize2copy(int left, int top, int cols, int rows, int wdg) ;

    void ok_resize(int left, int top, int cols, int rows, const Mat &img, const Mat &alt, const Mat &scaled) const;

    void pre_run_assert() const;
};
//}
#endif
