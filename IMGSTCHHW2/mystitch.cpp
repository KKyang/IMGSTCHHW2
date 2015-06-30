#include "mystitch.h"
#include <QDebug>

bool cmp(featurePoints a, featurePoints b)
{
    return (a.scale_subl < b.scale_subl);
}

myStitch::myStitch()
{
}

//The class main process function
void myStitch::process(QStringList &filenames, std::vector<cv::Mat> &inputArrays, cv::Mat &outputArray)
{
    if(filenames.empty())
    {
        qDebug() << "Error. No file names";
        return;
    }

    if(inputArrays.empty())
    {
        qDebug() << "Error. No images";
        return;
    }
    for(int i = 0; i < inputArrays.size(); i++)
    {
        if(inputArrays[i].empty())
        {
            qDebug() << "Is empty!";
            return;
        }
    }

    std::vector<std::vector<featurePoints>> p(inputArrays.size());
    std::vector<cv::Size> pic_size;

    for(int k = 0;k< inputArrays.size();k++)
    {
        sendProgress(QString("Finding SIFT keypoints (Pair: " + QString::number(k + 1) + " / " + QString::number(inputArrays.size()) + ")"), 0 + (int)(60.0 * ((double)(k - 1) / inputArrays.size())));
        SIFT(inputArrays[k], p[k]);
        pic_size.push_back(cv::Size(inputArrays[k].cols, inputArrays[k].rows));
    }

    estimate m;
    std::vector<std::vector<cv::DMatch>> gm;
    std::vector<double> FL;
    sendProgress(QString("Finding matching points & focal length (pixels)"), 60);
    m.process(p, pic_size, gm, FL);


    std::vector<int> dx;
    std::vector<int> dy;
    sendProgress(QString("Align matches"), 75);
    for(int k = 1; k < inputArrays.size(); k++)
    {

        m.alignMatches(inputArrays[k-1], p[k - 1], inputArrays[k], p[k], gm[k - 1], dx, dy, FL[k-1], FL[k]);
    }

    sendProgress(QString("Warping images"), 85);
    std::vector <cv::Point> upedge;
    std::vector <cv::Point> downedge;
    std::vector <cv::Mat> warpingImg;
    GetF focal;
    focal.warping(inputArrays, FL,warpingImg,upedge,downedge);

    myBlend blend;

    for(int i = 0; i < warpingImg.size() - 1; i++)
    {
        sendProgress(QString("Blending images (Pair: " + QString::number(i + 1) + " / " + QString::number(inputArrays.size() - 1) + ")"), 90 + (int)(5.0 * ((double)(k - 1) / inputArrays.size())));
        int dyy;
        if(dy[i] > warpingImg[i].rows)
        {
            dyy = -1*(warpingImg[i].rows-abs(warpingImg[i].rows-dy[i]));
        }
        else
        {
            dyy = dy[i];
        }

        blend.multiBandBlend(warpingImg[i], warpingImg[i + 1], dx[i], dyy);
    }

    sendProgress(QString("Writing images into panorama"), 95);
    cv::Mat left;
    int cols = 0;
    int rows = 0;
    for(int i = 0; i < warpingImg.size(); i++)
    {
        if( i != warpingImg.size() - 1)
        {
            if(dy[i] > warpingImg[i].rows)
            {
                rows +=  warpingImg[i].rows -warpingImg[i].rows;
            }
            else
                rows += warpingImg[i].rows - dy[i];
        }
        else
        {
            rows += warpingImg[i].rows;
        }

        if( i != warpingImg.size() - 1)
            cols += warpingImg[i].cols - dx[i];
        else
            cols += warpingImg[i].cols;
    }



    cv::Mat result;
    cv::Size size(cols,rows*2);
    result.create(size,CV_MAKETYPE(left.depth(),3));
    result = cv::Scalar::all(0);
    left = result(cv::Rect(0,warpingImg[0].rows/10,warpingImg[0].cols,warpingImg[0].rows+warpingImg[0].rows/10));
    warpingImg[0].copyTo(left);
    int distance = 0;
    int distancey = warpingImg[0].rows/10;
    //cv::Point uppoint;
    double minb = warpingImg[0].rows/2;
    double mina = warpingImg[0].cols/2;
    double maxb = warpingImg[0].rows/2;
    double maxa = warpingImg[0].cols/2;
    double upb = 0.0;
    double upa = 0.0;
    double downb = 0.0;
    double downa = 0.0;

    for(int b = distancey; b < distancey+warpingImg[0].rows; b++)
        for(int a = distance; a < warpingImg[0].cols+distance; a++)
        {
            if(b-distancey>=0 && b-distancey<warpingImg[0].rows && a-distance>=0 && a-distance<warpingImg[0].cols )
            {
                result.at<cv::Vec3b>(b, a)[0] = warpingImg[0].at<cv::Vec3b>(b-distancey,a-distance)[0];
                result.at<cv::Vec3b>(b, a)[1] = warpingImg[0].at<cv::Vec3b>(b-distancey,a-distance)[1];
                result.at<cv::Vec3b>(b, a)[2] = warpingImg[0].at<cv::Vec3b>(b-distancey,a-distance)[2];
            }
        }

    for(int i =1;i<=dx.size();i++)
    {
        distance = distance + warpingImg[i-1].cols - dx[i-1];
        if(dy[i-1]>warpingImg[i-1].rows)
        {
            distancey = distancey -abs(warpingImg[i-1].rows - dy[i-1]);
        }
        else
        {
            distancey = distancey +abs(warpingImg[i-1].rows- dy[i-1]);
        }


        for(int b = distancey; b < distancey+warpingImg[i].rows; b++)
        {

            for(int a = distance; a < warpingImg[i].cols+distance; a++)
            {
                if(b<0)
                {

                }
                else
                {
                    if(result.at<cv::Vec3b>(b, a)[0] == 0 && result.at<cv::Vec3b>(b, a)[1] == 0 && result.at<cv::Vec3b>(b, a)[2] == 0)
                    {
                        if(b-distancey>=0 && b-distancey<warpingImg[i].rows && a-distance>=0 && a-distance<warpingImg[i].cols )
                        {
                            result.at<cv::Vec3b>(b, a)[0] = warpingImg[i].at<cv::Vec3b>(b-distancey,a-distance)[0];
                            result.at<cv::Vec3b>(b, a)[1] = warpingImg[i].at<cv::Vec3b>(b-distancey,a-distance)[1];
                            result.at<cv::Vec3b>(b, a)[2] = warpingImg[i].at<cv::Vec3b>(b-distancey,a-distance)[2];
                        }
                    }
                    if(b < minb)
                    {
                        minb = b;
                        mina = a;
                        upb =b;
                        upa = a;
                    }
                    if(b>maxb)
                    {
                        maxb = b;
                        maxa = a;
                        downb = b-warpingImg[i].rows;
                        downa = a;
                    }
                }
            }
        }
    }
#ifdef _DEBUG
    qDebug()<< minb << mina << maxb << maxa;
#endif
    cv::Mat rotateImg;
    //rotateImg.create(cv::Size(),CV_MAKETYPE(rotateImg.depth(),3);
    //result.create(size,CV_MAKETYPE(left.depth(),3));
    double angle = atan((upb-downb)/(upa-downa))*180/3.14159;
    if(upa == downa)
    {
        angle = 0;
    }

    cv::Point2f pt(result.cols/2,result.rows/2);
    cv::Mat r = cv::getRotationMatrix2D(pt,angle,1.0);
    cv::warpAffine(result,rotateImg,r,cv::Size(result.cols,result.rows));

    int upline =0;
    int downline=result.rows;
    for(int b= 0;b<rotateImg.rows;b++)
    {
        int upn = 0;
        int downn = 0;
        for(int a=0;a<rotateImg.cols;a++)
        {
            if(rotateImg.at<cv::Vec3b>(b, a)[0] == 0 && rotateImg.at<cv::Vec3b>(b, a)[1] == 0 && rotateImg.at<cv::Vec3b>(b, a)[2] == 0)
            {
                if(b<rotateImg.rows/2)
                {
                    upn++;
                    if(upn == rotateImg.cols  && upline < b )
                    {
                        upline = b;
                    }
                }
                else
                {
                    downn++;
                    if(downn == rotateImg.cols  && downline >b)
                    {
                        downline = b;
                    }
                }
            }
        }
    }



    cv::Mat cutImage;
    int finalrows = downline - upline+1;
    int finalcols = rotateImg.cols;
    cv::Size sizefinal(finalcols,finalrows);
    cutImage.create(sizefinal,CV_MAKETYPE(rotateImg.depth(),3));
    cutImage = cv::Scalar::all(0);
#ifdef _DEBUG
    qDebug()<<result.rows<<finalrows<<result.cols<<finalcols;
#endif
    for(int b = 0; b < cutImage.rows; b++)
    {
        for(int a = 0; a < cutImage.cols; a++)
        {
            //qDebug()<<b+minb<<a;
            if(b +upline >= rotateImg.rows){}
            else
            {
                cutImage.at<cv::Vec3b>(b, a)[0] = rotateImg.at<cv::Vec3b>(b+upline,a)[0];
                cutImage.at<cv::Vec3b>(b, a)[1] = rotateImg.at<cv::Vec3b>(b+upline,a)[1];
                cutImage.at<cv::Vec3b>(b, a)[2] = rotateImg.at<cv::Vec3b>(b+upline,a)[2];
            }
        }
    }



    int leftline =0;
    int rightline=cutImage.cols;
    for(int a= 0;a<cutImage.cols;a++)
    {
        int leftn = 0;
        int rightn = 0;
        for(int b=0;b<cutImage.rows;b++)
        {
            if(cutImage.at<cv::Vec3b>(b, a)[0] == 0 && cutImage.at<cv::Vec3b>(b, a)[1] == 0 && cutImage.at<cv::Vec3b>(b, a)[2] == 0)
            {
                if(a<cutImage.cols/2)
                {
                    leftn++;
                    if(leftn == cutImage.rows  && leftline < a )
                    {
                        leftline = a;
                    }
                }
                else
                {
                    rightn++;
                    if(rightn == cutImage.rows  && rightline >a)
                    {
                        rightline = a;
                    }
                }
            }
        }
    }



    cv::Mat cutImage2;
    int finalrows2 = cutImage.rows;
    int finalcols2 = rightline-leftline;
    cv::Size sizefinal2(finalcols2,finalrows2);
    cutImage2.create(sizefinal2,CV_MAKETYPE(cutImage.depth(),3));
    cutImage2 = cv::Scalar::all(0);
#ifdef _DEBUG
    qDebug()<<result.rows<<finalrows2<<result.cols<<finalcols2;
#endif
    for(int b = 0; b < cutImage2.rows; b++)
    {
        for(int a = 0; a < cutImage2.cols; a++)
        {
            //qDebug()<<b+minb<<a;
            if(a+leftline >= cutImage.cols){}
            else
            {
                cutImage2.at<cv::Vec3b>(b, a)[0] = cutImage.at<cv::Vec3b>(b,a+leftline)[0];
                cutImage2.at<cv::Vec3b>(b, a)[1] = cutImage.at<cv::Vec3b>(b,a+leftline)[1];
                cutImage2.at<cv::Vec3b>(b, a)[2] = cutImage.at<cv::Vec3b>(b,a+leftline)[2];
            }
        }
    }





    outputArray.release();
    outputArray = cutImage2.clone();
    sendProgress(QString("Done"), 100);
}

