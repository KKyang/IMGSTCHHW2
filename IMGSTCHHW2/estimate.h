#ifndef ESTIMATE_H
#define ESTIMATE_H
#define _USE_MATH_DEFINES
#define SEARCH_ITER 100

#include <math.h>
#include "featureproperties.h"
#include "opencv2/opencv.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/stitching/stitcher.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"

class estimate
{
public:
    estimate();
    //This function calculates the sift match points and estimates the focal length of the pictures
    void process(std::vector<std::vector<featurePoints>> &f, std::vector<cv::Size> &pic_size, std::vector<std::vector<cv::DMatch>> &good_matches, std::vector<double> &focal);

    void drawMatches(cv::Mat &img1, std::vector<featurePoints> &f1, cv::Mat &img2, std::vector<featurePoints> &f2, std::vector<cv::DMatch> good_matches);
    void alignMatches(cv::Mat &img1, std::vector<featurePoints> &f1, cv::Mat &img2, std::vector<featurePoints> &f2, std::vector<cv::DMatch> good_matches,
                     std::vector<int> &x,std::vector<int> &y,double FL1,double FL2);
private:
    //These two functions convert our structure to OpenCV's structure. We use OpenCV's flann and focal estimation to get the parameters.
    void toCVImageFeatures(std::vector<std::vector<featurePoints> > &f, std::vector<cv::Size> &pic_size, std::vector<cv::detail::ImageFeatures> &ifs);
    void toCVDescriptor(std::vector<featurePoints> &f, cv::Mat &d);

};

#endif // ESTIMATE_H
