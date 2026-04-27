#include "configdata.h"
#include "x_moildev.h"
#include <cmath>

using namespace std;

double CJ_ONE = cos(30/180*2*3.14) ;
double CJ_ZERO = sin(30/180*2*3.14) ; //0) ;
double my_global_alpha_range_vender = 110 ;

Moildev::Moildev()
{
    cfg = new ConfigData();
    initSin();
}

bool Moildev::Config(string cameraName, double cameraSensorWidth, double cameraSensorHeight,
                     double iCx, double iCy, double i_ratio,
                     double imageWidth, double imageHeight, double calibrationRatio,
                     double para0, double para1, double para2, double para3, double para4, double para5)
{
    cfg->setCameraName(cameraName);
    cfg->setCameraSensorWidth(cameraSensorWidth);
    cfg->setCameraSensorHeight(cameraSensorHeight);
    cfg->setICx(iCx) ;
    cfg->setICy(iCy) ;
    cfg->set_asp_ratio(i_ratio);
    cfg->setImageWidth(imageWidth);
    cfg->setImageHeight(imageHeight);
    cfg->setCalibrationRatio(calibrationRatio);
    cfg->setParameter0(para0);
    cfg->setParameter1(para1);
    cfg->setParameter2(para2);
    cfg->setParameter3(para3);
    cfg->setParameter4(para4);
    cfg->setParameter5(para5);
    cfg->initAlphaRho_Table();
    return true;
}

//double Moildev::getImageWidth()
//{
//    return cfg->getImageWidth();
//}
//
//double Moildev::getImageHeight()
//{
//    return cfg->getImageHeight();
//}

//double Moildev::getiCy()
//{
//    return cfg->getIcy();
//}

//double Moildev::get_asp_ratio() // 030721@cj
//{                               /**
//                                 * parameter
//                                 */
//    return cfg->getAspRatio();
//}
//
//ConfigData *Moildev::getcd()
//{
//    return cfg;
//}

static void euler_y1x2y3(const double c1, const double s1, const double c2, const double s2, const double c3,
                         const double s3, double uX, double uY, double uZ, double &tempX, double &tempY,
                         double &tempZ)
{
    tempX = (c1 * c3 - c2 * s1 * s3) * uX + s1 * s2 * uY + (c1 * s3 + c2 * c3 * s1) * uZ;
    tempY = (s2 * s3) * uX + c2 * uY - c3 * s2 * uZ;
    tempZ = (-c3 * s1 - c1 * c2 * s3) * uX + c1 * s2 * uY + (c1 * c2 * c3 - s1 * s3) * uZ;
}

static void euler_x1y2x3(const double c1, const double s1, const double c2, const double s2, const double c3,
                         const double s3, double uX, double uY, double uZ, double &tempX, double &tempY,
                         double &tempZ)
{
    tempX = (c2)*uX + (-s2 * s3) * uY + (c3 * s2) * uZ;
    tempY = (s1 * s2) * uX + (c1 * c3 - c2 * s1 * s3) * uY + (-c1 * s3 - c2 * c3 * s1) * uZ;
    tempZ = (-c1 * s2) * uX + (c3 * s1 + c1 * c2 * c3) * uY + (c1 * c2 * c3 - s1 * s3) * uZ;
}

static void euler_z1x2z3(const double c1, const double s1, const double c2, const double s2, const double c3,
                         const double s3, double uX, double uY, double uZ, double &tempX, double &tempY,
                         double &tempZ)
{  //M1 only now 03-24-2022
    tempX = (c1 * c3 - c2 * s1 * s3) * uX + (-c1 * s3 - c2 * c3 * s1) * uY + (s1 * s2) * uZ;
    tempY = (c3 * s1 + c1 * c2 * s3) * uX + (c1 * c2 * c3 - s1 * s3) * uY + (-c1 * s2) * uZ;
    tempZ = (s2 * s3) * uX + (c3 * s2) * uY + (c2)*uZ;
}

static void euler_ZYX (const double c1, const double s1, const double c2, const double s2, const double c3,
                         const double s3, double uX, double uY, double uZ, double &tempX, double &tempY,
                         double &tempZ)
{
    tempX = uX * (c1*c2)   + uY* (c1*s2*s3-c3*s1)    + uZ* (s1*s3+c1*c3*s2) ;
    tempY = uX * (c2*s1)   + uY* (c1*c3 + s1*s2*s3)  + uZ* (c3*s1*s2-c1*s3);
    tempZ = uX * (-s2)     + uY* ( s2*s3)            + uZ* (c2*c3);
}

//car_anypoint()
static void euler_XYZ (const double c1, const double s1, const double c2, const double s2, const double c3,
                         const double s3, double uX, double uY, double uZ, double &tempX, double &tempY,
                         double &tempZ)
{
    tempX = uX * (c2*c3)            + uY* (-c2*s3)            + uZ* (s2) ;
    tempY = uX * (c1*s3+c3*s1*s2)   + uY* (c1*c3 - s1*s2*s3)  + uZ* (-c2*s1);
    tempZ = uX * (s1*s3-c1*c3*s2)   + uY* (c3*s1+c1*s2*s3)    + uZ* (c1*c2);
}


static double get_z1y2x3_alpha(double a_thita, double a_phi, int positionX, int positionY, int dcx, int dcy,
                               double my_PCT_UNIT_WIDTH,
                               double my_PCT_UNIT_HEIGHT,
                               double myFOCAL_LENGTH_FOR_ZOOM)
{
    double c1 = cos(a_thita);
    double s1 = sin(a_thita);
    double c2 = cos(a_phi);
    double s2 = sin(a_phi);
    double c3 = CJ_ONE;
    double s3 = CJ_ZERO;
    double uX = (positionX - dcx) * my_PCT_UNIT_WIDTH;
    double uY = (positionY - dcy) * my_PCT_UNIT_HEIGHT;
    double uZ = myFOCAL_LENGTH_FOR_ZOOM;
    double X, Y, Z;
    euler_z1x2z3(c1, s1, c2, s2, c3, s3, uX, uY, uZ, X, Y, Z);

    // double tempX = (c1 * c3 - c2 * s1 * s3) * uX + s1 * s2 * uY + (c1 * s3 + c2 * c3 * s1) * uZ;
    // double tempY = (s2 * s3) * uX + c2 * uY - c3 * s2 * uZ;
    // double tempZ = (-c3 * s1 - c1 * c2 * s3) * uX + c1 * s2 * uY + (c1 * c2 * c3 - s1 * s3) * uZ;
    X = -X;
    Y = -Y;
    double alpha = atan2(sqrt(X * X + Y * Y), Z);
    // cout << "fun_ptr_get_alpha(alpha): " << alpha << endl ;
    return alpha;
}


/*!
 * from <anypont> matrix, the mapped destination, inversely project its pixel coordinates on the fisheye image.
 * @param a_thita
 * @param a_phi
 * @param a_psi
 * @param positionX
 * @param positionY
 * @param dcx
 * @param dcy
 * @param my_PCT_UNIT_WIDTH
 * @param my_PCT_UNIT_HEIGHT
 * @param myFOCAL_LENGTH_FOR_ZOOM
 * @param p_fun_euler_rotate
 * @return
 */