void myStitch::DOG(cv::Mat inputArray, std::vector<cv::Mat> &_dogs, std::vector<cv::Mat> &_gpyr, const double _k, const double _sig)
{
    if(inputArray.type() == CV_8UC3)
    {
        cv::cvtColor(inputArray, inputArray, cv::COLOR_BGR2GRAY);
    }

    if(inputArray.type() == CV_8UC1)
    {
        inputArray.convertTo(inputArray, CV_32FC1);
    }

    std::vector<double> si;
    si.push_back(_sig);

    int i = 0;
    for(i = 1; i < (int)(s + 3); i++)
    {
        double prev = pow(_k, (double)(i - 1)) * _sig;
        double news = prev * _k;
        si.push_back(std::sqrt(news * news - prev * prev));
    }
    si[0] = std::sqrt(_sig * _sig - 0.25);

    for(int b = 0; b < (int)ocv; b++) // 3
    {
        for(int a = 0; a < (int)(s + 3); a++) // 6
        {
            if(a == 0 && b == 0)
            {
                cv::GaussianBlur(inputArray, _gpyr[0], cv::Size(0, 0), si[a], si[a]);
            }
            else if(a == 0)
            {
               cv::resize(_gpyr[(b - 1) * (s + 3) + s], _gpyr[b * (s + 3) + a], cv::Size(_gpyr[(b - 1) * (s + 3) + s].cols / 2.0, _gpyr[(b - 1) * (s + 3) + s].rows / 2.0), 0, 0, cv::INTER_NEAREST);
            }
            else
            {
                cv::GaussianBlur(_gpyr[b * (s + 3) + a - 1], _gpyr[b * (s + 3) + a], cv::Size(0, 0), si[a], si[a]);
                cv::Mat tmp(_gpyr[b * (s + 3) + (a - 1)].rows, _gpyr[b * (s + 3) + (a - 1)].cols, CV_32FC1);

                #pragma omp parallel for private(i)
                for (int j = 0; j < tmp.rows; j++)
                {
                    for (i = 0; i < tmp.cols; i++)
                    {
                        tmp.at<float>(j, i) = _gpyr[b * (s + 3) + a].at<float>(j, i) - _gpyr[b * (s + 3) + a - 1].at<float>(j, i);
                    }
                }

                //Build the pyramid
                _dogs.push_back(tmp.clone());
            }
        }
    }
}

