
#define CVUI_IMPLEMENTATION


#include <csignal>
#include <filesystem>
#include "cvui.h"
#include "nppm.h"
#include "geo.h"
#include "cast.h"
//#include "fish.h"

#define EASTR(a) "a" << a
const double ROLL_TEST = 0.0 ;

extern Cast *ing_cast_ptr;

Geo *UI_geo;
VideoCapture *pVC ;
VideoCapture camera_capture ;
VideoCapture narl_capture(CAP_V4L2) ;
cv::VideoCapture worked_cap;
//#include "../util/tools.h"

#define MAP_CACHE_ENABLED true

extern struct running_mode el_state;

stringstream Nppm::iLog(
    const string &var_token, int var
) const
{
    string part_1;
    part_1.append(" < ").append(var_token).append(" > is : [");

    stringstream buffer;
    buffer << part_1 << var << "]" << endl;
    LOG(buffer.str());
    buffer.str("");
    buffer.clear();
    return buffer;
}

Nppm::Nppm(
    int argc, char *argv[]
)
{
    
    ing_cast_ptr = new Cast(argc, argv);
    cout << "display_cols:" << ing_cast_ptr->get_la_display_cols() << endl;
    cout << "display_rows:" << ing_cast_ptr->get_la_display_rows() << endl;
    cout << " display_asp:" << ing_cast_ptr->la_display_asp << endl;
//    ing_cast ->md = new Moildev();
//    ing_cast ->get_running_cast("") ;
}

namespace vp {
    struct UI_activated {
        bool viewports_redrawn;
        bool using_image;
        bool car_views[6];
        bool mode_1;
        bool mode_2;
        bool tube_pano;
        bool car_pano;
    };
}


struct vp_ext {
    bool clicked;
};

namespace vp {
    vp::UI_activated modified;
}

void Nppm::la_xpn_DisplayCh() {
    if (vp::modified.viewports_redrawn) {
        cout << "viewports_redrawn is true" << endl;
        for (int ai = 0; ai < 6; ai++) {
            if (vp::modified.car_views[ai])
                cout << "..car_views[" << ai << "]:" << vp::modified.car_views[ai] << endl;
        }
        if (vp::modified.mode_1)
            cout << " ..vp::modified.mode_1:" << vp::modified.mode_1 << endl;
        if (vp::modified.mode_2)
            cout << " ..vp::modified.mode_2:" << vp::modified.mode_2 << endl;
        if (vp::modified.tube_pano)
            cout << " ..tube_pano:" << vp::modified.tube_pano << endl;
        if (vp::modified.car_pano)
            cout << " ..vp::modified.car_pano:" << vp::modified.car_pano << endl;
    }
    
    if (ing_cast_ptr->la_using_image.empty()) {
        tLog("EXIT!!", "ing_cast->la_using_image.empty()", ing_cast_ptr->la_using_image.empty());
        exit(0);
    }
    if (el_state.util_media_type == ING_STAT::MediaType::CAMERA) {
        //cvtColor(la_xpn_image_input_alt, la_xpn_image_input_alt, COLOR_BGR2RGB);
        //cvtColor(la_xpn_image_pano, la_xpn_image_pano, COLOR_BGR2RGB);
    }
    
//Always display
//    int var=ing_geo->get_A_SRC_COLS() ;
//    string var_token("ing_geo->get_A_SRC_COLS()") ;
//    stringstream buffer = iLog(var_token, var);
//    iLog("ing_cast->la_using_image.cols", ing_cast->la_using_image.cols) ;
//    tLog("ing_cast->la_using_image.cols", ing_cast->la_using_image.cols) ;
//    tLog("BORDER_CONSTANT",BORDER_CONSTANT) ;
//    //ing_cast->la_using_image;
//    tLog("ing_cast->la_nppm_ing_abs_folder",ing_cast->la_nppm_ing_abs_folder) ;
//    buffer <<ing_geo->get_A_SRC_ROWS() <<endl ;LOG(buffer.str()) ;buffer.str(""); buffer.clear();

    if(UI_geo->space_Ok.using_image) {
        cv::resize(ing_cast_ptr->la_using_image, la_xpn_image_input_alt,
                   Size(UI_geo->get_A_SRC_COLS(), UI_geo->get_A_SRC_ROWS()));
/***
 * cv::Scalar(0, 0xff, 0): green color
 */
        la_xpn_frame_scaled(cv::Rect(UI_geo->get_A_SRC_LEFT() - 5,
                                     UI_geo->get_A_SRC_TOP() - 5,
                                     UI_geo->get_A_SRC_COLS() + 10,
                                     UI_geo->get_A_SRC_ROWS() + 10)) = cv::Scalar(0, 0xff, 0);

        la_xpn_image_input_alt.copyTo(
                la_xpn_frame_scaled(Rect(UI_geo->get_A_SRC_LEFT(), UI_geo->get_A_SRC_TOP(),
                                         UI_geo->get_A_SRC_COLS(), UI_geo->get_A_SRC_ROWS())));


    }
/*

//    int left =ing_geo->get_A_SRC_LEFT();
//    int top = ing_geo->get_A_SRC_TOP() ;
//    int cols= ing_geo->get_A_SRC_COLS() ;
//    int rows = ing_geo->get_A_SRC_ROWS() ;
//    int wdg = 140 ;
//    Mat img=ing_cast->la_using_image ;
//    Mat alt = la_xpn_image_input_alt ;
//    Mat scaled=la_xpn_frame_scaled ;
//
//    //ok_resize(left, top, cols, rows, img, alt, scaled);
//
//    if ((top+rows) < ing_geo->get_dsply_rows() && (left+cols) <ing_geo->get_dsply_cols()) {
//        tLog(121, "cols", cols);
//        tLog(122, "rows", rows);
//        cv::resize(img, alt, Size(cols, rows));
//        alt.copyTo(scaled(Rect(left, top, cols, rows)));
//    }

 */


/*

    resize2copy(left, top, cols, rows, wdg);

//     double asp= static_cast<double>(rows)/static_cast<double>(cols) ;
//    if ((top+rows) > ing_geo->get_dsply_rows() ) {
//       rows = ing_geo->get_dsply_rows() - top -wdg ;
//       cols = static_cast<int>(((static_cast<double>(rows))/asp)) ;
//    }
//    else if( (left+cols) > ing_geo->get_dsply_cols()){
//        cols = ing_geo->get_dsply_cols() - left -wdg ;
//        rows= static_cast<int> (static_cast<double>(cols) *asp) ;
//    }
//    tLog(127, "cols", cols) ;
//    tLog(128, "rows", rows) ;
//    cv::resize(ing_cast->la_using_image, la_xpn_image_input_alt,
//               Size(cols, rows));
//    la_xpn_image_input_alt.copyTo(
//            la_xpn_frame_scaled(Rect(left, top, cols, rows)));

*/
    if(el_state.util_media_type == ING_STAT::MediaType::VIDEO){
        vp::modified.mode_1 = true;
        vp::modified.mode_2 = true;
        vp::modified.tube_pano = true;
        vp::modified.car_pano = true;
        for (int ai = 0; ai < 6; ai++)
            vp::modified.car_views[ai] = true;
        vp::modified.viewports_redrawn = true;
    }

    if (el_state.util_system_state == ING_STAT::SystemState::NPPM) {
        //if (vp::modified.mode_1 && (ing_geo->get_A_PIPE_TOP() + ing_geo->get_A_PIPE_ROWS() < ing_geo->get_dsply_rows())) {
        if (vp::modified.mode_1 && UI_geo->space_Ok.mode_1) {

            int a_pnhl_left = UI_geo->get_A_PIPE_LEFT(); //small
            int a_pnhl_top = UI_geo->get_A_PIPE_TOP();
            remap(ing_cast_ptr->la_using_image, la_xpn_image_pnhl, la_xpn_pnhl_map_x, la_xpn_pnhl_map_y, INTER_CUBIC,
                  BORDER_CONSTANT, Scalar(0, 0, 0));
            {
                cv::resize(la_xpn_image_pnhl, la_xpn_image_pnhl,
                           Size(UI_geo->get_A_PIPE_COLS(), UI_geo->get_A_PIPE_ROWS()));
                la_xpn_image_pnhl.copyTo(
                        la_xpn_frame_scaled(
                                Rect(a_pnhl_left, a_pnhl_top, la_xpn_image_pnhl.cols, la_xpn_image_pnhl.rows)));
                //tLog(128, "ing_cast->alpha_pipe_init", ing_cast->alpha_pipe_init);
            }
        }
        if (vp::modified.mode_2 && UI_geo->space_Ok.mode_2) {
            //cout <<"vp::modified.mode_2 : " <<vp::modified.mode_2 << endl ;
            int a_car_left = UI_geo->get_A_CAR_LEFT(); //small
            int a_car_top = UI_geo->get_A_CAR_TOP();
            remap(ing_cast_ptr->la_using_image, la_xpn_image_car, la_xpn_car_map_x, la_xpn_car_map_y, INTER_CUBIC,
                  BORDER_CONSTANT, Scalar(0, 0, 0));
            cv::resize(la_xpn_image_car, la_xpn_image_car, Size(UI_geo->get_A_CAR_COLS(), UI_geo->get_A_CAR_ROWS()));
            la_xpn_image_car.copyTo(
                    la_xpn_frame_scaled(Rect(a_car_left, a_car_top, la_xpn_image_car.cols, la_xpn_image_car.rows)));
            //vp::modified.mode_2 =false ;
        }
//tLog("Here exit smoothly", "", "") ;
//exit(0) ;
        int a_pano_left = UI_geo->get_A_PANO_LEFT();
        int a_pano_top = UI_geo->get_A_PANO_TOP();
        int a_pano_cols = UI_geo->get_A_PANO_COLS();
        int a_pano_rows = UI_geo->get_A_PANO_ROWS();

        if (ing_cast_ptr->la_car_pano_or_tube_tube_pano == "car_pano" && vp::modified.car_pano) {
            //Mat aT ;
            la_xpn_frame_scaled(Rect(UI_geo->get_A_PANO_LEFT(), a_pano_top, a_pano_cols, a_pano_rows)) = 0;
            remap(ing_cast_ptr->la_using_image, la_xpn_image_rt, la_xpn_rt_map_x, la_xpn_rt_map_y,
                  INTER_CUBIC, BORDER_CONSTANT, Scalar(0, 0, 0));

            roi_car_pano_auto_resize(ing_cast_ptr->la_roi_car_pano_left, ing_cast_ptr->la_roi_car_pano_top,
                                     ing_cast_ptr->la_roi_car_pano_cols, ing_cast_ptr->la_roi_car_pano_rows);

            //@cj 08042022 side effect to <la_xpn_image_rt>
            //cj::resize before copy
            cv::resize(la_xpn_image_rt, la_xpn_image_rt, Size(a_pano_cols, a_pano_rows)); //cj,/2));
            la_xpn_image_rt.copyTo(
                    la_xpn_frame_scaled(Rect(UI_geo->get_A_PANO_LEFT(), a_pano_top, a_pano_cols, a_pano_rows)));
            //vp::modified.car_pano= false ;
        } else if (ing_cast_ptr->la_car_pano_or_tube_tube_pano == "tube_pano" && vp::modified.tube_pano) {
            //la_xpn_frame_scaled(Rect(a_pano_left, a_pano_top, a_pano_cols, a_pano_rows)) = 0;
            remap(ing_cast_ptr->la_using_image, la_xpn_image_pano, la_xpn_pano_map_x, la_xpn_pano_map_y,
                  INTER_CUBIC, BORDER_CONSTANT, Scalar(0, 0, 0));
            cv::resize(la_xpn_image_pano, la_xpn_image_pano,
                       Size(UI_geo->get_A_PANO_COLS(),
                            UI_geo->get_A_PANO_ROWS())); //cj,/2));             //            }
            la_xpn_image_pano.copyTo(la_xpn_frame_scaled(Rect(a_pano_left, a_pano_top, a_pano_cols, a_pano_rows)));
            //vp::modified.tube_pano = false ;
        }

    } else if (el_state.util_system_state == ING_STAT::SystemState::CAR_VIEW) {
        for (int aI = 0; aI < 6; aI++) {
            if (vp::modified.car_views[aI]) {
                remap(ing_cast_ptr->la_using_image, la_xpn_image_car, la_car_map[aI][0], la_car_map[aI][1], INTER_CUBIC,
                      BORDER_CONSTANT,
                      Scalar(0, 0, 0));
                cv::resize(la_xpn_image_car, la_xpn_image_car,
                           Size(UI_geo->get_geo_car_port(aI, 2), UI_geo->get_geo_car_port(aI, 3)));

                la_xpn_image_car.copyTo(la_xpn_frame_scaled(Rect(
                        UI_geo->get_geo_car_port(aI, 0),
                        UI_geo->get_geo_car_port(aI, 1),
                        la_xpn_image_car.cols,
                        la_xpn_image_car.rows)));
                //cout << "??..vp::modified.car_views[aI]: " <<  aI << endl ;
            }
            //vp::modified.car_views[aI] = false;
        }
    }

    //reset_all_ui_flags
    vp::modified.mode_1 = false;
    vp::modified.mode_2 = false;
    vp::modified.tube_pano = false;
    vp::modified.car_pano = false;
    for (int ai = 0; ai < 6; ai++)
        vp::modified.car_views[ai] = false;
    vp::modified.viewports_redrawn = false;
    //tLog("Here exit smoothly", "", "") ;
    //exit(0) ;
}



