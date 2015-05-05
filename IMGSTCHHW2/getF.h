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
    void warping(std::vector<cv::Mat> &inputArrays,std::vector<double> FL2,std::vector<cv::Mat> &Output,std::vector<cv::Point> &upedge,std::vector<cv::Point> &downedge);
private:
    std::vector<double> FocalL;

};


#endif // GETF_H
