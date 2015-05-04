#ifndef MYBLEND_H
#define MYBLEND_H
#define RIGHT 1
#define LEFT 0
#include <opencv2/opencv.hpp>

class myBlend
{
public:
    myBlend();
    void multiBandBlend(cv::Mat &limg, cv::Mat &rimg, int dx, int dy);
private:
    cv::Mat getGaussianKernel(int x, int y, int dx, int dy = 0);
    void buildLaplacianMap(cv::Mat &inputArray, std::vector<cv::Mat> &outputArrays, int dx, int dy, int lr);
    void blendImg(cv::Mat &img, cv::Mat &overlap_area, int dx, int dy, int lr);
    const int level = 5;
};

#endif // MYBLEND_H