//SIFT main function
void myStitch::SIFT(cv::Mat &inputArray, std::vector<featurePoints> &_features)
{
    std::vector<cv::Mat> gpyr((int)(ocv * (s + 3)));
    std::vector<cv::Mat> dogs;

    DOG(inputArray, dogs, gpyr, k, sig);

    featurePoints f;

    std::vector<featurePoints> siftFeatures;
    for(int b = 0; b < ocv; b++)
        for(int a = 1; a <= s; a++)
        {
            for(int j = 1; j < dogs[(a) + b * (s + 2)].rows - 1; j++)
                for(int i = 1; i < dogs[(a) + b * (s + 2)].cols - 1; i++)
                {
                    //Threshold of half of 0.03
                    if(abs(dogs[(a) + b * (s + 2)].at<float>(j, i)) > cvFloor(0.5 * 0.03 / s * 255))
                        if(isExtreme(dogs, b, a, j, i))
                        {
                            if(interp(dogs, b, a, j, i, f))
                            {
                                siftFeatures.push_back(f);
                            }
                        }

                }
        }


    orien(siftFeatures, gpyr);

    descriptor(siftFeatures, gpyr);

    //Copy features to output vector
    _features.clear();
    _features.resize(siftFeatures.size());
    for(int i = 0; i < siftFeatures.size(); i++)
    {
        _features[i] = siftFeatures[i];
    }
}

