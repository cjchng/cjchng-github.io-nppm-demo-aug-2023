#ifndef CONFIGDATA_H
#define CONFIGDATA_H
#include <string>
#include <iostream>
#include <math.h>

#define PI 3.1415926
using namespace std;

class ConfigData
{
private:
    double asp_ratio;

private:
    double iCx;
    double iCy;

    double para0;
    double para1;
    double para2;
    double para3;
    double para4;
    double para5;

    double imageWidth;
    double imageHeight;

    double cameraSensorWidth;  //first argument of camera <cfg>
    double cameraSensorHeight;

    double calibrationRatio;
    string cameraName;

    
public:

    double getAspRatio() const;
    void setAspRatio(double aspRatio);

public:
    double getPara2() const;

    void setPara2(double para2);

    double getPara3() const;

    void setPara3(double para3);

    double getPara4() const;

    void setPara4(double para4);

    double getPara5() const;
    void setPara5(double para5);


    
public:
    double getPara1() const;
    void setPara1(double para1);


public:
    double getPara0() const {
        return para0;
    }
    void setPara0(double para0) {
        ConfigData::para0 = para0;
    }


public:
    double getICy() const;
    void setICy(double iCy);

public:
    double getICx() const;
    void setICx(double iCx);


public:
    //double dCx;
    //double dCy;
    //double asp_ratio;

    


public:

    int alphaToRho_Table[1800]; // degree /10
    int rhoToAlpha_Table[3600]; // pixel
    
    ConfigData();
    
    void setCameraSensorWidth(double cameraSensorWidth);
    void setCameraSensorHeight(double cameraSensorHeight);
    //void setIcx(double iCx);
    //void setIcy(double iCy);

    void set_asp_ratio(double ratio);


    void setParameter0(double parameter0);
    void setParameter1(double parameter1);
    void setParameter2(double parameter2);
    void setParameter3(double parameter3);
    void setParameter4(double parameter4);
    void setParameter5(double parameter5);
    void setCalibrationRatio(double calibrationRatio);
    void setCameraName(string cameraName);
    double getCameraSensorWidth();
    double getCameraSensorHeight();
    //double getIcx();
    //double getRatio();

    double getImageWidth();
    void setImageWidth(double imageWidth);

    double getImageHeight();
    void setImageHeight(double imageHeight);

    double getParameter0();
    double getParameter1();
    double getParameter2();
    double getParameter3();
    double getParameter4();
    double getParameter5();
    double getCalibrationRatio();
    string getCameraName();

    void initAlphaRho_Table();
    int getRhoFromAlpha(double alpha);
    double getAlphaFromRho(int rho);
};

#endif // CONFIGDATA_H