static double get_mapped_alpha(double a_thita, double a_phi, double a_psi, int positionX, int positionY, int dcx, int dcy,
                               double my_PCT_UNIT_WIDTH,
                               double my_PCT_UNIT_HEIGHT,
                               double myFOCAL_LENGTH_FOR_ZOOM,
                               void (*p_fun_euler_rotate)(const double c1, const double s1, const double c2,
                                                          const double s2, const double c3,
                                                          const double s3, double uX, double uY, double uZ, double &tempX,
                                                          double &tempY,
                                                          double &tempZ))
{
    double c1 = cos(a_thita);
    double s1 = sin(a_thita);
    double c2 = cos(a_phi);
    double s2 = sin(a_phi);
    double c3 = cos(a_psi) ; //CJ_ONE;
    double s3 = sin(a_psi) ; //CJ_ZERO;
    double uX = (positionX - dcx) * my_PCT_UNIT_WIDTH;
    double uY = (positionY - dcy) * my_PCT_UNIT_HEIGHT;
    double uZ = myFOCAL_LENGTH_FOR_ZOOM;
    double X, Y, Z;
    p_fun_euler_rotate(c1, s1, c2, s2, c3, s3, uX, uY, uZ, X, Y, Z);
    X = -X;
    Y = -Y;
    double alpha = atan2(sqrt(X * X + Y * Y), Z);
    // cout << "fun_ptr_get_alpha(alpha): " << alpha << endl ;
    return alpha;
}

static double get_x1y2x3_alpha(double a_thita, double a_phi, int positionX, int positionY, int dcx, int dcy,
                               double my_PCT_UNIT_WIDTH,
                               double my_PCT_UNIT_HEIGHT,
                               double myFOCAL_LENGTH_FOR_ZOOM)
{
    double c1 = cos(a_thita);
    double s1 = sin(a_thita);
    double c2 = cos(a_phi);
    double s2 = sin(a_phi);
    double c3 = 1;
    double s3 = 0;
    double uX = (positionX - dcx) * my_PCT_UNIT_WIDTH;
    double uY = (positionY - dcy) * my_PCT_UNIT_HEIGHT;
    double uZ = myFOCAL_LENGTH_FOR_ZOOM;
    double X, Y, Z;
    euler_x1y2x3(c1, s1, c2, s2, c3, s3, uX, uY, uZ, X, Y, Z);

    X = -X;
    Y = -Y;
    double alpha = atan2(sqrt(X * X + Y * Y), Z);
    // cout << "fun_ptr_get_alpha(alpha): " << alpha << endl ;
    return alpha;
}

static double get_y1x2y3_alpha(double a_thita, double a_phi, int positionX, int positionY,
                               int dcx, int dcy,
                               double my_PCT_UNIT_WIDTH,
                               double my_PCT_UNIT_HEIGHT,
                               double myFOCAL_LENGTH_FOR_ZOOM)
{
    double c1 = cos(a_thita);
    double s1 = sin(a_thita);
    double c2 = cos(a_phi);
    double s2 = sin(a_phi);
    double c3 = CJ_ONE;
    double s3 = CJ_ZERO;
    double uX = (positionX - dcx) * my_PCT_UNIT_WIDTH;
    double uY = (positionY - dcy) * my_PCT_UNIT_HEIGHT;
    double uZ = myFOCAL_LENGTH_FOR_ZOOM;
    double X, Y, Z;
    euler_y1x2y3(c1, s1, c2, s2, c3, s3, uX, uY, uZ, X, Y, Z);
    // double tempX = (c1 * c3 - c2 * s1 * s3) * uX + s1 * s2 * uY + (c1 * s3 + c2 * c3 * s1) * uZ;
    // double tempY = (s2 * s3) * uX + c2 * uY - c3 * s2 * uZ;
    // double tempZ = (-c3 * s1 - c1 * c2 * s3) * uX + c1 * s2 * uY + (c1 * c2 * c3 - s1 * s3) * uZ;
    X = -X;
    Y = -Y;
    double alpha = atan2(sqrt(X * X + Y * Y), Z);
    // cout << "fun_ptr_get_alpha(alpha): " << alpha << endl ;
    return alpha;
}

/// infinite tube mode perspective transformation
///
/// \param mapX [out] Horizontal coordinates table
/// \param mapY [out] Vertical coordinates tablemy_alpha_range_vender
/// \param zoom [in]
/// \param magnification [in]
/// \return
double Moildev::AnyPointM(float *mapX, float *mapY, int w, int h,
                          double alphaOffset,
                          double betaOffset,
                          double zoom, double magnification)
{
    AnyPoint_M_M(mapX, mapY, w, h,
                 alphaOffset,
                 betaOffset, 
                 zoom,

                 0.0, // roll extension

                 magnification,
                 *euler_z1x2z3
                 //                 ,
                 //                 *get_z1x2z3_alpha
    );
    //*euler_x1y2x3,
    //(*get_x1y2x3_alpha));
    return 0;
}

///
/// \param x_from
/// \param x_end
/// \param y_end
/// \param y_from
void Moildev::obtain_xy_bound(int &x_from, int &x_end, int &y_end, int &y_from)
{
    int mag = 1;
    double dcx = cfg->getImageWidth() / 2 * mag;
    double dcy = cfg->getImageHeight() / 2 * mag;

    double SphereRadian = FOCAL_LENGTH_FOR_ZOOM * PCT_UNIT_WIDTH * 4;
    /// virtual radian length of
    x_from = (dcx - SphereRadian * mag); // * cos(PI/4.0)) ;
    x_end = (dcx + SphereRadian * mag);  // * cos(PI/4.0)) ;

    y_from = (dcy - SphereRadian * mag); // * cos(PI/4.0)) ;
    y_end = (dcy + SphereRadian * mag);  // * cos(PI/4.0)) ;

    std::cout << "dcx= " << dcx << endl;
    std::cout << "x_from= " << x_from << endl;
    std::cout << "x_end= " << x_end << endl;

    if (x_from < 0)
        x_from = 0;
    if (y_from < 0)
        y_from = 0;
    // int y_from = 0 ;
    // int y_end = 0 ;
}

/***
 * Generic AnyPoint()
 *
 * @param mapX [out]
 * @param mapY [out]
 * @param mapper_width
 * @param mapper_height
 * @param pch_azm_pivot
 * @param yaw_zth_pivot
 * @param zoom_over
 * 
 * @param cj_roll_degree
 * 
 * @param may_be_camera_asp
 * @param fun_euler_rotate
 * @return
 */