bool myStitch::isExtreme(std::vector<cv::Mat> dogs, int _l, int _sl, int j, int i)
{
    int status = dogs[(_sl) + _l * (s + 2)].at<float>(j, i) > dogs[(_sl + 1) + _l * (s + 2)].at<float>(j, i) ? 2 : 1;

    if(status == 1)
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            int&& tx = i + x;
            int&& ty = j + y;
            if((tx >= 0 && tx < dogs[(_sl) + _l * (s + 2)].cols && ty >= 0 && ty < dogs[(_sl) + _l * (s + 2)].rows))
            {
                for(int z = -1; z <= 1; z++)
                {
                    if(dogs[(_sl) + _l * (s + 2)].at<float>(j, i) > dogs[(_sl + z) + _l * (s + 2)].at<float>(ty, tx))
                    {
                        return false;
                    }
                }
            }
        }
    }
    else if(status == 2)
    for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
            int&& tx = i + x;
            int&& ty = j + y;
            if((tx >= 0 && tx < dogs[(_sl) + _l * (s + 2)].cols && ty >= 0 && ty < dogs[(_sl) + _l * (s + 2)].rows))
            {
                for(int z = -1; z <= 1; z++)
                {
                    if(dogs[(_sl) + _l * (s + 2)].at<float>(j, i) < dogs[(_sl + z) + _l * (s + 2)].at<float>(ty, tx))
                    {
                        return false;
                    }
                }
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}


