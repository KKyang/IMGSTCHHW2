#ifndef GETF_H
#define GETF_H

#include <QString>
#include <QByteArray>
#include <time.h>
#include "exif.h"
#include "opencv2/opencv.hpp"

class GetF
{
public:
    //GetF();
    void getFocalL(QStringList &list,std::vector<double> &focalL);
    void warping(std::vector<cv::Mat> &inputArrays,std::vector<double> FL2,std::vector<cv::Mat> &Output,std::vector<cv::Point> &upedge,std::vector<cv::Point> &downedge);

    //std::vector<cv::Mat> warping(std::vector<cv::Mat> &inputArrays,std::vector<double>FL2);
private:
    std::vector<double> FocalL;

};


#endif // GETF_H