//__attribute__((access (write_only, 1, 2),
//               access (read_only, 3, 4)
//                ))
double Moildev::AnyPoint_M_M(float *mapX, float *mapY, int mapper_width, int mapper_height,
                             double pch_azm_pivot,
                             double yaw_zth_pivot,
                             double zoom_over,

                             double cj_roll_degree,

                             double may_be_camera_asp,
                             void (*fun_euler_rotate)(const double c1, const double s1,
                                                      const double c2, const double s2,
                                                      const double c3, const double s3,
                                                      double uX, double uY, double uZ,
                                                      double &tempX, double &tempY, double &tempZ)
                                                       /*,
                            double (*fun_ptr_get_alpha)(
                                    double a_thita,
                                    double a_phi,
                                    int positionX,
                                    int positionY,
                                    int dcx,
                                    int dcy,
                                    double my_PCT_UNIT_WIDTH,
                                    double my_PCT_UNIT_HEIGHT,
                                    double myFOCAL_LENGTH_FOR_ZOOM)*/
                            ) const
{   //my_* is something should be review
    double a_rcpr_zoom = 1.0 / zoom_over;
    double my_PCT_UNIT_WIDTH = 1.0;   //? <PCT phyical unit>
    double my_PCT_UNIT_HEIGHT = 1.0;
    double my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM = mapper_width / 2 * my_PCT_UNIT_WIDTH * a_rcpr_zoom;
    // default unit focal length is 60 degrees, i.e. cos(60 degree) is 1/2.
    bool to_adjust_mapper_center = true;
    /**
     * <my_vendor_ALPHA_OF_LENS> must come from Vendor's product Specification
     */
    double my_ALPHA_IN_RADIA = my_global_alpha_range_vender  * PI / 180; ////////////////////
    int my_h_offset = 0 ;
    int my_v_offset = 0 ;
    double sensor_asp = cfg->getCameraSensorHeight() / cfg->getCameraSensorWidth();

    const double icx = cfg->getICx();
    const double icy = cfg->getICy();
    double a_dcx = cfg->getImageWidth() / 2 + my_h_offset; // center of mapper, here, as 2D mapper origin
    double a_dcy = cfg->getImageHeight() / 2 + my_v_offset;
    /**
     * the mapper is set as of same dimension to cmos sensor
     */
    // cout << "a_dcx: " << a_dcx << endl;
    const double a_pch_vir_axis_in_radian = pch_azm_pivot * (PI / 180); // Scale to rotate XYZ
    const double a_yaw_vir_axis_in_radian = yaw_zth_pivot * (PI / 180);
    const double a_roll_vir_axis_in_radian = cj_roll_degree * PI / 180;
    /**
     * rotation matrix
     * ISO 8000-2:2019 r-thita-phi spherical system
     * Proper Euler Angles
     * Y1X2Y3 in
     * https://space_Ok.wikipedia.org/wiki/Euler_angles#Tait.E2.80.93Bryan_angles
     */
    double right_most;
    double left_most;
    double top_most;
    double down_most;
    {
        /***
         * View angle of the mapped boundaries are for the prevention of showing the junk area
         */
        left_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,
                                     0, mapper_height / 2,
                                     mapper_width / 2, mapper_height / 2,
                                     my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                     *fun_euler_rotate);
        right_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,
                                      mapper_width, mapper_height / 2,
                                      mapper_width / 2, mapper_height / 2,
                                      my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                      *fun_euler_rotate);

        top_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,
                                    mapper_width / 2,
                                    0, // mapper-ceiling
                                    mapper_width / 2,
                                    mapper_height / 2,
                                    my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                    *fun_euler_rotate);

        down_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,
                                     mapper_width / 2,
                                     mapper_height, // mapper-bottom
                                     mapper_width / 2,
                                     mapper_height / 2,
                                     my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                     *fun_euler_rotate);
    }

    if (right_most > my_ALPHA_IN_RADIA)
    {
        for (; a_dcx < mapper_width; a_dcx += 1)
        {

            right_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,

                                          mapper_width, mapper_height / 2,
                                          a_dcx,
                                          mapper_height / 2,
                                          my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                          *fun_euler_rotate);

            if (right_most < my_ALPHA_IN_RADIA)
            {
                break;
            }
            cout << "r-a_dcx: " << a_dcx << endl;
        }
    }
    else if (left_most > my_ALPHA_IN_RADIA)
    {

        for (; a_dcx > 0; a_dcx -= 1)
        {

            left_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,
                                         0,
                                         mapper_height / 2,
                                         mapper_width / 2,
                                         mapper_height / 2,
                                         my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                         *fun_euler_rotate);

            if (left_most < my_ALPHA_IN_RADIA)
            {
                break;
            }
            // cout << "l-a_dcx: " << a_dcx << endl;
        }
    }

    if (down_most > my_ALPHA_IN_RADIA)
    {
        for (; a_dcy < mapper_height; a_dcy += 1)
        {
            down_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,
                                         mapper_width / 2,
                                         mapper_height, ///// mapper-floor
                                         mapper_width / 2,
                                         a_dcy,
                                         my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                         *fun_euler_rotate);
            if (down_most < my_ALPHA_IN_RADIA)
            {
                break;
            }
        }
    }

    else if (top_most > my_ALPHA_IN_RADIA)
    {
        for (; a_dcy > 0; a_dcy -= 1)
        {
            top_most = get_mapped_alpha(a_yaw_vir_axis_in_radian, a_pch_vir_axis_in_radian, a_roll_vir_axis_in_radian,

                                        mapper_width / 2,
                                        0, /////mapper-ceiling
                                        mapper_width / 2,
                                        a_dcy,
                                        my_PCT_UNIT_WIDTH, my_PCT_UNIT_HEIGHT, my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM,
                                        *fun_euler_rotate);
            if (top_most < my_ALPHA_IN_RADIA)
            {
                break;
            }
        }
    }

    cout << "my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM: " << my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM << endl;
    cout << "mapper_width: " << mapper_width << endl;
    cout << endl;

    cout << "my_vendor_ALPHA_OF_LENS: " << my_ALPHA_IN_RADIA << endl;
    cout << "                    left_most: " << left_most << endl;
    cout << "                   right_most:" << right_most << endl;
    cout << "                     top_most:  " << top_most << endl;
    cout << "                    down_most: " << down_most << endl;

    double alpha_max = 0.0;
    double beta, alpha;

    const double c1 = cos(a_yaw_vir_axis_in_radian);
    const double s1 = sin(a_yaw_vir_axis_in_radian);
    const double c2 = cos(a_pch_vir_axis_in_radian);
    const double s2 = sin(a_pch_vir_axis_in_radian);
    const double c3 = cos(a_roll_vir_axis_in_radian);
    const double s3 = sin(a_roll_vir_axis_in_radian);
    int aTmp = 0;

    // a_dcx = a_dcx + mapper_width - orig_x ;
    for (int positionY = 0; positionY < mapper_height; positionY++)
    {
        for (int positionX = aTmp; positionX < mapper_width + aTmp; positionX++)
        {
            double vx = (positionX - a_dcx) * my_PCT_UNIT_WIDTH;
            double vy = (positionY - a_dcy) * my_PCT_UNIT_HEIGHT;

            double uX = vx;
            double uY = vy;
            double uZ = my_h_VIRTUAL_FOCAL_LENGTH_w_ZOOM;

            double tempX;
            double tempY;
            double tempZ;
            fun_euler_rotate(c1, s1, c2, s2, c3, s3, uX, uY, uZ, tempX, tempY, tempZ);

            alpha = atan2(sqrt(tempX * tempX + tempY * tempY), tempZ);

            tempX = -tempX;
            tempY = -tempY;
            if (alpha <= my_ALPHA_IN_RADIA) // * PI / 180) // 110 degree, max alpha of raspPI
            {
                alpha_max = (alpha_max > alpha) ? alpha_max : alpha;
                if (tempX != 0)
                    beta = atan2(tempY, tempX);
                else if (tempY >= 0)
                    beta = PI / 2;
                else
                    beta = -(PI / 2);
                ////////////////////////////////////////////////////////////////////////
                double vga_ih = alpha_to_ih_in_vga(alpha);
                /*(cfg->para0 * alpha * alpha * alpha * alpha * alpha * alpha
                        + cfg->getPara1() * alpha * alpha * alpha * alpha * alpha
                        + cfg->getPara2() * alpha * alpha * alpha * alpha
                        + cfg->getPara(3) * alpha * alpha * alpha
                        + cfg->para4 * alpha * alpha
                        + cfg->getPara(5) * alpha) ;*/
                /////////////////////////////////////////////////////////////////////////
                double opt_h = icx * cfg->getCameraSensorWidth() - // * sensor_asp -
                               vga_ih * cfg->getCalibrationRatio() // un-normalize VGA
                                   * cfg->getCameraSensorWidth() / sensor_asp * cos(beta);
                double orig_x = round(opt_h / cfg->getCameraSensorWidth());
                //////////////////////////////////////////////////////////////////////
                double opt_v = icy * cfg->getCameraSensorHeight() -
                               vga_ih * cfg->getCalibrationRatio() * cfg->getCameraSensorHeight() * sin(beta);
                double orig_y = round(opt_v / cfg->getCameraSensorHeight());
                /////////////////////////////////////////////////////////////////////
                if (orig_x >= 0 && orig_x < mapper_width && orig_y >= 0 && orig_y < mapper_height)
                {
                    *(mapX + (positionY * mapper_width + positionX)) = (float)orig_x;
                    *(mapY + (positionY * mapper_width + positionX)) = (float)orig_y;
                }
                else
                {
                    *(mapX + (positionY * mapper_width + positionX)) = 0;
                    *(mapY + (positionY * mapper_width + positionX)) = 0;
                }
            }
        }
    }
    std::cout << "..pch_azm_pivot " << pch_azm_pivot << std::endl;
    std::cout << "yaw_zth_pivot " << yaw_zth_pivot << std::endl;
    std::cout << "alpha_max " << alpha_max << std::endl << endl;
    return 0;
}