bool myStitch::interp(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i, featurePoints &f)
{
    int a = 0;
    cv::Mat x;

    //Scale extreme point location
    int interpnum = 5;
    for(a = 0; a < interpnum; a++)
    {
        x = xHat(_dog, _layer, _sublayer, j, i);
        // 0.5 from Lowe's paper
        if(abs(x.at<double>(0, 0)) < 0.5 && abs(x.at<double>(1, 0)) < 0.5 && abs(x.at<double>(2, 0)) < 0.5)
            break;

        i += cvRound(x.at<double>(0, 0));
        j += cvRound(x.at<double>(1, 0));
        _sublayer += cvRound(x.at<double>(2, 0));


        if(_sublayer < 1 || _sublayer > (int)(s)|| j >= _dog[_layer * (s + 2) + _sublayer].rows - 1
                                                || i >= _dog[_layer * (s + 2) + _sublayer].cols - 1
                                                || j < 1 || i < 1)
        {
            return false;
        }
    }
    //If fail to find the precise location
    if(a >= interpnum)
        return false;

    //Count the contrast threshold
    cv::Mat D2(1, 1, CV_64FC1);
    cv::Mat &&dD = diff(_dog, _layer, _sublayer, j, i);
    cv::gemm(dD, x, 1, NULL, 0, D2, cv::GEMM_1_T);
    double thresh = (double)_dog[_layer * (s + 2) + _sublayer].at<float>(j, i) * norming + D2.at<double>(0, 0) * 0.5;

    if(abs(thresh) * s < 0.03)
    {
        return false;
    }

    //Remove if is not edge
    if(!removeEdge(_dog[_layer * (s + 2) + _sublayer], j, i))
        return false;

    //Write data into feature point structure
    f.x = i;
    f.y = j;
    f.l = _layer;
    f.subl = _sublayer;
    f.xHat[0] = (i + x.at<double>(0, 0)) * pow(2.0, _layer);
    f.xHat[1] = (j + x.at<double>(1, 0)) * pow(2.0, _layer);
    f.xHat[2] = x.at<double>(2, 0);

    double _sub = f.subl + f.xHat[2];
    f.scale_subl = sig * pow(2.0, f.l + _sub / s);
    f.scale = sig * pow(2.0, _sub / s);
    f.response = abs(thresh);

    return true;
}

cv::Mat myStitch::xHat(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i)
{
    cv::Mat &&dD = diff(_dog, _layer, _sublayer, j, i);
    cv::Mat &&dH = hessian(_dog, _layer, _sublayer, j, i);

    cv::Mat invdH(3, 3, CV_64FC1);
    cv::invert(dH, invdH, cv::DECOMP_SVD);

    cv::Mat x(3, 1, CV_64FC1);
    cv::gemm(invdH, dD, -1, NULL, 0, x);

    return x;
}

cv::Mat myStitch::diff(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i)
{
    double dx, dy, ds;
    dx = ((double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j, i + 1)) - (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j, i - 1))) / 2.0 * norming;
    dy = ((double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j + 1, i)) - (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j - 1, i))) / 2.0 * norming;
    ds = ((double)(_dog[_layer * (s + 2) + _sublayer + 1].at<float>(j, i)) - (double)(_dog[_layer * (s + 2) + _sublayer - 1].at<float>(j, i))) / 2.0 * norming;

    cv::Mat result(3, 1, CV_64FC1);
    result.at<double>(0, 0) = dx;
    result.at<double>(1, 0) = dy;
    result.at<double>(2, 0) = ds;

    return result;
}

