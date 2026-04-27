#ifndef MOILDEV_H
#define MOILDEV_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <vector>
//#include <opencv2/core/hal/interface.h>
#include "configdata.h"

#define PI 3.1415926
#define PI_2 1.5707963
#define PCT_UNIT_WIDTH 1.27
#define PCT_UNIT_HEIGHT 1.27
#define FOCAL_LENGTH_FOR_ZOOM 250

#define APPROX_FACT 8
using namespace std;

class Moildev
{
public:
        class ConfigData *cfg;
        
        Moildev();
        void fish_rotate(unsigned char *dst, unsigned char *src, int cols, int rows, double c0, double r0, double degree) ;

        //static bool fastMode = false;
        bool Config(string cameraName, double cameraSensorWidth, double cameraSensorHeight,
                    double iCx, double iCy, double i_ratio,
                    double imageWidth, double imageHeight, double calibrationRatio,
                    double para0, double para1, double para2, double para3, double para4, double para5);
        //double getImageHeight();
        //double getImageWidth();
        //double getiCx();
        //double getiCy();
        //double get_asp_ratio();

 //       ConfigData *getcd();
        double AnyPointM(float *mapX, float *mapY, int w, int h, double alphaOffset, double betaOffset, double zoom, double magnification);
        /**
     *
     * @param mapX
     * @param mapY
     * @param mapper_width :width of mapX
     * @param mapper_height :width of mapY
     * @param pch_azm_pivot : zenithal angle
     * @param yaw_zth_pivot : azimutal angle
     * @param zoom
     * @param sensor_asp : aspect ratio
     * @return
     */
        double AnyPoint_M_M(float *mapX, float *mapY,
                            int mapper_width, int mapper_height,
                            double pch_azm_pivot,
                            double yaw_zth_pivot,
                            double zoom,

                            double cj_roll_degree,

                            double sensor_asp,
                            void (*fun_euler_rotate)(const double c1, const double s1, const double c2, const double s2, const double c3,
                                                     const double s3, double uX, double uY, double uZ, double &tempX, double &tempY,
                                                     double &tempZ)//,

                           // double (*fun_ptr_get_alpha)(double a_thita, double a_phi, int positionX, int positionY,
                                                        //int dcx, int dcy, double my_PCT_UNIT_WIDTH, double my_PCT_UNIT_HEIGHT,
                                                        //double myFOCAL_LENGTH_FOR_ZOOM)
                                                        ) const;

        double fastAnyPointM(float *mapX, float *mapY, int w, int h, double alphaOffset, double betaOffset, double zoom, double magnification);
        //double AnyPointM2_123(float *mapX, float *mapY, int w, int h, double yaw_left, double yaw_right,
        //                      double pitch_top, double pitch_bottom, double zoom, double magnification) const;

        double AnyPointM2(float *mapX, float *mapY, int w, int h,
                          double phi_pivot,
                          double thita_pivot,
                          double zoom,
                          double sensor_asp) const;

        double CarAnyPoint(float *mapX, float *mapY,
                           int w, int h,
                           double phi_pivot,
                           double thita_pivot,
                           double,
                           double zoom,
                           double sensor_asp) const;

        double fastAnyPointM2(float *mapX, float *mapY, int w, int h, double thetaX_degree, double thetaY_degree, double zoom, double magnification);
        //double PanoramaX(float *mapX, float *mapY, int w, int h, double magnification, double alpha_max) ;

        double PanoramaX(float *p_mapX, float *p_mapY, int p_width, int p_height, double p_magnification,
                         double p_alpha_bottom, double p_alpha_top);

        double PanoramaX(float *p_mapX, float *p_mapY, int p_width, int p_height, double p_magnification,
                         double p_alpha_bottom, double p_alpha_top, double p_beta_begin, double p_beta_end);
        /*
    double revPanorama(Mat *panoImage, Mat *result, int w, int h, double alpha_max, double beta_offset) ;
        */
/// input the obtained parameter to have ih
/// \param p_alpha_in_rad
/// \return the ih in type of double
        double alpha_to_ih_in_vga(double p_alpha_in_rad) const;
        double ab_to_ih_to_alpha(double a, double b, double icx, double icy){
            double ih = sqrt(pow((a-icx),2) + pow(b-icy, 2)) ;
            return 0 ;
//            double alpha =
        }

        double PanoramaM(float *mapX, float *mapY, int w, int h, double magnification, double alpha_max);
        double PanoramaM_Rt(float *mapX, float *mapY, int w, int h, double magnification, 
                            double alpha_max, double iC_alpha_degree, double iC_beta_degree);
        double xPanoramaM_Rt(float *p_mapX, float *p_mapY, int p_cols, int p_rows, double my_p_magnification,
                             double p_alpha_max, double p_iC_alpha_degree, double p_iC_beta_degree,
                             double p_alpha_from, double p_alpha_end) ;

        double cj_xPanoramaM_Rt(float *p_mapX, float *p_mapY, int p_cols, int p_rows, double my_p_magnification,
                        double p_alpha_max, double p_iC_alpha_degree, double p_iC_beta_degree,
                        double p_alpha_from, double p_alpha_end) ;

                                                
        int getRhoFromAlpha(double alpha);
        double getAlphaFromRho(int rho);
        vector<int> getAlphaBetaFromPos(int Mode, vector<int> Pos);

private:
        double sinArray[90 * APPROX_FACT + 1];
        void initSin();
        double ApproxSin(double angle);
        double ApproxCos(double angle);

        float ApproxAtan(float z);
        float ApproxAtan2(float y, float x);
        void obtain_xy_bound(int &x_from, int &x_end, int &y_end, int &y_from);
};

#endif