double Moildev::CarAnyPoint(float *mapX, float *mapY,
                           int w, int h,
                           double phi_pivot,
                           double thita_pivot,
                           double zoom,

                           double cj_roll_degree,

                           double sensor_asp) const
{
    AnyPoint_M_M(mapX, mapY, w, h,
                 phi_pivot,
                 thita_pivot,
                 zoom,

                 cj_roll_degree, // roll extension
                 
                 sensor_asp,
                 *euler_XYZ) ;
                 //*euler_y1x2y3);
    cout << "CarAnyPoint: cj_roll_degree: " << cj_roll_degree << endl ;
    return 0.0;
}
/**
   * ISO 8000-2:2019 r-thita-phi spherical system
   * Proper Euler Angles
   * Y1X2Y3 in
   * https://www.wikipedia.org/wiki/Euler_angles#Tait.E2.80.93Bryan_angles
   */
double Moildev::AnyPointM2(float *mapX, float *mapY,
                           int w, int h,
                           double phi_pivot,
                           double thita_pivot,
                           double zoom,
                           double sensor_asp) const
{

    CarAnyPoint(mapX, mapY,
                 w, h,
                 phi_pivot,
                 thita_pivot,
                 zoom,

                 0.0, // roll extension
                 
                 sensor_asp);
    return 0;
}

double
Moildev::fastAnyPointM(float *mapX, float *mapY, int w, int h, double alphaOffset, double betaOffset, double zoom,
                       double magnification)
{
    double icx = cfg->getICx() * magnification;
    double icy = cfg->getICy() * magnification;
    double dcx = cfg->getImageWidth() / 2 * magnification;
    double dcy = cfg->getImageHeight() / 2 * magnification;

    betaOffset += 180;
    double mAlphaOffset = alphaOffset * (PI / 180);
    double mBetaOffset = betaOffset * (PI / 180);
    double senH, senV, tempX, tempY, tempZ, beta, alpha;

    // int rows = h;
    // int cols = w;
    double origPostionX, origPostionY;
    double widthCosB = PCT_UNIT_WIDTH * cos(mBetaOffset);
    double heightCosASinB = PCT_UNIT_HEIGHT * cos(mAlphaOffset) * sin(mBetaOffset);
    double flZoomSinASinB = FOCAL_LENGTH_FOR_ZOOM * zoom * sin(mAlphaOffset) * sin(mBetaOffset);
    double widthSinB = PCT_UNIT_WIDTH * sin(mBetaOffset);
    double heightCosACosB = PCT_UNIT_HEIGHT * cos(mAlphaOffset) * cos(mBetaOffset);
    double flZoomSinACosB = FOCAL_LENGTH_FOR_ZOOM * zoom * sin(mAlphaOffset) * cos(mBetaOffset);
    double heightSinA = PCT_UNIT_HEIGHT * sin(mAlphaOffset);
    double flZoomCosA = FOCAL_LENGTH_FOR_ZOOM * zoom * cos(mAlphaOffset);
    for (int positionY = 0; positionY < h; positionY++)
    {
        for (int positionX = 0; positionX < w; positionX++)
        {
            tempX = (positionX - dcx) * widthCosB - (positionY - dcy) * heightCosASinB + flZoomSinASinB;
            tempY = (positionX - dcx) * widthSinB + (positionY - dcy) * heightCosACosB - flZoomSinACosB;
            tempZ = (positionY - dcy) * heightSinA + flZoomCosA;

            alpha = ApproxAtan2(sqrt(tempX * tempX + tempY * tempY), tempZ);

            // if (alpha <= 1.919862)
            {
                if (tempX != 0)
                    beta = ApproxAtan2(tempY, tempX);
                else if (tempY >= 0)
                    beta = PI / 2;
                else
                    beta = -(PI / 2);

                double alpha_cal = (cfg->getPara0() * alpha * alpha * alpha * alpha * alpha * alpha +
                                    cfg->getPara1() * alpha * alpha * alpha * alpha * alpha +
                                    cfg->getPara2() * alpha * alpha * alpha * alpha + 
                                    cfg->getPara3() * alpha * alpha * alpha +
                                    cfg->getPara4() * alpha * alpha +
                                    cfg->getPara5() * alpha);

                senH = icx * cfg->getAspRatio() - alpha_cal * cfg->getCalibrationRatio() * magnification * ApproxCos(beta);
                senV = icy - alpha_cal * cfg->getCalibrationRatio() * magnification * ApproxSin(beta);

                origPostionX = round(senH / cfg->getAspRatio());
                origPostionY = round(senV);
                if (origPostionX >= 0 && origPostionX < w && origPostionY >= 0 && origPostionY < h)
                {
                    *(mapX + (positionY * w + positionX)) = (float)origPostionX;
                    *(mapY + (positionY * w + positionX)) = (float)origPostionY;
                }
                else
                {
                    *(mapX + (positionY * w + positionX)) = 0;
                    *(mapY + (positionY * w + positionX)) = 0;
                }
            }
        }
    }
    return 0;
}