cv::Mat myStitch::hessian(std::vector<cv::Mat> _dog, int _layer, int _sublayer, int j, int i)
{
    double c, dxx, dxy, dyy, dys, dss, dxs;
    c = (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j, i));
    dxx = ((double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j, i + 1)) + (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j, i - 1)) - 2 * c) * norming;
    dxy = ((double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j + 1, i + 1)) + (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j - 1, i - 1))
          -(double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j - 1, i + 1)) - (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j + 1, i - 1))) / 4.0 * norming;
    dyy = ((double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j + 1, i)) + (double)(_dog[_layer * (s + 2) + _sublayer].at<float>(j - 1, i)) - 2 * c) * norming;
    dys = ((double)(_dog[_layer * (s + 2) + _sublayer + 1].at<float>(j + 1, i)) + (double)(_dog[_layer * (s + 2) + _sublayer - 1].at<float>(j - 1, i))
          -(double)(_dog[_layer * (s + 2) + _sublayer - 1].at<float>(j + 1, i)) - (double)(_dog[_layer * (s + 2) + _sublayer + 1].at<float>(j - 1, i))) / 4.0 * norming;
    dss = ((double)(_dog[_layer * (s + 2) + _sublayer + 1].at<float>(j, i)) + (double)(_dog[_layer * (s + 2) + _sublayer - 1].at<float>(j, i)) - 2 * c) * norming;
    dxs = ((double)(_dog[_layer * (s + 2) + _sublayer + 1].at<float>(j, i + 1)) + (double)(_dog[_layer * (s + 2) + _sublayer - 1].at<float>(j, i - 1))
          -(double)(_dog[_layer * (s + 2) + _sublayer + 1].at<float>(j, i - 1)) - (double)(_dog[_layer * (s + 2) + _sublayer - 1].at<float>(j, i + 1))) / 4.0 * norming;

    cv::Mat result(3, 3, CV_64FC1);
    result.at<double>(0, 0) = dxx;
    result.at<double>(0, 1) = dxy;
    result.at<double>(0, 2) = dxs;
    result.at<double>(1, 0) = dxy;
    result.at<double>(1, 1) = dyy;
    result.at<double>(1, 2) = dys;
    result.at<double>(2, 0) = dxs;
    result.at<double>(2, 1) = dys;
    result.at<double>(2, 2) = dss;

    return result;
}

bool myStitch::removeEdge(cv::Mat _dogImg, int j, int i)
{
    double c, dxx, dxy, dyy;
    c = (double)(_dogImg.at<float>(j, i));
    dxx = ((double)(_dogImg.at<float>(j, i + 1)) + (double)(_dogImg.at<float>(j, i - 1)) - 2 * c) * norming;
    dxy = ((double)(_dogImg.at<float>(j + 1, i + 1)) + (double)(_dogImg.at<float>(j - 1, i - 1))
          -(double)(_dogImg.at<float>(j - 1, i + 1)) - (double)(_dogImg.at<float>(j + 1, i - 1))) / 4.0 * norming;
    dyy = ((double)(_dogImg.at<float>(j + 1, i)) + (double)(_dogImg.at<float>(j - 1, i)) - 2 * c) * norming;

    double tr = dxx + dyy;
    double dH = dxx * dyy - (dxy * dxy);

    if(dH <= 0 || tr * tr / dH >= pow(11.0, 2) / 10.0)
        return false;

    return true;
}