//2bd
void Nppm::resize2copy(int left, int top, int cols, int rows, int wdg) {
    double asp= static_cast<double>(rows) / static_cast<double>(cols) ;
    if ((top+rows) > UI_geo->get_dsply_rows() ) {
       rows = UI_geo->get_dsply_rows() - top - wdg ;
       cols = static_cast<int>(((static_cast<double>(rows))/asp)) ;
    }
    else if((left+cols) > UI_geo->get_dsply_cols()){
        cols = UI_geo->get_dsply_cols() - left - wdg ;
        rows= static_cast<int> (static_cast<double>(cols) *asp) ;
    }
    tLog(127, "cols", cols) ;
    tLog(128, "rows", rows) ;
//    cv::resize(ing_cast->la_using_image, la_xpn_image_input_alt,
//           Size(cols, rows), INTER_AREA);
//    la_xpn_image_input_alt.copyTo(
//            la_xpn_frame_scaled(Rect(left, top, cols, rows)));
}

void Nppm::ok_resize(int left, int top, int cols, int rows, const Mat &img, const Mat &alt, const Mat &scaled) const {
    if ((top + rows) < UI_geo->get_dsply_rows() && (left + cols) < UI_geo->get_dsply_cols()) {
        tLog(121, "cols", cols);
        tLog(122, "rows", rows);
        resize(img, alt, Size(cols, rows));
        alt.copyTo(scaled(Rect(left, top, cols, rows)));
    }
}

////END OF [void Nppm::la_xpn_DisplayCh()]

//void Nppm::roi_car_pano_auto_resize(double left_head, double top_head, double cols_gain, double rows_gain) {
//
//    double a_left = la_xpn_image_rt.cols * left_head;
//    double a_top = la_xpn_image_rt.rows * top_head;
//    double a_cols = la_xpn_image_rt.cols * cols_gain;
//    double a_rows = la_xpn_image_rt.rows * rows_gain;
//
//    Rect roi(a_left, a_top, a_cols, a_rows);
//    la_xpn_image_rt = la_xpn_image_rt(roi);
//    //resize(la_xpn_image_rt, la_xpn_image_rt, Size(), 2.0, 2.0, INTER_LINEAR);
//    double magnify = 1 / (cols_gain > rows_gain ? cols_gain : rows_gain);
//    tLog(311, "magnify@roi_car_pano_auto_resize()", magnify ) ;
//    double magnify_cols = 1 / cols_gain;
//    double magnify_rows = 1 / rows_gain;
//
//    resize(la_xpn_image_rt, la_xpn_image_rt, Size(), magnify_cols, magnify_rows, INTER_CUBIC);
//}

void Nppm::roi_car_pano_auto_resize(double left_head, double top_head, double cols_gain, double rows_gain) {
    double a_left = la_xpn_image_rt.cols * left_head;
    double a_top = la_xpn_image_rt.rows * top_head;
    double a_cols = la_xpn_image_rt.cols * cols_gain;
    double a_rows = la_xpn_image_rt.rows * rows_gain;

    Rect roi(a_left, a_top, a_cols, a_rows);
    la_xpn_image_rt = la_xpn_image_rt(roi);

    double magnify = 1 / (cols_gain > rows_gain ? cols_gain : rows_gain);
    double magnify_cols = 1 / cols_gain;
    double magnify_rows = 1 / rows_gain;

    resize(la_xpn_image_rt, la_xpn_image_rt, Size(), magnify_cols, magnify_rows, INTER_CUBIC);
}



void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_RBUTTONDOWN )
    {
        cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if  ( event == EVENT_MBUTTONDOWN )
    {
        cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
    }
    else if ( event == EVENT_MOUSEMOVE )
    {
        cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

    }
}

//declar a extern variable for Nppm class

#include <X11/Xlib.h>
extern Nppm *obj_npp ;
int cur_x=7200, cur_y=400;

void getCursorScreenPosition(int& x, int& y) {
    Display* display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);

    int rootX, rootY, winX, winY;
    unsigned int mask;
    XQueryPointer(display, root, &root, &root, &rootX, &rootY, &winX, &winY, &mask);

    x = rootX;
    y = rootY;

    XCloseDisplay(display);
}

// void onMouse(int event, int x, int y, int flags, void* userdata) {
//     static bool isDragging = false;
//     cv::Point dragStart;
//     if (event == cv::EVENT_RBUTTONDOWN) {
//         std::cout << "Right-click event detected at position (" << x << ", " << y << ") and exit(...)" << std::endl;
//         exit(0) ;
//     }
//     else if (event == cv::EVENT_LBUTTONDOWN) {
//         isDragging = true;
//     } else if (event == cv::EVENT_LBUTTONUP) {
//         isDragging = false;
//     } else if (event == cv::EVENT_MOUSEMOVE && isDragging) {

//         dragStart = cv::Point(x, y);
//         std::cout << "Left-click event detected at WINDOW (" << x << ", " << y << ")" << std::endl;

//         int cursorX, cursorY;
//         getCursorScreenPosition(cursorX, cursorY);
//         std::cout << "Cursor position on screen: (" << cursorX << ", " << cursorY << ")" << std::endl;

//         cur_x= cursorX - x ;
//         cur_y= cursorY - y ;
//     }
// }

bool isDragging = false;
cv::Point dragStart;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    static bool isDragging = false;
    cv::Point dragStart;
    
    if (event == cv::EVENT_RBUTTONDOWN) {
        std::cout << "Right-click event detected at position (" << x << ", " << y << ") and exit(...)" << std::endl;
        exit(0);
    } 
    if (event == cv::EVENT_LBUTTONDOWN) {
        isDragging = true;
        cout << "Left-click DOWN event detected at position (" << x << ", " << y << ")" << std::endl;
        dragStart = cv::Point(x, y);
    } else if (event == cv::EVENT_LBUTTONUP) {
        isDragging = false;
        cout << "Left-click UP event detected at position (" << x << ", " << y << ")" << std::endl;
    } else if (event == cv::EVENT_MOUSEMOVE && isDragging) {
        cv::Point currentPos(x, y);
        cv::Point delta = currentPos - dragStart;
        // Update position or perform other actions based on the delta
        std::cout << "Dragging Delta: (" << delta.x << ", " << delta.y << ")" << std::endl;
        cur_x = delta.x;
        cur_y = delta.y;
    }
}