double
Moildev::fastAnyPointM2(float *mapX, float *mapY, int w, int h, double thetaX_degree, double thetaY_degree, double zoom,
                        double magnification)
{
    double icx = cfg->getICx() * magnification;
    double icy = cfg->getICy() * magnification;
    double dcx = cfg->getImageWidth() / 2 * magnification;
    double dcy = cfg->getImageHeight() / 2 * magnification;

    // betaOffset += 180;
    double thetaX = thetaX_degree * (PI / 180);
    double thetaY = thetaY_degree * (PI / 180);

    double senH, senV, tempX, tempY, tempZ, beta, alpha;

    double origPostionX, origPostionY;

    double widthCosB = PCT_UNIT_WIDTH * cos(thetaY);
    double heightSinASinB = PCT_UNIT_HEIGHT * sin(thetaX) * sin(thetaY);
    double flZoomCosASinB = FOCAL_LENGTH_FOR_ZOOM * zoom * cos(thetaX) * sin(thetaY);
    double heightCosA = PCT_UNIT_HEIGHT * cos(thetaX);
    double flZoomSinA = FOCAL_LENGTH_FOR_ZOOM * zoom * sin(thetaX);
    double widthSinB = PCT_UNIT_WIDTH * sin(thetaY);
    double heightSinACosB = PCT_UNIT_HEIGHT * sin(thetaX) * cos(thetaY);
    double flZoomCosACosB = FOCAL_LENGTH_FOR_ZOOM * zoom * cos(thetaX) * cos(thetaY);
    for (int positionY = 0; positionY < h; positionY++)
    {
        for (int positionX = 0; positionX < w; positionX++)
        {
            tempX = (positionX - dcx) * widthCosB + (positionY - dcy) * heightSinASinB + flZoomCosASinB;
            tempY = (positionY - dcy) * heightCosA - flZoomSinA;
            tempZ = -(positionX - dcx) * widthSinB + (positionY - dcy) * heightSinACosB + flZoomCosACosB;

            tempX = -tempX;
            tempY = -tempY;
            alpha = ApproxAtan2(sqrt(tempX * tempX + tempY * tempY), tempZ);
            // if (alpha <= 1.919862)
            {
                if (tempX != 0)
                    beta = ApproxAtan2(tempY, tempX);
                else if (tempY >= 0)
                    beta = PI / 2;
                else
                    beta = -(PI / 2);

                senH = icx * cfg->getCameraSensorWidth() * cfg->getAspRatio() -
                       (cfg->getPara0() * alpha * alpha * alpha * alpha * alpha * alpha +
                        cfg->getPara1() * alpha * alpha * alpha * alpha * alpha +
                        cfg->getPara2() * alpha * alpha * alpha * alpha +
                        cfg->getPara3() * alpha * alpha * alpha +
                        cfg->getPara4() * alpha * alpha +
                        cfg->getPara5() * alpha) *
                           cfg->getCalibrationRatio() *
                           cfg->getCameraSensorHeight() * magnification * ApproxCos(beta);

                senV = icy * cfg->getCameraSensorHeight() -
                       (cfg->getPara0() * alpha * alpha * alpha * alpha * alpha * alpha +
                        cfg->getPara1() * alpha * alpha * alpha * alpha * alpha +
                        cfg->getPara2() * alpha * alpha * alpha * alpha +
                        cfg->getPara3() * alpha * alpha * alpha +
                        cfg->getPara4() * alpha * alpha +
                        cfg->getPara5() * alpha) *
                           cfg->getCalibrationRatio() *
                           cfg->getCameraSensorHeight() * magnification * ApproxSin(beta);
                origPostionX = round(senH / (cfg->getCameraSensorWidth() * cfg->getAspRatio()));
                origPostionY = round(senV / cfg->getCameraSensorHeight());
                if (origPostionX >= 0 && origPostionX < w && origPostionY >= 0 && origPostionY < h)
                {
                    *(mapX + (positionY * w + positionX)) = (float)origPostionX;
                    *(mapY + (positionY * w + positionX)) = (float)origPostionY;
                }
                else
                {
                    *(mapX + (positionY * w + positionX)) = 0;
                    *(mapY + (positionY * w + positionX)) = 0;
                }
            }
        }
    }

    return 0;
}
/// find ih given known alpha
/// \param p_alpha_in_rad
/// \return

double Moildev::alpha_to_ih_in_vga(double p_alpha_in_rad) const
{
    return (cfg->getPara0() * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad *
                p_alpha_in_rad +
            cfg->getPara1() * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad +
            cfg->getPara2() * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad +
            cfg->getPara3() * p_alpha_in_rad * p_alpha_in_rad * p_alpha_in_rad +
            cfg->getPara4() * p_alpha_in_rad * p_alpha_in_rad +
            cfg->getPara5() * p_alpha_in_rad);
}

#include <cassert>

double Moildev::PanoramaX(float *p_mapX, float *p_mapY, int p_width, int p_height,
                          double p_magnification, double p_alpha_max, double p_alpha_min)
{
    PanoramaX(p_mapX, p_mapY, p_width, p_height, p_magnification,
              p_alpha_max, p_alpha_min, 0.0, 2 * PI);
    return 0.0;
}

double Moildev::PanoramaX(float *p_mapX, float *p_mapY, int p_width, int p_buf_height,
                          double p_magnification, double p_alpha_bottom, double p_alpha_top, double p_beta_begin,
                          double p_beta_interval)
{
    // cout << "PanoramaM 0006" << endl;

    double a_icx = cfg->getICx() * p_magnification;
    double a_icy = cfg->getICy() * p_magnification;

    double a_alpha, a_beta;
    int a_max_row_seq = p_buf_height;
    int a_max_col_seq = p_width;
    double a_srcPostionX, a_srcPostionY;
    int a_mem_inc = 0;

    // double alphaBegin = p_alpha_max ;
    // p_alpha_max = 90.0 ;
    cout << "       cfg->iCx:" << cfg->getICx() << endl;
    cout << "       cfg->iCy:" << cfg->getICy() << endl;
    cout << "          a_icx:" << a_icx << endl;
    cout << "          a_icy:" << a_icy << endl;
    cout << "        p_width:" << p_width << endl;
    cout << "       p_height:" << p_buf_height << endl;
    cout << "p_magnification:" << p_magnification << endl;
    cout << "    p_alpha_max:" << p_alpha_bottom << endl;
    cout << endl
         << "cfg->getCalibrationRatio():" << cfg->getCalibrationRatio() << endl;

    p_magnification = 1;

    const double D2R = 2.0 * PI / 360.0;
    const double R2D = 360.0 / 2.0 / PI;
    double brickH;
    double a_zone;
    {
        static bool a_first = true;
        if (a_first)
        { //(1) same focal length for pinhole and fisheye
            //(2) the veiwport can be larger then 180 degrees
            //(3) p_height map to the IH of perspective pano
            //(4) p_width map to beta [0..2*pi]
            //
            //

            double alpha_top_bootom[2] = {p_alpha_top, p_alpha_bottom}; //--------modify here---------
            assert(alpha_top_bootom[0] >= 1.0);                         // limit

            double alphaS_from_90[2] = {90 - alpha_top_bootom[0], 90 - alpha_top_bootom[1]}; //{65, -20} ;
            assert(alphaS_from_90[0] > alphaS_from_90[1]);

            double zone[2];
            double zone_levels[2];
            double radius_of_pano = p_width / (2.0 * PI);
            // width of the buffer will process beta from [0..2*PI]
            cout << "[radius_of_pano]or[normalized height] = " << radius_of_pano << endl;

            for (unsigned int i = 0; i < 2; i++) // top and bottom
            {                                    // normalized IH to pinhole camera
                cout << "zone[" << i << "] in unit of RAD. = " << (zone[i] = tan(alphaS_from_90[i] * D2R)) << endl;
            }

            double zone_range = zone[0] - zone[1]; // range of IH be process
            cout << "zone_length = " << zone_range << endl;

            for (unsigned int i = 0; i < 2; i++)
            {
                zone_levels[i] = zone[i] / zone_range;
                cout << "zone_levels[i] = " << zone_levels[i] << endl;
            }

            // unsigned int inter_horizon = p_buf_height * (abs(zone_levels[0])) ;
            //  cout << "inter_horizon = " << inter_horizon << endl ;
            brickH = zone_range / p_buf_height;
            cout << "brickH = " << brickH << endl
                 << endl;

            double topBrick = brickH * zone_levels[0];
            double bottomBrick = brickH * zone_levels[1];
            cout << "   topBrick= " << topBrick << endl;
            cout << "bottomBrick= " << bottomBrick << endl
                 << endl;

            int a20th = 1;
            if (p_buf_height > 40)
            {
                a20th = p_buf_height / 20;
            }

            cout << "a20h = " << a20th << endl
                 << endl; // show 20 lines for IH checking

            for (int i = 0; i <= p_buf_height; i++)
            {
                if (i % a20th == 0)
                {
                    cout << i << " brickH, alpha=" << (90.0 - atan(zone[0] - brickH * i) * R2D);
                    cout << "   IH = " << alpha_to_ih_in_vga((90.0 - atan(zone[0] - brickH * i) * R2D) * D2R) << endl;
                }
            }
            cout << p_buf_height << " brickH, alpha=" << (90.0 - atan(zone[0] - brickH * p_buf_height) * R2D);
            cout << "   IH = " << alpha_to_ih_in_vga((90.0 - atan(zone[0] - brickH * p_buf_height) * R2D) * D2R) << endl;
            a_zone = zone[0];
        }
    }

    for (int a_positionY = 0; a_positionY < a_max_row_seq; a_positionY++) // PanoH
    {
        // a_alpha = (double)a_positionY / (double)a_positionY * p_alpha_max * PI / 180;
        // double alpha_cal = (cfg->getPara0() * a_alpha * a_alpha * a_alpha * a_alpha * a_alpha * a_alpha + cfg->getPara1() * a_alpha * a_alpha * a_alpha * a_alpha * a_alpha + cfg->getPara2() * a_alpha * a_alpha * a_alpha * a_alpha + cfg->getPara(3) * a_alpha * a_alpha * a_alpha + cfg->para4 * a_alpha * a_alpha + cfg->getPara(5) * a_alpha);

        a_alpha = (90.0 - atan(a_zone - brickH * a_positionY) * R2D);
        double alpha_cal = alpha_to_ih_in_vga(a_alpha * D2R);

        for (int a_positionX = 0; a_positionX < a_max_col_seq; a_positionX++)
        {
            a_beta = (double)a_positionX / (double)a_max_col_seq * 2 * PI * (p_beta_interval) / (2 * PI);

            a_beta = (PI - a_beta);
            a_beta += p_beta_begin;

            a_srcPostionX = round(a_icx - alpha_cal * cfg->getCalibrationRatio() * p_magnification * cos(a_beta));
            a_srcPostionY = round(a_icy - alpha_cal * cfg->getCalibrationRatio() * p_magnification * sin(a_beta));
            if (a_srcPostionX >= 0 && a_srcPostionX < a_max_col_seq && a_srcPostionY >= 0 &&
                a_srcPostionY < a_max_row_seq)
            {
                *(p_mapX + (a_mem_inc)) = (float)a_srcPostionX;
                *(p_mapY + (a_mem_inc++)) = (float)a_srcPostionY;
            }
            else
            {
                *(p_mapX + (a_mem_inc)) = 0;
                *(p_mapY + (a_mem_inc++)) = 0;
            }
        }
    }
    return 0;
}