void myStitch::orien(std::vector<featurePoints> &f, std::vector<cv::Mat> &_gpyr)
{

  int fs = f.size();

  int dir;
  double weight;
  std::vector<featurePoints> tmp;


  for(int a = 0; a < fs; a++)
  {
    // 36 directions
    std::vector<double> h(36);

    int radius = cvRound(1.5 * 3.0 * f[a].scale);
    double _sig = 2.0 * pow(1.5 * f[a].scale, 2);

    for(int j = - radius; j <= radius; j++)
      for(int i = - radius; i <= radius; i++)
      {
          int tx = f[a].x + i;
          int ty = f[a].y + j;
          if(ty >= 1 && ty < _gpyr[f[a].l * (s + 3) + f[a].subl].rows - 1 && tx >= 1 && tx < _gpyr[f[a].l * (s + 3) + f[a].subl].cols - 1)
          {
            double dx = _gpyr[f[a].l * (s + 3) + f[a].subl].at<float>(ty, tx + 1) - _gpyr[f[a].l * (s + 3) + f[a].subl].at<float>(ty, tx - 1);
            double dy = _gpyr[f[a].l * (s + 3) + f[a].subl].at<float>(ty - 1, tx) - _gpyr[f[a].l * (s + 3) + f[a].subl].at<float>(ty + 1, tx);
            double &&mag = sqrt(dx * dx + dy * dy);
            double &&the = atan2(dy , dx);

            if(the >= M_PI * 2){the -= (M_PI * 2);}
            if(the < 0){the += (M_PI * 2);}

            weight = exp( - ( j * j + i * i ) / _sig );
            dir = cvRound((h.size() / 360.0f) * ((the * 180.0) / M_PI));

            if(dir >= h.size()){dir -= h.size();}
            if(dir < 0){ dir += h.size();}

            h[dir] += weight * mag;
          }
      }

    //Apply Gaussian of 3x3 mask
    double prev = h[h.size() - 1], next;
    std::vector<double> t(h.size());
    for(int i = 0; i < h.size(); i++)
    {
        i + 1 >= h.size() ? next = h[0] : next = h[i + 1];
        t[i] = 0.25 * prev + 0.5 * h[i] + 0.25 * next;
        prev = h[i];
    }
    int i = 0;

#pragma omp parallel for
    for(i = 0; i < h.size(); i++)
    {
        h[i] = t[i];
    }
    t.clear();

    double val = h[0];
    for(int i = 1; i < h.size(); i++)
    {
        if(h[i] > val)
        {
            val = h[i];
        }
    }

    int x, y;

    //Find 0.8 * peak of the orientation. 15% of points will have more than one orientation features
    for(int i = 0; i < h.size(); i++ )
    {
        x = (i == 0            ? h.size() - 1 : i - 1);
        y = (i == h.size() - 1 ? 0            : i + 1);

        if(h[i] > h[x]  &&  h[i] > h[y]  &&  h[i] >= val * 0.8)
        {
            double ddir = i + (0.5 * (h[x] - h[y]) / (h[x] - 2 * h[i] + h[y]));
            if(ddir < 0){ddir = h.size() + ddir;}
            if(ddir >= h.size()){ddir = ddir - h.size();}
            featurePoints t = f[a];
            t.orien = (360.0 - (ddir * (360.0 / h.size()))) * M_PI / 180.0;
            tmp.push_back(t);
        }
    }
  }

  f.clear();
  for(int i = 0; i < tmp.size(); i++)
  {
      f.push_back(tmp[i]);
  }
}

