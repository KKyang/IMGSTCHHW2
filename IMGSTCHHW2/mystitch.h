#ifndef MYSTITCH_H
#define MYSTITCH_H
#define _USE_MATH_DEFINES

#include <QObject>
#include <math.h>
#include <QStringList>
#include "getF.h"
#include "featureproperties.h"
#include "estimate.h"
#include "myblend.h"
#include "opencv2/opencv.hpp"

class myStitch : public QObject
{
    Q_OBJECT
public:
    myStitch();
    void process(QStringList &filenames, std::vector<cv::Mat> &inputArrays, cv::Mat &outputArray);

    void SIFT(cv::Mat &inputArray, std::vector<featurePoints> &_features);
    void drawSIFTFeatures(std::vector<featurePoints> &f, cv::Mat &img);
private:
    //Difference of Gaussian Pyramid
    void DOG(cv::Mat inputArray, std::vector<cv::Mat> &_dogs, std::vector<cv::Mat> &_gpyr, const double _k, const double _sig);
    bool isExtreme(std::vector<cv::Mat> dogs, int _l, int _sl, int j, int i);
    bool interp(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i, featurePoints &f);
    //Differential function
    cv::Mat xHat(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i);
    cv::Mat diff(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i);
    cv::Mat hessian(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i);

    bool removeEdge(cv::Mat _dogImg, int j, int i);
    void orien(std::vector<featurePoints> &f, std::vector<cv::Mat> &_gpyr);
    void descriptor(std::vector<featurePoints> &f, std::vector<cv::Mat> &_gpyr);

    const double ocv = 3.0;
    const double s = 3.0;
    const double sig = 1.6;
    const double k = pow(2.0, 1.0 / s);
    const double norming = 1.0 / 255.0;
signals:
    void sendProgress(const QString &pro, int num);
};

#endif // MYSTITCH_H
