//
// Created by cj on 2021/11/16.
//

#ifndef NPPM_DEMO_GEO_H
#define NPPM_DEMO_GEO_H
#include <iostream>
#include <cmath>
struct Space_OK {
    bool viewports_redrawn;
    bool using_image;
    bool car_views[6];
    bool mode_1;
    bool mode_2;
    bool tube_pano;
    bool car_pano;
};//(ing_geo->get_A_CAR_TOP() + ing_geo->get_A_CAR_ROWS() < ing_geo->get_dsply_rows())

//namespace  g {
    class Geo {
    public:
        Space_OK space_Ok;
    private:
        static const int myFOUR = 4;
        static const int mySIX = 6;
        const double a_CARDINAL_EAST = 0;   // Cardinal zero degree as the zero angular scalar
        const double a_ANTI_PI_BY_SIX = 12; // Angular 2PI as 12 units ;
        const int TWENTY_FOUR = 24;

    private:   //pure private
        int la_pyz_left;
        int la_pyz_top;
        int la_pyz_front_nr;
        int la_pyz_front_left;
        int la_pyz_front_top;
        int la_pyz_front_cols;
        int la_pyz_front_rows;
        int la_middle_dash;     //= 96; //this is a must
        double vir_fram_asp; //=0.75 ;

    private: //semi-private
        int dsply_cols ;      //
        int dsply_rows ;      //
        int A_PANO_LEFT ; // = dsply_cols / 60; //32;
        int A_PANO_TOP ;  // = dsply_rows /TWENTY_FOUR 24;  //48;
        int A_PANO_COLS ; // = dsply_cols - 2 * wall_cmn;
        int A_PANO_ROWS ;

        int a_PANO_WG_TOP ;

        int A_SRC_LEFT ; //  A_PANO_LEFT + (A_PIPE_COLS + wall_cmn) * 1;
        int A_SRC_TOP ;  //  A_PIPE_TOP;
        int A_SRC_COLS ; //  A_PIPE_COLS;
        int A_SRC_ROWS ; //  A_PIPE_ROWS;

        int A_CAR_LEFT ;  //  A_PANO_LEFT + (A_PIPE_COLS + wall_cmn) * 2;
        int A_CAR_TOP ;   //  A_PIPE_TOP;
        int la_car_cols ; //  A_PIPE_COLS;
        int la_car_row ;  //  A_PIPE_ROWS;

        //range of pano-wg: 640
        int A_PIPE_LEFT; // = A_PANO_LEFT;
        int A_PIPE_TOP;  // = A_PANO_ROWS + A_PANO_TOP + ceiline; //60 is the ceiline of src
        int A_PIPE_COLS; // = (A_PANO_COLS - 2 * wall_cmn) / 3;
        int A_PIPE_ROWS; //  A_PIPE_COLS * vir_fram_asp ; // ; //  A_PIPE_COLS * Cast::la_vir_frame_asp;

        int BTN_COLS ; 
        int BTN_ROWS ;
        //const int a_BTN_COLS = A_SRC_COLS() / 6;
        //const int a_BTN_ROWS = 40;
        //const int bottom_nppm_wd_left = ing_geo->get_A_PIPE_LEFT();
        //const int bottom_nppm_wg_top = ing_geo->get_A_SRC_TOP() + ing_geo->get_A_SRC_ROWS() + 16;


        int geo_car_port[mySIX][myFOUR];

    public:    //int get_  () {return ;}
        int get_dsply_cols  () {return dsply_cols;}
        int get_dsply_rows  () {return dsply_rows;}

        int get_A_PANO_LEFT  () {return A_PANO_LEFT ;}
        int get_A_PANO_TOP  () {return A_PANO_TOP ;}
        int get_A_PANO_COLS  () {return A_PANO_COLS ;}
        int get_A_PANO_ROWS  () {return A_PANO_ROWS ;}

        int get_a_PANO_WG_TOP() {return a_PANO_WG_TOP ;}

        int get_A_SRC_LEFT() {return A_SRC_LEFT ;}
        int get_A_SRC_TOP() {return A_SRC_TOP ;}
        int get_A_SRC_COLS() {return A_SRC_COLS ;}; //  A_PIPE_COLS;
        int get_A_SRC_ROWS() {return A_SRC_ROWS ;}

        int get_A_CAR_LEFT() {return A_CAR_LEFT;}
        int get_A_CAR_TOP() {return A_CAR_TOP ; }
        int get_A_CAR_COLS() {return la_car_cols;}
        int get_A_CAR_ROWS() {return la_car_row;}

        int get_A_PIPE_LEFT() {return A_PIPE_LEFT; }
        int get_A_PIPE_TOP() {return A_PIPE_TOP;}
        int get_A_PIPE_COLS() {return A_PIPE_COLS;}
        int get_A_PIPE_ROWS  () {return A_PIPE_ROWS; }
        int get_BTN_COLS() {return BTN_COLS; }
        int get_BTN_ROWS() {return BTN_ROWS; }
        int get_geo_car_port(int seq, int ltrb) {return geo_car_port[seq] [ltrb]; }

    public:
        Geo(int cols, int rows, double vir_fram_asp) : dsply_cols(cols),
                                                       dsply_rows(rows),
                                                       vir_fram_asp(vir_fram_asp) {

            std::cout << "kkk  dsply_cols: " << dsply_cols << std::endl;
            std::cout << "kkk  dsply_rows: " << dsply_rows << std::endl;
            std::cout << "kkkvir_fram_asp: " << vir_fram_asp << std::endl;

            //for 16:9 4:3 1:1 image modification
            int vir_frame_fitting = .03 * dsply_cols * vir_fram_asp;    ////////.02->.03

            A_PANO_LEFT = dsply_cols / 60;
            A_PANO_TOP = 48; //dsply_rows / TWENTY_FOUR;  //48;
            A_PANO_COLS = dsply_cols - 2 * vir_frame_fitting;
            double screen_asp = static_cast<double>(dsply_rows)/static_cast<double>(dsply_cols) ; // <800/1920 > is 0.417
            if(screen_asp <0.45 ) {
                //A_PANO_ROWS = static_cast<int> (dsply_cols / M_PI / 2 * 1.5);  //1.2 old value
                A_PANO_ROWS = dsply_rows - 140 ;  //1.2 old value
                //A_PANO_COLS -= A_PANO_ROWS ;
            }
            else
            {
                A_PANO_ROWS = (int) (dsply_cols / M_PI / 2 * 1.2);
            }

            la_middle_dash = 100; //80 / vir_fram_asp;
            A_PIPE_LEFT = 2 * vir_frame_fitting + 20 ;
            A_PIPE_TOP = A_PANO_ROWS + A_PANO_TOP + la_middle_dash; //60 is the ceiline of src
            A_PIPE_COLS = (dsply_cols - 4 * A_PIPE_LEFT) / 3;
            A_PIPE_ROWS = A_PIPE_COLS * vir_fram_asp; // = A_PIPE_COLS * Cast::la_vir_frame_asp;

            a_PANO_WG_TOP = A_PANO_TOP + A_PANO_ROWS + 0.65 * vir_frame_fitting;

            A_SRC_LEFT = A_PIPE_LEFT + (A_PIPE_COLS + 2 * vir_frame_fitting) * 1;
            A_SRC_TOP = A_PIPE_TOP;
            A_SRC_COLS = A_PIPE_COLS;
            A_SRC_ROWS = A_PIPE_ROWS;

            A_CAR_LEFT = A_PIPE_LEFT + (A_PIPE_COLS + 2 * vir_frame_fitting) * 2;
            A_CAR_TOP = A_PIPE_TOP;
            la_car_cols = A_PIPE_COLS;
            la_car_row = A_PIPE_ROWS;
            //range of pano-wg: 640
            ////////////////////////////////////////////////////////////////////////
            BTN_COLS = A_SRC_COLS / 8 ;
            BTN_ROWS = 32 ;
            ////////////////////////////////////////////////////////////////////////
            //Here
            la_pyz_left = A_PANO_LEFT;
            la_pyz_top = A_PANO_TOP;
            const int FOUR = 4;
            const int SIX = 6;
            la_pyz_front_nr = FOUR;
            la_pyz_front_left = la_pyz_left;
            la_pyz_front_top = la_pyz_top;
            //int a_pyz_front_effect = (dsply_cols - 1 * vir_frame_fitting) / FOUR;

            const int idx_LEFT = 0;
            const int idx_TOP = 1;
            const int idx_COLS = 2;
            const int idx_ROWS = 3;

            int a_port_rows = A_PANO_ROWS;
            int a_port_column = a_port_rows * 1.2;
            la_pyz_left = A_PANO_LEFT;
            la_pyz_top = A_PANO_TOP;
            int a_pyz_cols_no_border = dsply_cols - 2 * la_pyz_left;
            int wall = (a_pyz_cols_no_border - 4 * a_port_column) / 3;
            int colums_wall = a_port_column + wall;

            for (int aI = 0; aI < FOUR; aI++) //left, top, cols, rows
            {
                geo_car_port[aI][idx_ROWS] = A_PANO_ROWS;
                geo_car_port[aI][idx_COLS] = (int) (A_PANO_ROWS * 1.2);
                geo_car_port[aI][idx_LEFT] = la_pyz_left + aI * colums_wall;
                geo_car_port[aI][idx_TOP] = la_pyz_front_top;
            }

            geo_car_port[4][idx_LEFT] = A_PIPE_LEFT;
            geo_car_port[4][idx_TOP] = A_PIPE_TOP;
            geo_car_port[4][idx_COLS] = A_PIPE_COLS;
            geo_car_port[4][idx_ROWS] = A_PIPE_COLS * vir_fram_asp;

            geo_car_port[5][idx_LEFT] = A_CAR_LEFT;
            geo_car_port[5][idx_TOP] = A_CAR_TOP;
            geo_car_port[5][idx_COLS] = la_car_cols;
            geo_car_port[5][idx_ROWS] = la_car_row;

            space_Ok.using_image = get_A_SRC_TOP() + get_A_SRC_ROWS() < get_dsply_rows() ;
            space_Ok.mode_1 = get_A_PIPE_TOP() + get_A_PIPE_ROWS() < get_dsply_rows() ;
            space_Ok.mode_2 = get_A_CAR_TOP() + get_A_CAR_ROWS() < get_dsply_rows() ;
        }
    };
//}

#endif //NPPM_DEMO_GEO_H
