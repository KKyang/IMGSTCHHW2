#include "getF.h"
#include <fstream>
#include <iomanip>
#include <QDebug>

void GetF::warping(std::vector<cv::Mat> &inputArrays,std::vector<double> FL2,std::vector<cv::Mat> &Output,std::vector<cv::Point> &upedge,std::vector<cv::Point> &downedge)
{

    for(int j = 0; j < inputArrays.size(); j++)
    {
        cv::Mat image = inputArrays[j].clone();

        int mid_x = image.cols/2;
        int mid_y = image.rows/2;

        double FL = FL2[j];


        cv::Mat temp=cv::Mat(image.rows,image.cols,CV_8UC3);
        temp = cv::Scalar::all(0);
        for(int b = 0; b < image.rows; b++)
        {
            for(int a = 0; a < image.cols; a++)
            {
                double theta = atan((a-mid_x)/FL);
                double h = (b-mid_y)/pow(pow((a-mid_x),2)+pow(FL,2),0.5);
                int x = FL*theta+mid_x;
                int y = FL*h+mid_y;
                temp.at<cv::Vec3b>(y, x)[0] = image.at<cv::Vec3b>(b,a)[0];
                temp.at<cv::Vec3b>(y, x)[1] = image.at<cv::Vec3b>(b,a)[1];
                temp.at<cv::Vec3b>(y, x)[2] = image.at<cv::Vec3b>(b,a)[2];
                if(b == 0)
                {
                    cv::Point temp = cv::Point(x,y);
                    upedge.push_back(temp);
                }
                else if(b==image.rows -1 )
                {
                    cv::Point temp = cv::Point(x,y);
                    downedge.push_back(temp);
                }
            }
        }
        Output.push_back(temp);
    }
}