double Moildev::PanoramaM(float *p_mapX, float *p_mapY, int p_width, int p_height, double p_magnification,
                          double p_alpha_max)
{
    double a_icx = cfg->getICx() * p_magnification;
    double a_icy = cfg->getICy() * p_magnification;
    double a_alpha, a_beta;
    int a_rows = p_height;
    int a_cols = p_width;
    double a_origPostionX, a_origPostionY;
    int a_mem_inc = 0;
    for (int a_positionY = 0; a_positionY < a_rows; a_positionY++)
    {
        a_alpha = (double)a_positionY / (double)a_rows * p_alpha_max * PI / 180;
        double alpha_cal = (cfg->getPara0() * a_alpha * a_alpha * a_alpha * a_alpha * a_alpha * a_alpha +
                            cfg->getPara1() * a_alpha * a_alpha * a_alpha * a_alpha * a_alpha +
                            cfg->getPara2() * a_alpha * a_alpha * a_alpha * a_alpha +
                            cfg->getPara3() * a_alpha * a_alpha * a_alpha +
                            cfg->getPara4() * a_alpha * a_alpha +
                            cfg->getPara5() * a_alpha);

        for (int a_positionX = 0; a_positionX < a_cols; a_positionX++)
        {
            a_beta = (double)a_positionX / (double)a_cols * 2 * PI;
            a_beta = -1 * (PI / 2 - a_beta);

            a_origPostionX = round(a_icx - alpha_cal * cfg->getCalibrationRatio() * p_magnification * cos(a_beta));
            a_origPostionY = round(a_icy - alpha_cal * cfg->getCalibrationRatio() * p_magnification * sin(a_beta));
            if (a_origPostionX >= 0 && a_origPostionX < a_cols && a_origPostionY >= 0 && a_origPostionY < a_rows)
            {
                *(p_mapX + (a_mem_inc)) = (float)a_origPostionX;
                *(p_mapY + (a_mem_inc++)) = (float)a_origPostionY;
            }
            else
            {
                *(p_mapX + (a_mem_inc)) = 0;
                *(p_mapY + (a_mem_inc++)) = 0;
            }
        }
    }
    return 0;
}

double Moildev::PanoramaM_Rt(float *mapX, float *mapY, int w, int h, double magnification, double alpha_max,
                             double iC_alpha_degree, double iC_beta_degree)
{
    return (xPanoramaM_Rt(mapX, mapY, w, h, magnification, alpha_max,
                          iC_alpha_degree, iC_beta_degree, 0.5, 1.5));
}

// spherical coordinate system :
// https://zh.wikipedia.org/wiki/%E7%90%83%E5%BA%A7%E6%A8%99%E7%B3%BB
// Rodrigues' rotation formula :
// https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
// Mathenatics :
// https://math.stackexchange.com/questions/1019910/rotation-matrix-in-spherical-coordinates
// Cross product :
// https://zh.wikipedia.org/wiki/%E5%8F%89%E7%A7%AF
// Inner product :
// https://en.wikipedia.org/wiki/Dot_product


