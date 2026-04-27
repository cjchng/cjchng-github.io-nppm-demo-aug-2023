#include "configdata.h"

ConfigData::ConfigData()
{
}

void ConfigData::setCameraSensorWidth(double cameraSensorWidth)
{
    this->cameraSensorWidth = cameraSensorWidth;
}


void ConfigData::setCameraSensorHeight(double cameraSensorHeight)
{
    this->cameraSensorHeight = cameraSensorHeight;
}

/*void ConfigData::setIcx(double iCx)
{
    this->iCx = iCx;
}*/
/*
void ConfigData::setIcy(double iCy)
{
    this->iCy = iCy;
}
*/
/*
 * void ConfigData ::setDcx(double dCx)
{
    this->dCx = dCx;
}
void ConfigData ::setDcy(double dCy)
{
    this->dCy = dCy;
}
 */
void ConfigData ::set_asp_ratio(double ratio)
{
    this->asp_ratio = ratio;
}
void ConfigData ::setImageWidth(double imageWidth)
{
    this->imageWidth = imageWidth;
}
void ConfigData ::setImageHeight(double imageHeight)
{
    this->imageHeight = imageHeight;
}
void ConfigData ::setParameter0(double para)
{
    this->para0 = para;
}
void ConfigData ::setParameter1(double para)
{
    this->para1 = para;
}
void ConfigData ::setParameter2(double para)
{
    this->para2 = para;
}
void ConfigData ::setParameter3(double para)
{
    this->para3 = para;
}
void ConfigData ::setParameter4(double para)
{
    this->para4 = para;
}
void ConfigData ::setParameter5(double para)
{
    this->para5 = para;
}
void ConfigData ::setCalibrationRatio(double calibrationRatio)
{
    this->calibrationRatio = calibrationRatio;
}
void ConfigData ::setCameraName(string cameraName)
{
    this->cameraName = cameraName;
}
double ConfigData ::getCameraSensorWidth()
{
    return this->cameraSensorWidth;
}
double ConfigData ::getCameraSensorHeight()
{
    return this->cameraSensorHeight;
}
//double ConfigData ::getIcx()
//{
//    return this->iCx;
//}

//double ConfigData ::getRatio()
//{
//    return this->asp_ratio;
//}
double ConfigData ::getImageWidth()
{
    return this->imageWidth;
}
double ConfigData ::getImageHeight()
{
    return this->imageHeight;
}
double ConfigData ::getParameter0()
{
    return this->para0;
}
double ConfigData ::getParameter1()
{
    return this->para1;
}
double ConfigData ::getParameter2()
{
    return this->para2;
}
double ConfigData ::getParameter3()
{
    return this->para3;
}
double ConfigData ::getParameter4()
{
    return this->para4;
}
double ConfigData ::getParameter5()
{
    return this->para5;
}
double ConfigData ::getCalibrationRatio()
{
    return this->calibrationRatio;
}
string ConfigData ::getCameraName()
{
    return this->cameraName;
}

void ConfigData ::initAlphaRho_Table()
{
    double alpha;
    for (int i = 0; i < 1800; i++)
    {
        alpha = (double)i / 10 * PI / 180;
        alphaToRho_Table[i] =
            (this->para0 * alpha * alpha * alpha * alpha * alpha * alpha + this->para1 * alpha * alpha * alpha * alpha * alpha + this->para2 * alpha * alpha * alpha * alpha + this->para3 * alpha * alpha * alpha + this->para4 * alpha * alpha + this->para5 * alpha) * this->calibrationRatio;
    }
    int i = 0, index = 0;
    while (i < 1800)
    {
        while (index < this->alphaToRho_Table[i])
        {
            rhoToAlpha_Table[index++] = i;
        }
        i++;
    }
    while (index < 3600)
    {
        rhoToAlpha_Table[index++] = i;
    }
}

int ConfigData ::getRhoFromAlpha(double alpha)
{
    return (alphaToRho_Table[(int)round(alpha * 10)]);
}

double ConfigData ::getAlphaFromRho(int rho)
{
    if (rho >= 0)
        return ((double)rhoToAlpha_Table[rho] / 10);
    else
        return (-(double)rhoToAlpha_Table[-rho] / 10);
}

double ConfigData::getAspRatio() const {
    /*
     * asp_ratio is private
     * how to define asp_ratio in
     */
    return asp_ratio;
}

void ConfigData::setAspRatio(double aspRatio) {
    asp_ratio = aspRatio;
}

double ConfigData::getICx() const {
    return iCx;
}

void ConfigData::setICx(double iCx) {
    ConfigData::iCx = iCx;
}

double ConfigData::getICy() const {
    return iCy;
}

void ConfigData::setICy(double iCy) {
    ConfigData::iCy = iCy;
}

double ConfigData::getPara1() const {
    return para1;
}

void ConfigData::setPara1(double para1) {
    ConfigData::para1 = para1;
}

double ConfigData::getPara2() const {
    return para2;
}

void ConfigData::setPara2(double para2) {
    ConfigData::para2 = para2;
}

double ConfigData::getPara3() const {
    return para3;
}

void ConfigData::setPara3(double para3) {
    ConfigData::para3 = para3;
}

double ConfigData::getPara4() const {
    return para4;
}

void ConfigData::setPara4(double para4) {
    ConfigData::para4 = para4;
}

double ConfigData::getPara5() const {
    return para5;
}

void ConfigData::setPara5(double para5) {
    ConfigData::para5 = para5;
}