void Nppm::run() {
    static const std::string kExitGuidanceSuffix = " | Note: Hold ESC to EXIT";
    if (ing_cast_ptr->la_screen_title.find(kExitGuidanceSuffix) == std::string::npos) {
        ing_cast_ptr->la_screen_title += kExitGuidanceSuffix;
    }

    ing_cast_ptr->setVirtualOrginLeft(cur_x);
    ing_cast_ptr->setVirtualOrginTop(cur_y);
    
    static char a_key_hit ;
    pre_run_assert();   //assertion before run

    //configure visual components
    UI_geo = new Geo(ing_cast_ptr->get_la_display_cols(),
                     ing_cast_ptr->get_la_display_rows(),
                     ing_cast_ptr->la_display_asp);

    cvui::init(ing_cast_ptr->la_screen_title, 1); //construct a named window
    //cvui capture mouse event, CallBackFunc can not set work well
    //setMouseCallback(ing_cast->la_screen_title, CallBackFunc, NULL);
    //setMouseCallback(WINDOW_NAME, locator, NULL);//Mouse callback function on define window//
    //double calibrationWidth = md->getImageWidth();

    //cv::setMouseCallback(ing_cast_ptr->la_screen_title, onMouse);

    ing_cast_ptr->md->cfg->getImageHeight() ;
    double a_cmos_rows = ing_cast_ptr->md->cfg->getImageHeight();
    const double a_cmos_cols = ing_cast_ptr->md->cfg->getImageWidth();

    la_xpn_image_pano = Mat(a_cmos_rows, ing_cast_ptr->md->cfg->getImageWidth(), CV_32F); //4
    la_xpn_image_rt = Mat(a_cmos_rows, ing_cast_ptr->md->cfg->getImageWidth(), CV_32F);   //4
    la_xpn_frame_scaled = Mat(cv::Size(ing_cast_ptr->get_la_display_cols(), ing_cast_ptr->get_la_display_rows()), CV_8UC3);
    la_xpn_image_input_alt = Mat(cv::Size(ing_cast_ptr->get_la_display_cols(), ing_cast_ptr->get_la_display_rows()),CV_8UC3);
    //la_2bd_stch_image_result = Mat(700, STITCH_WIDTH, CV_8UC3);
    //la_xpn_image_result_resized = Mat(700, STITCH_WIDTH, CV_8UC3);
    //la_2bd_stch_image_result = cv::Scalar(0, 0, 0);
    //la_xpn_image_result_resized = cv::Scalar(0, 0, 0);

    la_xpn_pano_map_x = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4
    la_xpn_pano_map_y = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4

    la_xpn_rt_map_x = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4
    la_xpn_rt_map_y = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4

    la_xpn_pnhl_map_x = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4
    la_xpn_pnhl_map_y = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4

    la_xpn_car_map_x = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4
    la_xpn_car_map_y = Mat(a_cmos_rows, a_cmos_cols, CV_32F); //4

    //la_m_ratio = ing_cast->md->get_asp_ratio(); //030721@cj   //la_respberry_width / calibrationWidth;

    //////////////////////////////////////////////////////////////////////////////
    string car_6_view_name[ing_cast_ptr->la_pyz_items][2];
    for (int aI = 0; aI < ing_cast_ptr->la_pyz_items; aI++) {
        for (int idx = 0; idx < 2; idx++) {
            la_car_map[aI][idx] = Mat(a_cmos_rows, a_cmos_cols, CV_32F);
        }
        string pitch = to_string(ing_cast_ptr->pyz[aI][0]); //pyz[] is external data
        string yaw = to_string(ing_cast_ptr->pyz[aI][1]);
        string zoom = to_string(ing_cast_ptr->pyz[aI][2]);
        string cameraID(ing_cast_ptr->md->cfg->getCameraName());
        string imageID(ing_cast_ptr->la_image_inst_short_file);
        string cmn = get_mapping_id(pitch, yaw, zoom, cameraID, imageID);
        car_6_view_name[aI][0] = cmn + "-h.car";
        car_6_view_name[aI][1] = cmn + "-v.car";
    }

    for (int aI = 0; aI < ing_cast_ptr->la_pyz_items; aI++) /////////////////////////////////////
    {
        for (int aJ = 0; aJ < 2; aJ++)
            cout << car_6_view_name[aI][aJ] << endl;
    }

    //if (el_state.util_system_state == ING_STAT::SystemState::CAR_VIEW)
    {   //MAP_CACHE is preferred since not suitable for embedded case
        // if center is modified we have to reset the using mapper
        for (int aI = 0; aI < ing_cast_ptr->la_pyz_items; aI++) { //string cmn("map-"+imageID+"-"+ cameraID + "-" + pitch + "_" + yaw + "_" + zoom) ;

            char *a_pt = &car_6_view_name[aI][0][0]; //.c_str()[0] ;
            if (!fopen(a_pt, "r")) { //No corresponding mapper for this parameters
                ing_cast_ptr->md->CarAnyPoint((float *) la_car_map[aI][0].data,
                                              (float *) la_car_map[aI][1].data,
                                              la_xpn_car_map_x.cols,
                                              la_xpn_car_map_x.rows,
                                              ing_cast_ptr->pyz[aI][0],
                                              ing_cast_ptr->pyz[aI][1],
                                              ing_cast_ptr->pyz[aI][2],

                                              ROLL_TEST,

                                              ing_cast_ptr->md->cfg->getAspRatio());
                for (int idx = 0; idx < 2; idx++) { //save the mapper
                    la_xpn_MatWrite(car_6_view_name[aI][idx], la_car_map[aI][idx]);
                }
            } else { //the mapper is ready
                for (int idx = 0; idx < 2; idx++) {
                    la_car_map[aI][idx] = la_xpn_MatRead(car_6_view_name[aI][idx]);
                    //cout << " la_car_map[aI][idx]: " << car_6_view_name[aI][idx]  << endl;
                    //waitKey() ;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    if (MAP_CACHE_ENABLED) {
        bool map_exist = false;
        string pano_file_name_part_1 = get_mapping_id(
                std::to_string(ing_cast_ptr->alpha_pano_max_init),
                std::to_string(ing_cast_ptr->alpha_pano_min_init),
                "rsv-cj",
                ing_cast_ptr->md->cfg->getCameraName(),
                ing_cast_ptr->la_image_inst_short_file);
        string pano_h(pano_file_name_part_1 + "-h.pano");
        string pano_v(pano_file_name_part_1 + "-v.pano");

        string rt_file_name_part_1 = get_mapping_id(
                std::to_string(ing_cast_ptr->la_iC_alpha_degree),
                std::to_string(ing_cast_ptr->la_iC_beta_degree),
                "rsv-cj",
                ing_cast_ptr->md->cfg->getCameraName(),
                ing_cast_ptr->la_image_inst_short_file);
        string rt_h(pano_file_name_part_1 + "-h.rt");
        string rt_v(pano_file_name_part_1 + "-v.rt");

        string tube_part_1 = get_mapping_id(
                std::to_string((int) ing_cast_ptr->alpha_pipe_init),
                std::to_string((int) ing_cast_ptr->beta_pipe_init),
                std::to_string((int) ing_cast_ptr->zoom_pipe_init),
                ing_cast_ptr->md->cfg->getCameraName(),
                ing_cast_ptr->la_image_inst_short_file);
        string pipe_h(tube_part_1 + "-h.pipe");
        string pipe_v(tube_part_1 + "-v.pipe");

        string car_part_1 = get_mapping_id(
                std::to_string((int) ing_cast_ptr->pitch_car_init),
                std::to_string((int) ing_cast_ptr->yaw_car_init),
                std::to_string((int) ing_cast_ptr->zoom_car_init),
                ing_cast_ptr->md->cfg->getCameraName(),
                ing_cast_ptr->la_image_inst_short_file);
        string car_h(car_part_1 + "-h.car");
        string car_v(car_part_1 + "-v.car");

        if (!fopen(pano_h.c_str(), "r") ||
            !fopen(pano_v.c_str(), "r") ||
            !fopen(pipe_h.c_str(), "r") ||
            !fopen(pipe_v.c_str(), "r"))
            map_exist = false;

        if (map_exist) {
            la_xpn_pano_map_x = la_xpn_MatRead(pano_h);
            la_xpn_pano_map_y = la_xpn_MatRead(pano_v);
            la_xpn_rt_map_x = la_xpn_MatRead(rt_h);
            la_xpn_rt_map_y = la_xpn_MatRead(rt_v);
            la_xpn_pnhl_map_x = la_xpn_MatRead(pipe_h);
            la_xpn_pnhl_map_y = la_xpn_MatRead(pipe_v);
            la_xpn_pnhl_map_x = la_xpn_MatRead(car_h);
            la_xpn_pnhl_map_y = la_xpn_MatRead(car_v);
        } else {
 //27Jun2022
 // try a deleting
            if (1) { // call pano_tube_or_car_mode
                double a_ui_pitch_cur_bottom = ing_cast_ptr->alpha_pano_max_init;
                double a_ui_pitch_cur_top = ing_cast_ptr->alpha_pano_min_init;
                double a_ui_beta_from = 0;  //ing_geo->a_CARDINAL_EAST;
                double a_ui_beta_span = 360; //ing_geo->a_ANTI_PI_BY_SIX;
                tube_pano_or_car_pano("tube_pano",
                                      a_ui_pitch_cur_bottom,
                                      a_ui_pitch_cur_top,
                                      a_ui_beta_from,
                                      a_ui_beta_span);
                la_xpn_MatWrite(pano_h, la_xpn_pano_map_x);
                la_xpn_MatWrite(pano_v, la_xpn_pano_map_y);
            }
            if (1) { //4 car_pano don't care parameter
                double a_ui_pitch_cur_top = ing_cast_ptr->alpha_pano_min_init;
                double a_ui_pitch_cur_bottom = ing_cast_ptr->alpha_pano_max_init;
                double a_ui_beta_from = 0; //ing_geo->a_CARDINAL_EAST;
                double a_ui_beta_span = 360; //ing_geo->a_ANTI_PI_BY_SIX;
                tube_pano_or_car_pano("car_pano",
                                      a_ui_pitch_cur_bottom,
                                      a_ui_pitch_cur_top,
                                      a_ui_beta_from,
                                      a_ui_beta_span);
                la_xpn_MatWrite(rt_h, la_xpn_rt_map_x);
                la_xpn_MatWrite(rt_v, la_xpn_rt_map_y);
            }
            ing_cast_ptr->md->AnyPointM((float *) la_xpn_pnhl_map_x.data,
                                        (float *) la_xpn_pnhl_map_y.data,
                                        la_xpn_pnhl_map_x.cols,
                                        la_xpn_pnhl_map_x.rows,
                                        ing_cast_ptr->alpha_pipe_init,
                                        ing_cast_ptr->beta_pipe_init,
                                        ing_cast_ptr->zoom_pipe_init,
                                        ing_cast_ptr->md->cfg->getAspRatio());
            la_xpn_MatWrite(pipe_h, la_xpn_pnhl_map_x);
            la_xpn_MatWrite(pipe_v, la_xpn_pnhl_map_y);

            ing_cast_ptr->md->CarAnyPoint((float *) la_xpn_car_map_x.data,
                                          (float *) la_xpn_car_map_y.data,
                                          la_xpn_car_map_x.cols,
                                          la_xpn_car_map_x.rows,
                                          ing_cast_ptr->pitch_car_init,
                                          ing_cast_ptr->yaw_car_init,
                                          ing_cast_ptr->zoom_car_init,

                                          ROLL_TEST,

                                          ing_cast_ptr->md->cfg->getAspRatio());
            la_xpn_MatWrite(car_h, la_xpn_pnhl_map_x);
            la_xpn_MatWrite(car_v, la_xpn_pnhl_map_y);
        }
    } else // MAP_CACHE_ENABLED is false
    {
        ing_cast_ptr->md->AnyPointM((float *) la_xpn_pnhl_map_x.data,
                                    (float *) la_xpn_pnhl_map_y.data,
                                    la_xpn_pnhl_map_x.cols,
                                    la_xpn_pnhl_map_x.rows,
                                    ing_cast_ptr->alpha_pipe_init,
                                    ing_cast_ptr->beta_pipe_init,
                                    ing_cast_ptr->zoom_pipe_init,
                                    ing_cast_ptr->md->cfg->getAspRatio());

        ing_cast_ptr->md->CarAnyPoint((float *) la_xpn_car_map_x.data,
                                      (float *) la_xpn_car_map_y.data,
                                      la_xpn_car_map_x.cols,
                                      la_xpn_car_map_x.rows,
                                      ing_cast_ptr->pitch_car_init,
                                      ing_cast_ptr->yaw_car_init,
                                      ing_cast_ptr->zoom_car_init,

                                      ROLL_TEST,

                                      ing_cast_ptr->md->cfg->getAspRatio());

        if (1) {
            double a_ui_pitch_cur_bottom = ing_cast_ptr->alpha_pano_max_init;
            double a_ui_pitch_cur_top = ing_cast_ptr->alpha_pano_min_init;
            double a_ui_beta_from = ing_cast_ptr->beta_pano_begin;
            double a_ui_beta_span = ing_cast_ptr->beta_pano_range;
            tube_pano_or_car_pano("tube_pano", a_ui_pitch_cur_bottom, a_ui_pitch_cur_top, a_ui_beta_from,
                                  a_ui_beta_span);
            tube_pano_or_car_pano("car_pano", a_ui_pitch_cur_bottom, a_ui_pitch_cur_top, a_ui_beta_from,
                                  a_ui_beta_span);
        }

        //alpha_m2, beta_m2, zoom_m2, asp_m);
    }

    cout << "md-> Mapping Init Ready!!" << endl;

    la_xpn_frame_scaled = cv::Scalar(90, 52, 49);
    //list all the windows ;
    vp::modified.using_image = true;

    if (el_state.util_system_state == ING_STAT::SystemState::CAR_VIEW)
        refresh_car_6views();
    else if (el_state.util_system_state == ING_STAT::SystemState::NPPM)
        refresh_nppm_viewports();

    tLog("First invoking to la_xpn_DisplayCh()", "el_state.util_system_state", "" ) ;

    la_xpn_DisplayCh();
    bool cj_pause = false;

    struct cj_wd {
        string the_var_as_name;
        double *the_var;
        double the_max;
        double the_min;
        double delta_u;
        double delta_l;
        int precision;
    };

    int e1, e2;
    clock_t tStart, tStop;

    cv::moveWindow(ing_cast_ptr->la_screen_title, 2000, 2000);
    while (true)   //cyclic running
    {
        cv::pollKey();
//        e1 = cv::getTickCount() ;
//        tStart = clock();

        static int s_captured_cnt = 0;
        vp::modified.viewports_redrawn = false;
        ///////////////////////////////
        {  //capturing image
            vp::modified.using_image = false;
            if (el_state.util_media_type == ING_STAT::MediaType::CAMERA) {
                vp::modified.viewports_redrawn = true;
                if (ing_cast_ptr->md->cfg->getCameraName() == "narl") {
                    cerr << "Unsupported legacy Raspberry Pi capture path reached unexpectedly." << endl;
                    cerr << "This recipe should have been rejected during validation." << endl;
                    exit(1);
                } else if (camera_capture.isOpened()) {
                    s_captured_cnt++;
                    camera_capture >> ing_cast_ptr->la_using_image; //captured from camera

                    cv::resize(ing_cast_ptr->la_using_image, ing_cast_ptr->la_using_image,
                               Size(ing_cast_ptr->md->cfg->getImageWidth(), ing_cast_ptr->md->cfg->getImageHeight()));
                    if (el_state.recordState == ING_STAT::RecordOnOff::ON)
                        videoWriter.write(ing_cast_ptr->la_using_image);
                    if (ing_cast_ptr->la_using_image.empty()) {
                        el_state.util_media_type = ING_STAT::MediaType::NONE;
                        el_state.util_system_state = ING_STAT::SystemState::ERROR;
                        cerr << "No Image been Captued, Hardware fault!";
                        exit(0);
                    }
                }
                vp::modified.using_image = true;
                if (el_state.util_system_state == ING_STAT::SystemState::CAR_VIEW)
                    refresh_car_6views();
                else if (el_state.util_system_state == ING_STAT::SystemState::NPPM)
                    refresh_nppm_viewports();
            } else if (el_state.util_media_type == ING_STAT::MediaType::IMAGE_FILE) {

//                fish::rotate_by_beta(ing_cast->la_using_image) ;
//                if (vp::modified.using_image) {
//                    vp::modified.using_image = false;
//                }

            } else if (el_state.util_media_type == ING_STAT::MediaType::VIDEO && !cj_pause) {
                vp::modified.viewports_redrawn = true;

                tLog(754,"... before.... ing_cast_ptr->la_using_image", &ing_cast_ptr->la_using_image ) ;
//                *pVC >> ing_cast_ptr->la_using_image;
                worked_cap >> ing_cast_ptr->la_using_image ;
                tLog(756,".....after ......ing_cast_ptr->la_using_image", &ing_cast_ptr->la_using_image ) ;
                cv::resize(ing_cast_ptr->la_using_image, ing_cast_ptr->la_using_image,
                           Size(ing_cast_ptr->md->cfg->getImageWidth(), ing_cast_ptr->md->cfg->getImageHeight()));

                vp::modified.using_image = true;
                if (el_state.util_system_state == ING_STAT::SystemState::CAR_VIEW)
                    refresh_car_6views();
                else if (el_state.util_system_state == ING_STAT::SystemState::NPPM)
                    refresh_nppm_viewports();
                vp::modified.mode_1 = true;
                vp::modified.mode_2 = true;
                //tLog("Here exit smoothly", "", "") ;
                //exit(0) ;            
            }
        } //update current image from media source, video, camera or file

        //Mat la_using_image_copy = ing_cast->la_using_image.clone();

        la_xpn_frame_scaled.copyTo(la_xpn_frame_cvui);
        ///Show Title
        { //show the title string to UI, not needed for
            string aTitle(ing_cast_ptr->la_screen_title + "{" + ing_cast_ptr->md->cfg->getCameraName() + ": ");
            string sCnt(to_string(s_captured_cnt));

            if (ing_cast_ptr->la_is_camera_ready == "camera") {
                string a_substring(" Image(s) Captured: ");
                aTitle += a_substring;
                aTitle += sCnt;
            } else if (ing_cast_ptr->la_is_camera_ready == "video") {
                aTitle += ing_cast_ptr->la_video_stream_uri;
            } else {
                aTitle += ing_cast_ptr->la_using_image_short_folder +
                          ing_cast_ptr->la_image_inst_short_file;
            }

            //void text(cv::Mat &theWhere, int theX, int theY, const cv::String &theText, double theFontScale = 0.4, unsigned int theColor = 0xCECECE);
            //cvui.h line 372, 2431, grepping "void text("
            cvui::text(la_xpn_frame_cvui, 64, 16, aTitle, 1, 0xff00);
        }

        //const int a_get_BTN_COLS() = ing_geo->get_A_SRC_COLS() / 6;
        //const int ing_geo->get_BTN_ROWS() = 40;


        cv::Mat frame = la_xpn_frame_cvui;

        if (1)   //To rotate the fisheye image
        {
/////////////////////////////////////////////////////////////////////////////////////////////
            int l, t, c, r, b;
            static double roll_fish = 0.0;
            static double last;
            cj_wd_2 roll_pair = {"FISH_ROLL", &roll_fish, 360, 0, 10.0, 1.0, 0, frame,
                                 UI_geo->get_A_CAR_LEFT() - 200,
                                 UI_geo->get_A_CAR_TOP() + UI_geo->get_A_CAR_ROWS() + 50,
                                 20, l, t, c, r, b};
            bool update_roll = counters_paired_2(roll_pair);

            if (update_roll || vp::modified.using_image) {

                static Mat src_clone = ing_cast_ptr->la_using_image.clone(); //la_using_image_copy.clone() ;
                if (vp::modified.using_image) {
                    src_clone = ing_cast_ptr->la_using_image.clone();
                }

                uchar *src;
                if (src_clone.isContinuous())
                    src = src_clone.data;
                //uint src_length = src_clone.total() * src_clone.channels();

                uchar *dst;
                //static is needed here, or delete the data attached
                static Mat dst_clone = src_clone.clone();
                dst_clone = 0;
                if (dst_clone.isContinuous())
                    dst = dst_clone.data;
                //uint dst_length = dst_clone.total() * dst_clone.channels();

                ing_cast_ptr->md->fish_rotate(dst, src,
                                              ing_cast_ptr->md->cfg->getImageWidth(),
                                              ing_cast_ptr->md->cfg->getImageHeight(),
                                              ing_cast_ptr->md->cfg->getICx(),
                                              ing_cast_ptr->md->cfg->getICy(),
                                              roll_fish);

                ing_cast_ptr->la_using_image = dst_clone;
                cout << "..roll_fish       :" << roll_fish << endl;
                cout << "..roll_init_offset:" << roll_fish - last << endl;
                cout << "cols:" << ing_cast_ptr->la_using_image.cols << endl;
                //cout << "src_length:" << src_length << endl ;
                cout << "dst_clone.cols:" << dst_clone.cols << endl;
                cout << "dst_clone.rows:" << dst_clone.rows << endl;
                vp::modified.using_image = true;
            }
            last = roll_fish;
        }

        const int bottom_nppm_wd_left = UI_geo->get_A_PIPE_LEFT();
        const int bottom_nppm_wg_top = UI_geo->get_A_SRC_TOP() + UI_geo->get_A_SRC_ROWS() + 16;

        if (el_state.util_system_state == ING_STAT::SystemState::CAR_VIEW) {
#define NUM_OF_CAR_VIEWS 6
            for (int aI = 0; aI < NUM_OF_CAR_VIEWS; aI++) { //6 view UI
                const int idx_LEFT = 0;
                const int idx_TOP = 1;
                //const int idx_COLS = 2;
                const int idx_ROWS = 3;

#define UI_COLS_ADJUST_UNIT 10
#define UI_ROWS_ADJUST_UNIT 30

                const int a_UI_group_left = UI_geo->get_geo_car_port(aI, idx_LEFT) + UI_COLS_ADJUST_UNIT;
                const int a_UI_group_top =
                        UI_geo->get_geo_car_port(aI, idx_TOP) + UI_geo->get_geo_car_port(aI, idx_ROWS) +
                        UI_ROWS_ADJUST_UNIT;

                const int a_UI_unit_cols = 1920 / 18;
                const double a_UI_text_size = 0.4;

                if (1) {
                    static double roll[NUM_OF_CAR_VIEWS];
                    for (int n = 0; n < NUM_OF_CAR_VIEWS; n++)
                        ing_cast_ptr->pyz[n][3] = roll[n];

                    string a_UI_name[4] = {"pitch", "yaw", "  zoom", "roll"};
                    int a_UI_color = 0x00ff00;
                    /*
                    double a_UI_min[4] = {-ing_cast->alpha_range_vender*.5, //pitch
                                          -ing_cast->alpha_range_vender*.5, //yaw
                                          1,                                //zoom
                                          -20                              //roll
                                            };
                    double a_UI_max[4] = {ing_cast->alpha_range_vender*.5,
                                          ing_cast->alpha_range_vender*.5,
                                          16,
                                          20};
                    */
                    double a_UI_min[4] = {-90, //pitch
                                          -90, //yaw
                                          0.5,                                //zoom
                                          -180                              //roll
                    };
                    double a_UI_max[4] = {90,
                                          90,
                                          10,
                                          180};

                    bool clicked = multi_tracks(4, //ui setting {"pitch", "yaw" , "  zoom", "roll"}
                                                a_UI_name,       //showing texts {"pitch", "yaw" , "  zoom", "roll"}
                                                a_UI_group_left, //left
                                                a_UI_group_top,  //top
                                                a_UI_unit_cols,  //columns
                                                &a_UI_min[0],    //casting
                                                &a_UI_max[0],    //
                                                &(ing_cast_ptr->pyz[aI][0]),
                                                a_UI_text_size,
                                                a_UI_color);
                    if (clicked) {
                        ing_cast_ptr->md->CarAnyPoint(
                                (float *) la_car_map[aI][0].data,
                                (float *) la_car_map[aI][1].data,
                                la_xpn_car_map_x.cols,
                                la_xpn_car_map_x.rows,
                                ing_cast_ptr->pyz[aI][0],
                                ing_cast_ptr->pyz[aI][1],
                                ing_cast_ptr->pyz[aI][2],
                                ing_cast_ptr->pyz[aI][3],
                                ing_cast_ptr->md->cfg->getAspRatio());

                        roll[aI] = ing_cast_ptr->pyz[aI][3];
                        if (clicked) vp::modified.car_views[aI] = true;
                        //cout << "aI: [" << aI << "]" << pyz[aI][0] << pyz[aI][1] << pyz[aI][2] << endl;
                    }
                }
                vp::modified.viewports_redrawn = vp::modified.viewports_redrawn || vp::modified.car_views[aI];
            }
        } else if (el_state.util_system_state == ING_STAT::SystemState::NPPM) {

            //NPPM can show 360 degree view of two variants
            //one is the usual panorama focusing y-fidelity on a uni-radian cylindrical surface,
            //next is linear alpha scale on the cylindrical  surface.
            //the second mode also can adjust a selected alpha-beta to as the virtual optical axis.
            //This 3D rotation provides a bias normalizing alignment between objects image and camera body
            /////////////////////////////////////



            const string tube_pano_btn("Car2Tube");
            const string car_pano_btn("Tube2Car");
            //const string beta_alpha_btn("BETA_ALPHA");
            static string name = tube_pano_btn;
            const int left = UI_geo->get_A_PANO_LEFT() + UI_geo->get_A_PANO_COLS() - UI_geo->get_BTN_COLS();
            int top = UI_geo->get_A_PANO_TOP() + UI_geo->get_A_PANO_ROWS() -
                      UI_geo->get_BTN_ROWS();//UI_geo->get_a_PANO_WG_TOP();
            string la_names[2] = {"car_pano", "tube_pano"};
            static int multi_switching = 0;
            bool clicked = cvui::button(la_xpn_frame_cvui,
                                        left, top,
                                        UI_geo->get_BTN_COLS(), UI_geo->get_BTN_ROWS(),
                                        name);
            if (clicked) {
                if (name == car_pano_btn) {
                    name = tube_pano_btn;
                    ing_cast_ptr->la_car_pano_or_tube_tube_pano = "car_pano";
                    multi_switching = 2;

                } else if (name == tube_pano_btn) {
                    name = car_pano_btn;
                    ing_cast_ptr->la_car_pano_or_tube_tube_pano = "tube_pano";
                    multi_switching = 1;
                }
            } else if (ing_cast_ptr->la_car_pano_or_tube_tube_pano == "tube_pano" || (multi_switching == 1)) {

                ui_tube_pano(); //side effect tube_pano

                if (vp::modified.tube_pano || (multi_switching == 1)) {
                    cout << "1: tube_pano is true? " << vp::modified.tube_pano << endl;
                    tube_pano_or_car_pano(
                            "tube_pano",
                            ing_cast_ptr->alpha_pano_max_init,
                            ing_cast_ptr->alpha_pano_min_init,
                            ing_cast_ptr->beta_pano_begin,
                            ing_cast_ptr->beta_pano_range);
                    //tube_pano = true;
                    vp::modified.tube_pano = true;
                    vp::modified.viewports_redrawn = true;
                    multi_switching = 0;
                }

            } else if (ing_cast_ptr->la_car_pano_or_tube_tube_pano == "car_pano" || (multi_switching == 2)) {
                {
                    auto cj_horizontal_span = UI_geo->get_dsply_cols() / 10;
                    auto my_btn_left_from = cj_horizontal_span / 2;
                    auto top2Top = 20;  //tops separated

                    cj_wd a[6];
                    a[0] = {"iC_alpha_degree", &ing_cast_ptr->la_iC_alpha_degree, 0, 180, 10, 1, 0};
                    a[1] = {"iC_beta_degree", &ing_cast_ptr->la_iC_beta_degree, 0, 360, 20, 1, 0};
                    a[2] = {"roi_car_pano_left", &ing_cast_ptr->la_roi_car_pano_left, 0, 1, 0.1, 0.01, 2};
                    a[3] = {"roi_car_pano_cols", &ing_cast_ptr->la_roi_car_pano_cols, 0, 1, 0.1, 0.01, 2};
                    a[4] = {"roi_car_pano_top", &ing_cast_ptr->la_roi_car_pano_top, 0, 1, 0.1, 0.01, 2};
                    a[5] = {"roi_car_pano_rows", &ing_cast_ptr->la_roi_car_pano_rows, 0, 1, 0.1, 0.01, 2};
                    {
                        vp::modified.car_pano = false;
                        bool u[6];
                        for (int i = 0; i < 6; i++) {
                            int l, t, c, r, b;
                            double cur_2 = *a[i].the_var;
                            u[i] = counters_paired(
                                    a[i].the_var_as_name,
                                    a[i].the_var,
                                    a[i].the_max,
                                    a[i].the_min,
                                    a[i].delta_u,
                                    a[i].delta_l,
                                    a[i].precision,
                                    cur_2,
                                    la_xpn_frame_cvui,
                                    my_btn_left_from + cj_horizontal_span * i,
                                    //
//pv_top+ing_geo->get_A_SRC_ROWS(),
//care about sensor asp & frame asp\
                                 //sensor asp is by camera
//frame asp is by monitor
//in casting will not decide which type of monitor will be used
//the sensor asp is come from h/w ratio of sensor if the sensor
//is of square.
//
//ing_cast->get_la_display_rows() - 100,
                                    UI_geo->get_A_PANO_TOP() + UI_geo->get_A_PANO_ROWS() + 40,
                                    top2Top,
                                    l, t, c, r, b);
                            if (u[i]) {
                                tLog(814, "car_pano Update Seq:", i);
                            }
                            vp::modified.car_pano = vp::modified.car_pano || u[i];
                        }
                        if (vp::modified.car_pano) {
                            if (ing_cast_ptr->la_roi_car_pano_left + ing_cast_ptr->la_roi_car_pano_cols > 1.0) {
                                if (u[2]) {
                                    ing_cast_ptr->la_roi_car_pano_cols = 1.0 - ing_cast_ptr->la_roi_car_pano_left;
                                } else {
                                    ing_cast_ptr->la_roi_car_pano_left = 1.0 - ing_cast_ptr->la_roi_car_pano_cols;
                                }
                            }

                            if (ing_cast_ptr->la_roi_car_pano_top + ing_cast_ptr->la_roi_car_pano_rows > 1.0) {
                                if (u[4]) {
                                    ing_cast_ptr->la_roi_car_pano_rows = 1.0 - ing_cast_ptr->la_roi_car_pano_top;
                                } else {
                                    ing_cast_ptr->la_roi_car_pano_top = 1.0 - ing_cast_ptr->la_roi_car_pano_rows;
                                }
                            }
                        }
                    }
                }


                //tLog(841,"ing_cast->get_la_display_rows()", ing_cast->get_la_display_rows() ) ;
                //if (digits_updated || tracks_updated[0] || tracks_updated[1] || multi_switching) {
                if (vp::modified.car_pano || multi_switching == 2) {
                    tLog(811, "vp::modified.car_pano", vp::modified.car_pano);
                    tLog(812, " multi_switching", multi_switching);
                    ing_cast_ptr->md->xPanoramaM_Rt((float *) la_xpn_rt_map_x.data, (float *) la_xpn_rt_map_y.data,
                                                    la_xpn_rt_map_x.cols,
                                                    la_xpn_rt_map_x.rows,
                                                    ing_cast_ptr->md->cfg->getAspRatio(),
                                                    (double) ing_cast_ptr->alpha_range_vender,
                                                    ing_cast_ptr->la_iC_alpha_degree,
                                                    ing_cast_ptr->la_iC_beta_degree,
                                                    ing_cast_ptr->la_rt_alpha_from, ing_cast_ptr->la_rt_alpha_to);

                    multi_switching = 0;
                    vp::modified.car_pano = true;
                }
            }
            ////////////////////////////////////////////////////////////////////





            ///////////////////////////////////////////////////////////////
            { // NPPM track UI
                if (1) { // drawing NPPM MODE-1 Anypoint()
                    int l, t, c, r, b;
                    int left = UI_geo->get_A_PIPE_LEFT();
                    int unit = UI_geo->get_A_PIPE_COLS() / 12;
                    int top = UI_geo->get_A_PIPE_TOP() + UI_geo->get_A_PIPE_ROWS() + 40;

                    cj_wd_2 aa[3];
                    aa[0] = {"zenith", &ing_cast_ptr->alpha_pipe_init, 90, 0, 10.0, 1.0, 0, frame, left + 0 * unit, top,
                             20,
                             l, t, c, r, b};
                    aa[1] = {"azimuth", &ing_cast_ptr->beta_pipe_init, 360, 0, 10.0, 1.0, 0, frame, left + 4 * unit,
                             top, 20, l, t, c, r, b};
                    aa[2] = {"zoom", &ing_cast_ptr->zoom_pipe_init, 15, 0, 1.0, 0.2, 0, frame, left + 8 * unit, top,
                             20, l, t, c, r, b};


                    vp::modified.mode_1 =
                            counters_paired_2(aa[0]) || counters_paired_2(aa[1]) || counters_paired_2(aa[2]);

                    if (vp::modified.mode_1) {
                        ing_cast_ptr->md->AnyPointM((float *) la_xpn_pnhl_map_x.data,
                                                    (float *) la_xpn_pnhl_map_y.data,
                                                    la_xpn_pnhl_map_x.cols,
                                                    la_xpn_pnhl_map_x.rows,
                                                    ing_cast_ptr->alpha_pipe_init,
                                                    ing_cast_ptr->beta_pipe_init,
                                                    ing_cast_ptr->zoom_pipe_init,
                                                    ing_cast_ptr->md->cfg->getAspRatio());
                        tLog(961, "vp::modified.mode_1", vp::modified.mode_1);
                    }
                }

                if (1) { ////drawing NPPM MODE-2 Anypoint()
                    //Rename to <CarAnyPoint> to extend th roll operation
                    //cv::Mat frame = la_xpn_frame_cvui;
                    int left = UI_geo->get_A_CAR_LEFT();
                    int unit = UI_geo->get_A_CAR_COLS() / 12;
                    int top = UI_geo->get_A_CAR_TOP() + UI_geo->get_A_CAR_ROWS() + 40;
                    int l, t, c, r, b;
                    cj_wd_2 aa[3];
                    aa[0] = {"pitch", &ing_cast_ptr->pitch_car_init, 90, -90, 10.0, 1.0, 0, frame, left + 0 * unit, top,
                             20,
                             l, t, c, r, b};
                    aa[1] = {"yaw", &ing_cast_ptr->yaw_car_init, 360, 0, 10.0, 1.0, 0, frame, left + 4 * unit,
                             top, 20, l, t, c, r, b};
                    aa[2] = {"zoom", &ing_cast_ptr->zoom_car_init, 15, 0, 1.0, 0.2, 0, frame, left + 8 * unit, top,
                             20, l, t, c, r, b};

                    vp::modified.mode_2 =
                            counters_paired_2(aa[0]) || counters_paired_2(aa[1]) || counters_paired_2(aa[2]);

                    if (vp::modified.mode_2) {
                        ing_cast_ptr->md->CarAnyPoint(
                                (float *) la_xpn_car_map_x.data,
                                (float *) la_xpn_car_map_y.data,
                                la_xpn_car_map_x.cols,
                                la_xpn_car_map_x.rows,
                                ing_cast_ptr->pitch_car_init,
                                ing_cast_ptr->yaw_car_init,
                                ing_cast_ptr->zoom_car_init,

                                ROLL_TEST,

                                ing_cast_ptr->md->cfg->getAspRatio());
                        tLog(990, "vp::modified.mode_2", vp::modified.mode_2);
                    }


                }
            }
            vp::modified.viewports_redrawn =
                    vp::modified.tube_pano || vp::modified.car_pano || vp::modified.mode_1 || vp::modified.mode_2;
        }

        //begin control buttons

        // const int a_BTN_TOP = ing_geo->get_A_SRC_TOP() ;
        const int a_BTN_TOP = UI_geo->get_A_PANO_TOP() + UI_geo->get_A_PANO_ROWS();
        //const int ing_geo->get_BTN_COLS() = ing_geo->A_SRC_COLS / 6;
        int a_UI_wall = UI_geo->get_BTN_COLS() / 16;
        const int a_BTN_LEFT = UI_geo->get_A_PANO_LEFT() + UI_geo->get_A_PANO_COLS()
                               - 5 * UI_geo->get_BTN_COLS() - 4 * a_UI_wall;
        auto btn_unit = UI_geo->get_BTN_COLS() + a_UI_wall;

        const basic_string<char> to_nppm_label("NPPM");
        if (cvui::button(la_xpn_frame_cvui, a_BTN_LEFT + btn_unit * 0, a_BTN_TOP, UI_geo->get_BTN_COLS(),
                         UI_geo->get_BTN_ROWS(),
                         to_nppm_label)) {
            el_state.util_system_state = ING_STAT::SystemState::NPPM;
            refresh_nppm_viewports();
//            cout << "NPPM button" << endl;
            la_xpn_frame_scaled = cv::Scalar(49, 52, 49);
        }

        const basic_string<char> to_car_label("CAR VIEW");
        if (cvui::button(la_xpn_frame_cvui, a_BTN_LEFT + btn_unit * 1, a_BTN_TOP, UI_geo->get_BTN_COLS(),
                         UI_geo->get_BTN_ROWS(),
                         to_car_label)) {
            el_state.util_system_state = ING_STAT::SystemState::CAR_VIEW;
            refresh_car_6views();
            //cout << "CAR" << endl;
            la_xpn_frame_scaled = cv::Scalar(49, 52, 49);
        }

        if (cvui::button(la_xpn_frame_cvui, a_BTN_LEFT + btn_unit * 2, a_BTN_TOP, UI_geo->get_BTN_COLS(),
                         UI_geo->get_BTN_ROWS(), "Camrea")) {
            btn_camera();
        }

        if (cvui::button(la_xpn_frame_cvui, a_BTN_LEFT + btn_unit * 3, a_BTN_TOP, UI_geo->get_BTN_COLS(),
                         UI_geo->get_BTN_ROWS(), "Save")) {
            //cout << "Save Button" << endl;
            saveImage(la_xpn_frame_cvui);

            if (ing_cast_ptr->la_is_camera_ready == "camera" || ing_cast_ptr->la_is_camera_ready == "video") {
                cout << "Waiting 1 second to prevent view_name!" << endl;
                waitKey(1000);
                saveImage(ing_cast_ptr->la_using_image);
                cout << "Save using Image!" << endl;
            }
        }

        if (cvui::button(la_xpn_frame_cvui, a_BTN_LEFT + btn_unit * 4, a_BTN_TOP, UI_geo->get_BTN_COLS(),
                         UI_geo->get_BTN_ROWS(), "Pause")) {
            cj_pause = !cj_pause;
        }
        cj_pause = false ;
////////////////////////////////////////////////////////////////////////////

        //show cursor (x,y) as name of check ;
        static bool mv = true;
        static int x, y;
        string name;
        name.append(std::to_string(x)).append(",").append(std::to_string(y));
        //tLog(882, "name", name) ;

//        if ((cvui::checkbox(la_xpn_frame_cvui, ing_geo->get_A_PANO_LEFT(), ing_geo->get_a_PANO_WG_TOP() + 100, name,
//                            &mv, 0xcecece))) {
        if (a_key_hit == 32) {
            mv = false;
        }
//        if ((cvui::checkbox(la_xpn_frame_cvui, UI_geo->get_A_PANO_LEFT() + UI_geo->get_A_PANO_COLS() - 100, UI_geo->get_A_PANO_TOP() - 20, name,
//                            &mv, 0xcecece))) {
//          //if(cmd_key_hit != 32) {
//
//            int ok=system("xdotool getmouselocation 2>/dev/null | sed 's/ sc.*//; s/.://g' >> mouse_cursor");
//            if(ok != 0){
//                cout << "xdotool getmouselocation failed" << endl ;
//            }
//            else {
//                cout << "xdotool getmouselocation success" << endl;
//
//                ifstream xy;
//                xy.open("mouse_cursor");
//
//                xy >> x;
//                xy >> y;
//                xy.close();
//                system("rm mouse_cursor");
//
//                if (x < ing_cast_ptr->get_VIRTUAL_ORGIN_LEFT() - 0) {
//                    ing_cast_ptr->set_VIRTUAL_ORGIN_LEFT(x);
//                }
//                if (y < ing_cast_ptr->get_VIRTUAL_ORGIN_TOP() - 0) {
//                    ing_cast_ptr->set_VIRTUAL_ORGIN_TOP(y);
//                }
//                if (x > ing_cast_ptr->get_VIRTUAL_ORGIN_LEFT() + ing_cast_ptr->get_la_display_cols() + 0) {
//                    ing_cast_ptr->set_VIRTUAL_ORGIN_LEFT(x);
//                }
//                if (y > ing_cast_ptr->get_VIRTUAL_ORGIN_TOP() + ing_cast_ptr->get_la_display_rows() + 0) {
//                    ing_cast_ptr->set_VIRTUAL_ORGIN_TOP(y - ing_cast_ptr->get_la_display_rows());
//                }
//                cv::moveWindow(ing_cast_ptr->la_screen_title, ing_cast_ptr->get_VIRTUAL_ORGIN_LEFT(),
//                               ing_cast_ptr->get_VIRTUAL_ORGIN_TOP());
//            }
//        }

//        cvui::update(); not needed for cvui::imshow(...)

        static int prev_x = ing_cast_ptr->get_VIRTUAL_ORGIN_LEFT(), prev_y = ing_cast_ptr->get_VIRTUAL_ORGIN_TOP();

        //cout << prev_x << ", " << prev_y << " & " << cur_x << ", " << cur_y << endl;    //for debug only
        if (prev_x != cur_x || prev_y != cur_y) {
            //show cur_x, cur_y & prev_x, prev_y on screen
            ing_cast_ptr->setVirtualOrginLeft(cur_x);
            ing_cast_ptr->setVirtualOrginTop(cur_y);
            cv::moveWindow(ing_cast_ptr->la_screen_title, cur_x, cur_y);
            prev_x = cur_x;
            prev_y = cur_y;
        }
        cvui::imshow(ing_cast_ptr->la_screen_title, la_xpn_frame_cvui); //also work for cv::imshow(...)

//        std::cout << "Mouse POP: (" << cur_x << ", " << cur_y << ") " ;
//        std::cout << "VIRTUAL_ORGIN on screen: (" << ing_cast_ptr->get_VIRTUAL_ORGIN_LEFT() << ", " << ing_cast_ptr->get_VIRTUAL_ORGIN_TOP() << ")" << std::endl;

        la_xpn_DisplayCh();
        a_key_hit = cv::waitKey(1);
        if(a_key_hit != -1) {
            tLog("<cmd_key_hit != -1> i.e. a key was hit", "key code", static_cast<int>(a_key_hit));
        }

        if (a_key_hit == 'r') {
            if (el_state.recordState == ING_STAT::RecordOnOff::OFF) {
                el_state.recordState = ING_STAT::RecordOnOff::ON;
                // videoWriter.open("capture.mp4", CV_FOURCC('D', 'I', 'V', '3'), 10, Size(fix_width, fix_height));
                double aH = ing_cast_ptr->md->cfg->getImageHeight();
                videoWriter.open("capture.mp4", VideoWriter::fourcc('D', 'I', 'V', '3'), 10,
                                 Size(ing_cast_ptr->md->cfg->getImageWidth(), ing_cast_ptr->md->cfg->getImageHeight()));
                // videoWriter.open("capture.mp4", CAP_ANY, 10, Size(fix_width, fix_height));
            } else {
                el_state.recordState = ING_STAT::RecordOnOff::OFF;
                videoWriter.release();
            }
        }                                          
//        cv::moveWindow(ing_cast->la_screen_title, ing_cast->get_VIRTUAL_ORGIN_LEFT(),
//                       ing_cast->get_VIRTUAL_ORGIN_TOP());
        if ((a_key_hit == 27) || (el_state.util_system_state == ING_STAT::SystemState::EXIT)) {
            tLog("exit the endless loop", "cmd_key_hit", static_cast<int>(a_key_hit)) ;
            std::filesystem::remove("mouse_cursor");
            cv::destroyAllWindows();
            return;
        }
        else if (a_key_hit != -1) {
            tLog("A new endless loop", "cmd_key_hit", static_cast<int>(a_key_hit)) ;
        }

        cvui::text(la_xpn_frame_cvui, la_xpn_frame_cvui.cols - 430, 18, "Note: Hold ESC to EXIT", 0.65, 0xff4d4d);
        cvui::text(la_xpn_frame_cvui, la_xpn_frame_cvui.cols - 432, 20, "Note: Hold ESC to EXIT", 0.65, 0xff4d4d);
        if (cvui::button(la_xpn_frame_cvui, la_xpn_frame_cvui.cols - 70, 10, 60, 30, "Exit")) {
            exit(0);  // Exit the loop and terminate the program
        }

        //if right button of the mouse id clicked, then exit the program



//        e2 = cv::getTickCount() ;
//        double time = static_cast<double>(e2 - e1)/ cv::getTickFrequency() ;
//        tLog(940, "time", time ) ;
//
//        tStop = clock() ;
//        double elapsed = ((double) (tStop - tStart)) / CLOCKS_PER_SEC;
//        tLog(940, "elapsed", elapsed ) ;
    }
}

void Nppm::pre_run_assert() const {
    assert(ing_cast_ptr->get_la_display_cols() > 0);            //assertion for debug
    assert(ing_cast_ptr->get_la_display_rows() > 0);
    assert(ing_cast_ptr->la_display_asp > 0);
}


string float_fmt_precision(float the_delta, int fmt_precision) {
    //TITLE: Convert float to string with precision & number of decimal specified
    // https://stackoverflow.com/questions/29200635/convert-float-to-string-with-precision-number-of-decimal-digits-specified
    // #include <iomanip>
    // #include <sstream>
    std::stringstream stream;
    stream << std::fixed << std::setprecision(fmt_precision) << the_delta;
    std::string label = stream.str();
    return label;
    ///////////////////////////////
}

bool
Nppm::counters_paired_2(cj_wd_2 &a) {
    return (counters_paired(a.the_var_as_name, a.the_var, a.the_max, a.the_min,
                            a.delta_u, a.delta_l, a.precision, *a.the_var,
                            a.frame_cvui, a.left, a.top, a.top2top, a.l, a.t, a.c, a.r, a.b));
}

/*
 * const int pv_left : left from position
 * const int pv_top : top from position
 * int digit_10 : delta of upper button
 * int digit_1 :  delta of lower button
 * double updated : the updated variable
 * double cur_2 : the current value
 * int i25 : scalar from top of upper button to top of lower button
 * int i60 :
 * Mat &frame_cvui : the drawing frame
 * int &left : position refer to <pv_left>
 * int &top :  position refer do <pv_top>
 * int &cols : cols of this widget
 * int &rows : row of this widget
 * int &between : diaplacement between bottom of upper and top of lower
 * */
bool Nppm::counters_paired(string the_var_str, double *the_updated, double the_max, double the_min,
                                 float the_delta_upper, float the_delta_lower, int the_precision, double cur_on_counter,
                                 Mat &frame_cvui, const int pv_left, const int pv_top, int top2top, int &left, int &top,
                                 int &cols, int &rows, int &between) const {

    bool ret_counter_pair_updated = false;
    bool counter_clicked = false;
    cvui::text(frame_cvui, pv_left, pv_top - 20, the_var_str, 0.4, 0xff00);

    int a_left;
    int a_top;
    int a_cols;
    int a_rows;
    if (counter_clicked = counter_in(cur_on_counter, the_delta_upper, 2, frame_cvui, pv_left, pv_top, a_left, a_top,
                                     a_cols, a_rows)) {
        *the_updated = cur_on_counter;
        ret_counter_pair_updated = ret_counter_pair_updated || counter_clicked;
    }

    int b_left;
    int b_top;
    int b_cols;
    int b_rows;
    if (counter_clicked = counter_in(cur_on_counter, the_delta_lower, 2, frame_cvui, pv_left, pv_top + top2top, b_left,
                                     b_top, b_cols, b_rows)) {
        *the_updated = cur_on_counter;
        ret_counter_pair_updated = ret_counter_pair_updated || counter_clicked;;
    }

    int max_min_precision = the_precision;
    string max_min_text = string("max:").
            append(float_fmt_precision(the_max, max_min_precision)).
            append(" min:").
            append(float_fmt_precision(the_min, max_min_precision));
    cvui::text(frame_cvui, pv_left, pv_top + 2 * top2top, max_min_text, 0.4, 0xff00);

    left = a_left;
    top = a_top;
    cols = a_cols;
    between = top2top - a_rows;
    rows = a_rows + b_rows + between;
    if (ret_counter_pair_updated) {
        tLog(1202, "digit_modified", ret_counter_pair_updated);
    }
    return ret_counter_pair_updated;
}

bool
Nppm::counter_in(double &the_var, float the_delta, int fmt_precision, cv::Mat &frame_cvui, const int pv_left,
                       const int pv_top, int &left, int &top, int &cols, int &rows) const {
    bool digit_modified = false;
    double tmp_last = the_var;

    //TITLE: Convert float to string with precision & number of decimal specified
    // https://stackoverflow.com/questions/29200635/convert-float-to-string-with-precision-number-of-decimal-digits-specified
    // #include <iomanip>
    // #include <sstream>
    std::stringstream stream;
    stream << std::fixed << std::setprecision(fmt_precision) << the_delta;
    std::string label = stream.str();
    ///////////////////////////////
    cvui::text(frame_cvui, pv_left + 100, pv_top + 5, label, 0.4, 0xff00);

    //<string label(std::to_string(the_delta)) ;>
    //statement above can not control decimal precision

    the_var = cvui::counter(frame_cvui, pv_left, pv_top, &the_var, the_delta, "%.2f");
    if (the_var != tmp_last) {
        tmp_last = the_var;
        digit_modified = true;
        //cout << "digit_modified" << endl;
    }
    left = -20; //refer pv_left
    top = 0; //refer pv_top
    cols = 110; //left add width of the cvui::counter, 90
    rows = 20;  //height of the cvui::counter
    if (digit_modified) {
        tLog(1230, "digit_modified", digit_modified);
    }
    return digit_modified;
}

void Nppm::help_dump_Rt_() const {
    cout << endl << "-------ROTATE---------" << endl;
    cout << "     iC_alpha_degree: " << ing_cast_ptr->la_iC_alpha_degree << endl;
    cout << "      iC_beta_degree: " << ing_cast_ptr->la_iC_beta_degree << endl;
    cout << "la_roi_car_pano_left: " << ing_cast_ptr->la_roi_car_pano_left << endl;
    cout << "la_roi_car_pano_cols: " << ing_cast_ptr->la_roi_car_pano_cols << endl;
    cout << " la_roi_car_pano_top: " << ing_cast_ptr->la_roi_car_pano_top << endl;
    cout << "la_roi_car_pano_rows: " << ing_cast_ptr->la_roi_car_pano_rows << endl << endl;
}

void Nppm::refresh_car_6views() {
    vp::modified.viewports_redrawn = true;
    for (int ai = 0; ai < 6; ai++)
        vp::modified.car_views[ai] = true;
}

void Nppm::refresh_nppm_viewports() {
    vp::modified.viewports_redrawn = true;
    vp::modified.mode_1 = true;
    vp::modified.mode_2 = true;
    vp::modified.tube_pano = true;
    vp::modified.car_pano = true;
}

/*!
 *
 * @param name
 * @param name
 * @param left
 * @param top
 * @param unit_cols
 * @param min
 * @param max
 * @param v
 * @param size
 * @param color
 * @return
 */
bool
Nppm::multi_tracks(int multi, string name[], int left, int top, int unit_cols, double min[], double max[], double v[],
                   double size, int color) {
    double ret = false;
    for (int aI = 0; aI < multi; aI++) {

        bool update = cvui::trackbar(la_xpn_frame_cvui,
                                     left + aI * unit_cols,
                                     top,
                                     unit_cols,
                                     &v[aI],
                                     min[aI],
                                     max[aI]);

        cvui::text(la_xpn_frame_cvui,
                   left + aI * unit_cols + unit_cols / multi, top,
                   name[aI], size, color);
        ret = ret || update;
    }
    //cout << "multi_tracks: " << ret << endl ;
    return ret;
}

void Nppm::tube_pano_or_car_pano(string the_mode,
                                 double a_ui_pitch_cur_bottom,
                                 double a_ui_pitch_cur_top,
                                 double a_ui_beta_from,
                                 double a_ui_beta_span) {
    if (the_mode.compare("tube_pano") == 0) {
        assert(a_ui_pitch_cur_bottom > a_ui_pitch_cur_top);
        {

            ing_cast_ptr->md->PanoramaX((float *) la_xpn_pano_map_x.data,
                                        (float *) la_xpn_pano_map_y.data,
                                        la_xpn_pano_map_x.cols,
                                        la_xpn_pano_map_x.rows,
                                        ing_cast_ptr->md->cfg->getAspRatio(),
                                        a_ui_pitch_cur_bottom,
                                        a_ui_pitch_cur_top);
                                        //, beta_pano_begin, beta_pano_range);
                                        //a_ui_beta_from / 12 * 2 * PI, a_ui_beta_span / 12 * 2 * PI); // panorama
        }
    } else if (the_mode.compare("car_pano") == 0) {
        ing_cast_ptr->md->xPanoramaM_Rt((float *) la_xpn_rt_map_x.data,
                                        (float *) la_xpn_rt_map_y.data,
                                        la_xpn_rt_map_x.cols,
                                        la_xpn_rt_map_x.rows,
                                        ing_cast_ptr->md->cfg->getAspRatio(),
                                        (double) ing_cast_ptr->alpha_range_vender,
                                        ing_cast_ptr->la_iC_alpha_degree,
                                        ing_cast_ptr->la_iC_beta_degree,
                                        ing_cast_ptr->la_rt_alpha_from,
                                        ing_cast_ptr->la_rt_alpha_to);
    } else {
        cout << "Exit!!  No pano mode of: {" << the_mode << "}" << endl;
        exit(0);
    }
}




string Nppm::get_mapping_id(const string &pitch, const string &yaw, const string &zoom, const string &cameraID,
                                  const string &imageID) const {
    string cmn(
            ing_cast_ptr->using_mappers_folder + "im[" + imageID + "]-cmr[" + cameraID + "]-[" + pitch + "]_[" + yaw +
            "]_[" + zoom +
            "]");
    cout << "using_mappers_folder: [" << ing_cast_ptr->using_mappers_folder << "]." << endl;
    cout << "mapping file name: [" << cmn << "]." << endl;
    //exit(0) ;
    return cmn;
}

void Nppm::ui_tube_pano() {
    const int RT_UI_TUBE_PNO_NUM = 4;
    //car_pano = false;
    const int left =  UI_geo->get_A_PANO_LEFT();       //screen size dependent value
    const int top = UI_geo->get_a_PANO_WG_TOP();        //screen size dependent value
    const int unit_scale = 220; //screen size dependent value

    static double *v[RT_UI_TUBE_PNO_NUM] = {&ing_cast_ptr->alpha_pano_max_init,
                                            &ing_cast_ptr->alpha_pano_min_init,
                                            &ing_cast_ptr->beta_pano_begin,
                                            &ing_cast_ptr->beta_pano_range};
    string title[RT_UI_TUBE_PNO_NUM] = {"alpha_pano_max_init", "alpha_pano_min_init",
                                        "beta_pano_begin", "beta_pano_range"};
    double min[RT_UI_TUBE_PNO_NUM] = {2, 1, 0, 0.0};
    double max[RT_UI_TUBE_PNO_NUM] = {ing_cast_ptr->alpha_range_vender, ing_cast_ptr->alpha_range_vender - 10, 360,
                                      450};

    bool ui_tube_pano_modified[RT_UI_TUBE_PNO_NUM];

    //static int inst_idx = 0 ;
    const int pano_FIVE = 5;
    static int updated = 0;
    int tmp = updated;
    int cur = cvui::counter(la_xpn_frame_cvui, left - 120, top + 5, &updated, 1, "%d");

    if (cur != tmp) {
        updated = (cur + pano_FIVE) % pano_FIVE; //for -1
        int16_t min0 = ing_cast_ptr->alpha_pano_min;
        int16_t tmp1 = ing_cast_ptr->alpha_range_vender / 3;
        int16_t mid2 = ing_cast_ptr->alpha_range_vender / 2;
        int16_t max3 = (int16_t) ing_cast_ptr->alpha_range_vender;
        int16_t a_pano_pitch_range[pano_FIVE][2] = {
                {min0, max3},
                {min0, mid2},
                {tmp1, mid2},
                {min0, mid2},
                {tmp1, max3},
        };
        //assert(a_pano_pitch_range[ai][0] << "||" << a_pano_pitch_range[ai][1]) ;
        for (int ai = 0; ai < 5; ai++) {
            cout << a_pano_pitch_range[ai][0] << "||" << a_pano_pitch_range[ai][1] << endl;
        }
        ing_cast_ptr->alpha_pano_max_init = a_pano_pitch_range[updated][1];
        ing_cast_ptr->alpha_pano_min_init = a_pano_pitch_range[updated][0];
        vp::modified.tube_pano = true;
        tmp = updated;
    }

    for (int aI = 0; aI < RT_UI_TUBE_PNO_NUM; aI++) {
        int a_left = left + aI * unit_scale;
        cvui::text(la_xpn_frame_cvui, a_left + unit_scale / 4, top, title[aI], .4, 0xff00);
        ui_tube_pano_modified[aI] = cvui::trackbar(la_xpn_frame_cvui,
                                                   a_left,
                                                   top,
                                                   unit_scale,
                                                   v[aI],
                                                   (double) min[aI],
                                                   (double) max[aI],
                                                   1,
                                                   "%.2Lf");
        vp::modified.tube_pano = vp::modified.tube_pano || ui_tube_pano_modified[aI];
    }
}

void Nppm::btn_camera() {
    tLog(1310, "Camera Button", "OK" ) ;
    ing_cast_ptr->open_cv_camera();
    el_state.util_media_type = ING_STAT::MediaType::IMAGE_FILE;
/*
    if (el_state.util_media_type != ING_STAT::MediaType::CAMERA) {
//        tLog(1312, "el_state.util_media_typen", "no match for ‘operator<<’") ;
//        if (!(*pVC).isOpened()) {
//            ing_cast->open_cv_camera();
//            el_state.util_media_type = ING_STAT::MediaType::CAMERA;
//            tLog(1316, "el_state.util_media_typen", "no match for ‘operator<<’") ;
//        } else {
//            (*pVC).release();
//
//            tLog(1312, "el_state.util_media_typen", "no match for ‘operator<<’") ;
//        }

    }
    else {
        (*pVC).release() ;
        tLog(1325, "Camera is clsoed now", "Please check program!") ;
    }
*/
}

void Nppm::la_xpn_MatWrite(const string &filename, const Mat &mat) {
    //Problem description: Invalid arguments ' Candidates
    // are: basic_ifstream(const char *, enum
    // std::_Ios_Openmode)

    ofstream fs(filename, fstream::binary); // @suppress("Invalid arguments")

    // Header
    int type = mat.type();
    int channels = mat.channels();
    fs.write((char *) &mat.rows, sizeof(int)); // rows
    fs.write((char *) &mat.cols, sizeof(int)); // cols
    fs.write((char *) &type, sizeof(int));     // type
    fs.write((char *) &channels, sizeof(int)); // channels

    // Data
    if (mat.isContinuous()) {
        fs.write(mat.ptr<char>(0), (mat.dataend - mat.datastart));
    } else {
        int rowsz = CV_ELEM_SIZE(type) * mat.cols;
        for (int r = 0; r < mat.rows; ++r) {
            fs.write(mat.ptr<char>(r), rowsz);
        }
    }
}

Mat Nppm::la_xpn_MatRead(const string &filename) {
    ifstream fs(filename, fstream::binary); // @suppress("Invalid arguments")

    // Header
    int rows, cols, type, channels;
    fs.read((char *) &rows, sizeof(int));     // rows
    fs.read((char *) &cols, sizeof(int));     // cols
    fs.read((char *) &type, sizeof(int));     // type
    fs.read((char *) &channels, sizeof(int)); // channels

    // Data
    Mat mat(rows, cols, type);
    fs.read((char *) mat.data, CV_ELEM_SIZE(type) * rows * cols);

    return mat;
}

string Nppm::saveImage(Mat img) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y[%H-%M-%S]", timeinfo);
    std::string str(buffer);
    std::cout << str;
    string path(ing_cast_ptr->la_image_inst_folder + "/" + ing_cast_ptr->md->cfg->getCameraName() + "@" + str + ".jpg");

    cout << "imwrite:" << path << endl;
    imwrite(path, img);
    return path;
}

void Nppm::freeMemory() {
    cout << "free memory" << endl;
    ing_cast_ptr->la_using_image.release();
}

Nppm::~Nppm() {
    freeMemory();
    delete ing_cast_ptr->md;
}

const Mat &Nppm::getLaXpnFrameScaled() const {
    return la_xpn_frame_scaled;
}

void Nppm::setLaXpnFrameScaled(const Mat &laXpnFrameScaled) {
    la_xpn_frame_scaled = laXpnFrameScaled;
}