double Moildev::xPanoramaM_Rt(float *p_mapX, float *p_mapY, int p_cols, int p_rows, 
                                double my_p_magnification, double my_p_alpha_max_vendor,
                                double p_iC_alpha_degree, double p_iC_beta_degree, 
                                double p_alpha_from, double p_alpha_end)
{  
    double icx = cfg->getICx() * my_p_magnification;
    double icy = cfg->getICy() * my_p_magnification;
 
    //const int a_p_rows = p_rows;
    //const int a_p_cols = p_cols;
    double origPostionX, origPostionY;
    int mem_offset = 0;
    double iC_alpha_pivot = p_iC_alpha_degree * PI / 180;
    double iC_beta_pivot = -p_iC_beta_degree * PI / 180;
    // iC_beta_pivot = PI/2 - iC_beta_pivot;

    //rotation axis K
    const double kx = sin(iC_alpha_pivot) * cos(iC_beta_pivot) ;
    const double ky = sin(iC_alpha_pivot) * sin(iC_beta_pivot) ;
    const double kz = cos(iC_alpha_pivot) ;
   
   //08042022
    double max_ing_alpha = 0;
    double min_ing_alpha = 3.14 ;
    double max_fish_alpha = 0;
    double min_fish_alpha = 3.14 ;

    double alpha_unit = (p_alpha_end - p_alpha_from) / p_rows ;

    //for (int cur_row = 0; cur_row < p_rows; cur_row++)
    for (int cur_row = p_alpha_from * p_rows; cur_row < p_alpha_end * p_rows; cur_row++)
    //from 0.5*1440<720> to 1440*1.5<2160>
    {
        double ing_alpha ;
        //ing_alpha = (cur_row * alpha_unit + p_alpha_from) * my_global_alpha_range_vender * PI /180.0;
        ing_alpha = (double)cur_row / (double)p_rows * my_global_alpha_range_vender * PI / 180;
        // ing_alpha 0.5*92.5*3.14/180

        const double target_alpha = iC_alpha_pivot + ing_alpha;
        double const Vx = sin(target_alpha) * cos(iC_beta_pivot);
        const double Vy = sin(target_alpha) * sin(iC_beta_pivot);
        const double az = cos(target_alpha);

        const double kxa_x = ky * az - kz * Vy;
        const double kxa_y = kz * Vx - kx * az;
        const double kxa_z = kx * Vy - ky * Vx;
        const double k_a = kx * Vx + ky * Vy + kz * az;

        for (int cur_col = 0; cur_col < p_cols; cur_col++)
        {
            // 0 <= <cur_beta> < 2*PI
            double  ing_beta;
            ing_beta = (double)cur_col / (double)p_cols * 2 * PI;
            double ing_sin_beta = sin(ing_beta);
            double ing_cos_beta = cos(ing_beta);
            double V_rot_x = ing_cos_beta * Vx + kxa_x * ing_sin_beta + kx * k_a * (1 - ing_cos_beta);
            double V_rot_y = ing_cos_beta * Vy + kxa_y * ing_sin_beta + ky * k_a * (1 - ing_cos_beta);
            double V_rot_z = ing_cos_beta * az + kxa_z * ing_sin_beta + kz * k_a * (1 - ing_cos_beta);

            double fish_alpha, fish_beta;
            fish_beta = atan2(V_rot_y, V_rot_x);
            fish_alpha = atan2(sqrt(pow(V_rot_x, 2) + pow(V_rot_y, 2)), V_rot_z);
            fish_beta = PI / 2 - fish_beta;

            //double alpha2vghIH = alpha_to_ih_in_vga(alpha);

            origPostionX = 1.0 * round(icx - alpha_to_ih_in_vga(fish_alpha) * cfg->getCalibrationRatio() * my_p_magnification * cos(fish_beta));
            origPostionY = 1.0 * round(icy - alpha_to_ih_in_vga(fish_alpha) * cfg->getCalibrationRatio() * my_p_magnification * sin(fish_beta));
            
            if (origPostionX >= 0 && origPostionX < p_cols && origPostionY >= 0 && origPostionY < p_rows) // && fish_alpha < my_global_alpha_range_vender /180 * PI)
            {
                *(p_mapX + (mem_offset)) = (float)origPostionX;
                *(p_mapY + (mem_offset++)) = (float)origPostionY;

                max_ing_alpha = max_ing_alpha < ing_alpha ? ing_alpha : max_ing_alpha ;
                min_ing_alpha = min_ing_alpha < ing_alpha ? min_ing_alpha : ing_alpha ;

                max_fish_alpha = max_fish_alpha < fish_alpha ? fish_alpha : max_fish_alpha ;
                min_fish_alpha = min_fish_alpha < fish_alpha ? min_fish_alpha : fish_alpha ;
            }
            else
            {
                *(p_mapX + (mem_offset)) = 0;
                *(p_mapY + (mem_offset++)) = 0;
            }
        }
    }
    cout << endl <<endl ;
    cout <<"max_ing_alpha: " << max_ing_alpha << endl ;
    cout <<"min_ing_alpha: " << min_ing_alpha << endl ;

    cout <<"max_fish_alpha: " << max_fish_alpha << endl ;
    cout <<"min_fish_alpha: " << min_fish_alpha << endl << endl;
    cout <<" my_global_alpha_range_vender: " << my_global_alpha_range_vender << endl << endl;

    return 0;
}



double Moildev::cj_xPanoramaM_Rt(float *p_mapX, float *p_mapY, int p_cols, int p_rows, 
                                double my_p_magnification, double my_p_alpha_max_vendor,
                                double p_iC_alpha_degree, double p_iC_beta_degree, 
                                double p_alpha_from, double p_alpha_end)
{  
    double icx = cfg->getICx() * my_p_magnification;
    double icy = cfg->getICy() * my_p_magnification;

    //const int a_p_rows = p_rows;
    //const int a_p_cols = p_cols;
    double origPostionX, origPostionY;
    int mem_offset = 0;
    double iC_alpha_pivot = p_iC_alpha_degree * PI / 180;
    double iC_beta_pivot = -p_iC_beta_degree * PI / 180;
    // iC_beta_pivot = PI/2 - iC_beta_pivot;

    //rotation axis
    const double kx = sin(iC_alpha_pivot) * cos(iC_beta_pivot) ;
    const double ky = sin(iC_alpha_pivot) * sin(iC_beta_pivot) ;
    const double kz = cos(iC_alpha_pivot) ;
   
   //08042022
    double max_ing_alpha = 0;
    double min_ing_alpha = 3.14 ;

    double max_fish_alpha = 0;
    double min_fish_alpha = 3.14 ;

    double alpha_unit = (p_alpha_end - p_alpha_from) / p_rows ;

    for (int cur_row = 0; cur_row < p_rows; cur_row++)
    //from 0.5*1440<720> to 1440*1.5<2160>
    {
        double ing_alpha ;
        ing_alpha = (cur_row * alpha_unit + p_alpha_from) * my_global_alpha_range_vender * PI /180.0;
        ing_alpha = (double)cur_row / (double)p_rows * my_global_alpha_range_vender * PI / 180;

        // ing_alpha 0.5*92.5*3.14/180

        const double target_alpha = iC_alpha_pivot + ing_alpha;
        double const Vx = sin(target_alpha) * cos(iC_beta_pivot);
        const double Vy = sin(target_alpha) * sin(iC_beta_pivot);
        const double az = cos(target_alpha);

        const double kxa_x = ky * az - kz * Vy;
        const double kxa_y = kz * Vx - kx * az;
        const double kxa_z = kx * Vy - ky * Vx;
        const double k_a = kx * Vx + ky * Vy + kz * az;

        for (int cur_col = 0; cur_col < p_cols; cur_col++)
        {
            // 0 <= <cur_beta> < 2*PI
            double  ing_beta;
            ing_beta = (double)cur_col / (double)p_cols * 2 * PI;
            double ing_sin_beta = sin(ing_beta);
            double ing_cos_beta = cos(ing_beta);
            double V_rot_x = ing_cos_beta * Vx + kxa_x * ing_sin_beta + kx * k_a * (1 - ing_cos_beta);
            double V_rot_y = ing_cos_beta * Vy + kxa_y * ing_sin_beta + ky * k_a * (1 - ing_cos_beta);
            double V_rot_z = ing_cos_beta * az + kxa_z * ing_sin_beta + kz * k_a * (1 - ing_cos_beta);
            double fish_alpha, fish_beta;
            

            fish_beta = atan2(V_rot_y, V_rot_x);
            // alpha = atan2(sqrt(pow(V_rot_x, 2) + pow(V_rot_y, 2)), V_rot_z);
            fish_alpha = atan2(sqrt(pow(V_rot_x, 2) + pow(V_rot_y, 2)), V_rot_z);
            fish_beta = PI / 2 - fish_beta;

            //double alpha2vghIH = alpha_to_ih_in_vga(alpha);

            origPostionX = 1.0 * round(icx - alpha_to_ih_in_vga(fish_alpha) * cfg->getCalibrationRatio() * my_p_magnification * cos(fish_beta));
            origPostionY = 1.0 * round(icy - alpha_to_ih_in_vga(fish_alpha) * cfg->getCalibrationRatio() * my_p_magnification * sin(fish_beta));
            
            if (origPostionX >= 0 && origPostionX < p_cols && origPostionY >= 0 && origPostionY < p_rows) // && fish_alpha < my_global_alpha_range_vender /180 * PI)
            {
                *(p_mapX + (mem_offset)) = (float)origPostionX;
                *(p_mapY + (mem_offset++)) = (float)origPostionY;

                max_ing_alpha = max_ing_alpha < ing_alpha ? ing_alpha : max_ing_alpha ;
                min_ing_alpha = min_ing_alpha < ing_alpha ? min_ing_alpha : ing_alpha ;

                max_fish_alpha = max_fish_alpha < fish_alpha ? fish_alpha : max_fish_alpha ;
                min_fish_alpha = min_fish_alpha < fish_alpha ? min_fish_alpha : fish_alpha ;
            }
            else
            {
                *(p_mapX + (mem_offset)) = 0;
                *(p_mapY + (mem_offset++)) = 0;
            }
        }
    }
    cout << endl <<endl ;
    cout <<"max_ing_alpha: " << max_ing_alpha << endl ;
    cout <<"min_ing_alpha: " << min_ing_alpha << endl ;

    cout <<"max_fish_alpha: " << max_fish_alpha << endl ;
    cout <<"min_fish_alpha: " << min_fish_alpha << endl << endl;

    cout <<" my_global_alpha_range_vender: " << my_global_alpha_range_vender << endl << endl;

    return 0;
}