void myStitch::descriptor(std::vector<featurePoints> &f, std::vector<cv::Mat> &_gpyr)
{
    //8 orientation, 4 * 4 descriptore (Lowe Sec 6.2)
    const int bins = 8;
    int fs = f.size();
    for(int a = 0; a < fs; a++)
    {
        cv::Mat& img = _gpyr[f[a].l * (s + 3) + f[a].subl];
        double h_w = 3.0 * f[a].scale_subl;
        double w = img.cols;
        double h = img.rows;
        int radius = std::min(cvRound(h_w * sqrt(2) * (4 + 1.0) * 0.5), (int)sqrt(w * w + h * h));
        double _sig = 4 * 4 * 0.5;

        double sin_t = sin(M_PI * 2 - f[a].orien);
        double cos_t = cos(M_PI * 2 - f[a].orien);

        for(int j = -radius; j <= radius; j++)
            for(int i = -radius; i <= radius; i++)
            {
                double j_rot = (i * sin_t + j * cos_t) / h_w; //i_rot, j_rot converts the 36 bin direction to 8 bin
                double i_rot = (i * cos_t - j * sin_t) / h_w;
                double i_bin = i_rot + 4 / 2 - 0.5;  //i_bin, j_bin is the 4 * 4 matrix position
                double j_bin = j_rot + 4 / 2 - 0.5;

                if(j_bin > -1.0 && j_bin < 4 && i_bin > -1.0 && i_bin < 4)
                {
                    int tx = f[a].x + i;
                    int ty = f[a].y + j;
                    if(ty > 0 && ty < img.rows - 1 && tx > 0 && tx < img.cols - 1)
                    {
                        //Calculate rientation
                        double dx = img.at<float>(ty, tx + 1) - img.at<float>(ty, tx - 1);
                        double dy = img.at<float>(ty - 1, tx) - img.at<float>(ty + 1, tx);
                        double &&mag = sqrt(dx * dx + dy * dy);
                        double &&the = atan2(dy , dx);

                        if(the >= M_PI * 2){the -= (M_PI * 2);}
                        if(the < 0){the += (M_PI * 2);}

                        double weight = exp(-(i_rot * i_rot + j_rot * j_rot)/ _sig);
                        mag = weight * mag;

                        double orien_bin = ((the - (M_PI * 2 - f[a].orien)) * 180.0 / M_PI) * (bins / 360.0);

                        double ob = cvFloor(orien_bin);
                        double ib = cvFloor(i_bin);
                        double jb = cvFloor(j_bin);

                        double do_bin = orien_bin - ob;
                        double di_bin = i_bin - ib;
                        double dj_bin = j_bin - jb;

                        //Make sure ob is in 0 - 8 dimension
                        if(ob < 0){ob += bins;}
                        if(ob >= bins){ob -= bins;}

                        //put the orientation descriptor into the right dimension
                        for(int y = 0; y <= 1; y++)
                        {
                            int yy = jb + y;
                            if(yy >= 0 && yy < 4)
                            {
                                double mag_yy;
                                if(j == 0)
                                    mag_yy = mag * (1.0 - (dj_bin));
                                else
                                    mag_yy = mag * dj_bin;

                                for(int x = 0; x <= 1; x++)
                                {
                                    int xx = ib + x;
                                    if(xx >= 0 && xx < 4)
                                    {
                                        double mag_xx;
                                        if(x == 0)
                                            mag_xx = mag_yy * (1.0 - (di_bin));
                                        else
                                            mag_xx = mag_yy * di_bin;
                                        for(int o = 0; o <= 1; o++)
                                        {
                                            double mag_oo;
                                            if(o == 0)
                                                mag_oo = mag_xx * (1.0 - (do_bin));
                                            else
                                                mag_oo = mag_xx * do_bin;

                                            int oo = (int)(ob + o) % 8;
                                            f[a].h[yy][xx][oo] += mag_oo;
                                        }
                                    }
                                }
                            }
                        }
                    }

                }
            }
        //norm
        double tmp = 0;
        for(int j = 0; j < 4; j++)
            for(int i = 0; i < 4; i++)
                for(int o = 0; o < 8; o++)
                {
                    tmp += f[a].h[j][i][o] * f[a].h[j][i][o];

                }

        for(int j = 0; j < 4; j++)
            for(int i = 0; i < 4; i++)
                for(int o = 0; o < 8; o++)
                {
                    double h_val = f[a].h[j][i][o] * 1.0 / sqrt(tmp * tmp);
                    if(h_val > 0.2){h_val = 0.2;}
                    f[a].h[j][i][o] = h_val;
                }

        tmp = 0;
        //renorm
        for(int j = 0; j < 4; j++)
            for(int i = 0; i < 4; i++)
                for(int o = 0; o < 8; o++)
                {
                    tmp += f[a].h[j][i][o] * f[a].h[j][i][o];

                }

        for(int j = 0; j < 4; j++)
            for(int i = 0; i < 4; i++)
                for(int o = 0; o < 8; o++)
                {
                    f[a].h[j][i][o] *= 1.0 / sqrt(tmp);
                }


        for(int j = 0; j < 4; j++)
            for(int i = 0; i < 4; i++)
                for(int o = 0; o < 8; o++)
                {
                    f[a].h[j][i][o] = std::min(255.0, 512.0 * f[a].h[j][i][o]);
                }
    }
}

void myStitch::drawSIFTFeatures(std::vector<featurePoints> &f, cv::Mat &img)
{
    cv::Mat tmp = img.clone();
    for(int i = 0; i < f.size(); i++)
    {
        cv::circle(tmp, cv::Point(f[i].xHat[0], f[i].xHat[1]), 2, cv::Scalar(0, 0, 255), -1, 8);
        int line_l = cvRound(f[i].scale_subl * 10.0);
        cv::Point t(line_l * cos(f[i].orien) + f[i].xHat[0], line_l * (-1) * sin(f[i].orien) + f[i].xHat[1]);
        cv::line(tmp, cv::Point(f[i].xHat[0], f[i].xHat[1]), t, cv::Scalar(255, 0, 0), 2, 8);
    }

    img = tmp.clone();
}