/*
#include <opencv2/opencv.hpp>
using namespace cv;

double Moildev::revPanorama(Mat *panoImage, Mat *result, int w, int h, double alpha_max, double beta_offset)
{
    Mat frame(h, w, CV_8UC3, panoImage);
    Mat frame2(h, w, CV_8UC3, result);

    double icx = camera_iCx;
    double icy = camera_iCy;
    double alpha, beta;
    int rows = frame.rows;
    int cols = frame.cols;
    double origPostionX, origPostionY;

    for (int positionY = 0; positionY < rows; positionY++)
    {
        for (int positionX = 0; positionX < cols; positionX++)
        {
            double r = sqrt(pow(positionX - icx, 2) + pow(positionY - icy, 2));
            alpha = getAlphaFromRho((int)r);
            beta = atan2(icy - positionY, positionX - icx) - (PI / 2);
            beta += beta_offset * PI / 180;
            while (beta < 0)
                beta += PI * 2;
            origPostionX = round(camera_imageWidth * beta / (PI * 2));
            origPostionY = round(camera_imageHeight * alpha / alpha_max);
            if (origPostionX >= 0 && origPostionX < cols && origPostionY >= 0 && origPostionY < rows)
            {
                frame2.at<Vec3b>(positionY, positionX) = frame.at<Vec3b>(origPostionY, origPostionX);
            }
            else
            {
                frame2.at<Vec3b>(positionY, positionX) = Vec3b(0, 0, 0);
            }
        }
    }
    return 0;
}
*/

// ref : https://en.wikipedia.org/wiki/Sine_wave
void Moildev::initSin()
{
    for (int i = 0; i <= (90 * APPROX_FACT); i++)
    {
        sinArray[i] = sin((double)i * PI / 180 / APPROX_FACT);
    }
}

double Moildev::ApproxSin(double angle)
{
    int deg = round(angle * 180 * APPROX_FACT / PI);
    deg %= 360 * APPROX_FACT;
    if (deg < 0)
        deg += 360 * APPROX_FACT;
    if (deg <= 90 * APPROX_FACT)
        return sinArray[deg];
    else if (deg <= 180 * APPROX_FACT)
    {
        return sinArray[180 * APPROX_FACT - deg]; // 90 - ( deg - 90 )
    }
    else if (deg <= 270 * APPROX_FACT)
    {
        return -sinArray[deg - 180 * APPROX_FACT];
    }
    else
    {                                              // > 270
        return -sinArray[360 * APPROX_FACT - deg]; // 270 - (deg - 270 ) - 180
    }
}

void Moildev::fish_rotate(unsigned char *dst, unsigned char *src, int cols, int rows,
                          double c0, double r0, double degree) {
   double rads = degree *3.1416/180.0 ;
   cout <<endl << "void Moildev::fish_rotate()+++" << endl ;
   for (int r=0;r<rows;r++)
	{
		for(int l=0;l<cols;l++)
		{
            int r1, c1;
            r1=(int)(r0+((r-r0)*cos(rads))-((l-c0)*sin(rads)));
			c1=(int)(c0+((r-r0)*sin(rads))+((l-c0)*cos(rads)));
			if(r1>=0 && r1 < rows && c1 >=0 && c1 << cols){
                for(int chn=0; chn<3; chn++)
                 *(dst+ (r1 * cols + c1)*3+chn) = *(src + (r*cols + l)*3+chn) ;
            }
			// // dst[r1][c1]=src[r][l];
		}
        // cout << "row:" << r << endl ;
        // cout << "cols:" << cols << endl ;
	}
    cout << "void Moildev::fish_rotate()---" << endl << endl ;
}


double Moildev::ApproxCos(double angle)
{
    return ApproxSin(angle + PI_2);
}

// ref : https://www.dsprelated.com/showarticle/1052.php
float Moildev::ApproxAtan(float z)
{
    const float n1 = 0.97239411f;
    const float n2 = -0.19194795f;
    return (n1 + n2 * z * z) * z;
}

float Moildev::ApproxAtan2(float y, float x)
{
    if (x != 0.0f)
    {
        if (fabsf(x) > fabsf(y))
        {
            const float z = y / x;
            if (x > 0.0)
            {
                // atan2(y,x) = atan(y/x) if x > 0
                return ApproxAtan(z);
            }
            else if (y >= 0.0)
            {
                // atan2(y,x) = atan(y/x) + PI if x < 0, y >= 0
                return ApproxAtan(z) + PI;
            }
            else
            {
                // atan2(y,x) = atan(y/x) - PI if x < 0, y < 0
                return ApproxAtan(z) - PI;
            }
        }
        else // Use property atan(y/x) = PI/2 - atan(x/y) if |y/x| > 1.
        {
            const float z = x / y;
            if (y > 0.0)
            {
                // atan2(y,x) = PI/2 - atan(x/y) if |y/x| > 1, y > 0
                return -ApproxAtan(z) + PI_2;
            }
            else
            {
                // atan2(y,x) = -PI/2 - atan(x/y) if |y/x| > 1, y < 0
                return -ApproxAtan(z) - PI_2;
            }
        }
    }
    else
    {
        if (y > 0.0f) // x = 0, y > 0
        {
            return PI_2;
        }
        else if (y < 0.0f) // x = 0, y < 0
        {
            return -PI_2;
        }
    }
    return 0.0f; // x,y = 0. Could return NaN instead.
}

int Moildev::getRhoFromAlpha(double alpha)
{
    return cfg->getAlphaFromRho(alpha);
}

double Moildev::getAlphaFromRho(int rho)
{
    return cfg->getAlphaFromRho(rho);
}

vector<int> Moildev::getAlphaBetaFromPos(int Mode, vector<int> Pos)
{
    vector<int> ab;
    double alpha, beta;
    double delta_x = (Pos[0] - cfg->getICx());
    double delta_y = -(Pos[1] - cfg->getICy());

    if (Mode == 0)
    {
        int r = int(round(sqrt(pow(delta_x, 2) + pow(delta_y, 2))));
        alpha = getAlphaFromRho(r);

        double angle;
        if (delta_x == 0)
            angle = 0;
        else
            angle = (atan2(delta_y, delta_x) * 180) / 3.14159265;
        beta = 90 - angle;
    }
    else
    {
        alpha = getAlphaFromRho(int(round(delta_y)));
        beta = getAlphaFromRho(int(round(delta_x)));
    }
    ab.push_back(alpha);
    ab.push_back(beta);
    return ab;
}