#include "estimate.h"
#include <QDebug>


estimate::estimate()
{
}


void estimate::process(std::vector<std::vector<featurePoints>> &f, std::vector<cv::Size> &pic_size, std::vector<std::vector<cv::DMatch>> &good_matches, std::vector<double> &focal)
{
    //Convert our features to OpenCV's image features.
    std::vector<cv::detail::ImageFeatures> ifs;
    toCVImageFeatures(f, pic_size, ifs);

    //Find focal length match infos
    std::vector<cv::detail::MatchesInfo> infos;
    cv::Ptr<cv::detail::FeaturesMatcher> matcher = new cv::detail::BestOf2NearestMatcher(false);
    (*matcher)(ifs, infos);

    //Estimate focals
    cv::detail::estimateFocal(ifs, infos, focal);

    //Find matching points
    good_matches.clear();
    good_matches.resize(infos.size());
    cv::FlannBasedMatcher fmatcher;
    for(int j = 0; j < ifs.size() - 1; j++)
    {
        cv::detail::ImageFeatures &m1 = ifs[j];
        cv::detail::ImageFeatures &m2 = ifs[j + 1];

        std::vector<cv::DMatch> matches;
        fmatcher.match(m1.descriptors, m2.descriptors, matches);

        double min_dist = 100.0;
        for( int i = 0; i < matches.size(); i++ )
        {
            double dist = matches[i].distance;
            if( dist < min_dist ) min_dist = dist;
        }

        //Eliminate outliers
        for( int i = 0; i < matches.size(); i++ )
        {
            if( matches[i].distance <= std::max(2 * min_dist, 0.2) )
            {
                good_matches[j].push_back(matches[i]);
            }
        }
    }
}

void estimate::toCVImageFeatures(std::vector<std::vector<featurePoints>> &f, std::vector<cv::Size> &pic_size, std::vector<cv::detail::ImageFeatures> &ifs)
{
    ifs.clear();
    ifs.resize(f.size());

    for(int i = 0; i < f.size(); i++)
    {
        cv::detail::ImageFeatures &imgf = ifs[i];
        toCVDescriptor(f[i], imgf.descriptors);

        imgf.img_idx = i;
        imgf.img_size = pic_size[i];
        imgf.keypoints.resize(f[i].size());

        for(int j = 0; j < f[i].size(); j++)
        {
            cv::KeyPoint &kpt = imgf.keypoints[j];
            featurePoints &ff = f[i][j];
            kpt.pt = cv::Point2f(ff.xHat[0], ff.xHat[1]);
            kpt.octave = ff.l + (ff.subl << 8) + (cvRound((ff.xHat[2] + 0.5) * 255) << 16);
            kpt.size = ff.scale_subl;
            kpt.response = ff.response;
            kpt.angle = (ff.orien * 180) / M_PI;
        }
    }
}

void estimate::toCVDescriptor(std::vector<featurePoints> &f, cv::Mat &d)
{
    int hz = f[0].h.size();
    int hy = f[0].h[0].size();
    int hx = f[0].h[0][0].size();

    cv::Mat descriptor((int)f.size(), (int)(hz * hy * hx), CV_32F);
    for(int j = 0; j < f.size(); j++)
    {
        for(int z = 0; z < hz; z++)
            for(int y = 0; y < hy; y++)
                for(int x = 0; x < hx; x++)
                {
                    descriptor.at<float>(j, z * hy * hx + y * hx + x) = f[j].h[z][y][x];
                }
    }

    d.release();
    d = descriptor.clone();
}

void estimate::drawMatches(cv::Mat &img1, std::vector<featurePoints> &f1, cv::Mat &img2, std::vector<featurePoints> &f2, std::vector<cv::DMatch> good_matches)
{

    int w = img1.cols + img2.cols;
    int h = img1.rows > img2.rows ? img1.rows : img2.rows;
    cv::Mat plate(h, w, CV_8UC3);

    for(int j = 0; j < plate.rows; j++)
    {
        for(int i = 0; i < img1.cols; i++)
        {
            for(int c = 0; c < 3; c++)
            {
                plate.at<cv::Vec3b>(j, i)[c] = img1.at<cv::Vec3b>(j, i)[c];
                plate.at<cv::Vec3b>(j, i + img1.cols)[c] = img2.at<cv::Vec3b>(j, i)[c];
            }
        }
    }

    for(int i = 0; i < good_matches.size(); i++)
    {
        cv::DMatch& m = good_matches[i];

        cv::Point one = cv::Point(f1[m.queryIdx].xHat[0], f1[m.queryIdx].xHat[1]);
        cv::Point two = cv::Point(f2[m.trainIdx].xHat[0] + img1.cols, f2[m.trainIdx].xHat[1]);
        cv::circle(plate, one, 2, cv::Scalar(0, 0, 255), -1, 8);
        cv::circle(plate, two, 2, cv::Scalar(0, 0, 255), -1, 8);
        cv::line(plate, one, two, cv::Scalar(rand() % 256, rand() % 256, rand() % 256), 2, 8);
    }
}

void estimate::alignMatches(cv::Mat &img1, std::vector<featurePoints> &f1, cv::Mat &img2, std::vector<featurePoints> &f2, std::vector<cv::DMatch> good_matches,
                 std::vector<int> &x,std::vector<int> &y,double FL1,double FL2)
{
    int cal_dx = 0;
    int cal_dy = 0;
    double avg_dx = 0 ;
    double avg_dy = 0;
    for(int i = 0; i < good_matches.size(); i++)
    {
        cv::DMatch& m = good_matches[i];

        if(i == good_matches.size()-1)
        {

            avg_dx = cal_dx/(good_matches.size()-1);
            x.push_back(int(avg_dx));
            avg_dy = cal_dy/(good_matches.size()-1);
            y.push_back(int(avg_dy));
            qDebug()<<avg_dx<<avg_dy;
        }
        else
        {
            int mid_x1 = img1.cols/2;
            int mid_x2 = img2.cols/2;
            int mid_y1 = img1.rows/2;
            int mid_y2 = img2.rows/2;

            double fL1 = FL1;
            double theta1 = atan((f1[m.queryIdx].xHat[0]-mid_x1)/fL1);
            double h1 = (f1[m.queryIdx].xHat[1]-mid_y1)/pow(pow((f1[m.queryIdx].xHat[0]-mid_x1),2)+pow(fL1,2),0.5);
            int x1 = fL1*theta1+mid_x1;
            int y1 = fL1*h1+mid_y1;


            double fL2 = FL2;
            double theta2 = atan((f2[m.trainIdx].xHat[0]-mid_x2)/fL2);
            double h2 = (f2[m.trainIdx].xHat[1]-mid_y2)/pow(pow((f2[m.trainIdx].xHat[0]-mid_x2),2)+pow(fL2,2),0.5);
            int x2 = fL2*theta2+mid_x2+img1.cols;
            int y2 = fL2*h2+mid_y2;
            int distance = x2 - x1;
            int distancey1 = img1.rows-y1+y2;
            //int distancey2 = img2.rows-y2+y1;
//            //qDebug()<<"img1.rows = "<< img1.rows<<" dy = "<<distancey;
//            int distance = f2[m.trainIdx].xHat[0]+img1.cols - f1[m.queryIdx].xHat[0];
//            int distancey = f2[m.trainIdx].xHat[1] +img1.rows - f1[m.queryIdx].xHat[1];

            cal_dx = cal_dx+distance;
//            if(f2[m.trainIdx].xHat[1]- f1[m.queryIdx].xHat[1] > 0)
//            {
//                cal_dy = cal_dy+distancey;
//            }
//            else
//            {
//                cal_dy = cal_dy+distancey +2*(img1.rows - distancey);
//            }
            //qDebug()<<"("<<x1<<","<<y1<<")  "<<"("<<x2<<","<<y2<<")  ";
            //qDebug()<<y1<<y2<<distancey1;

//            if(y1<y2)
//            {
//                cal_dy = cal_dy+distancey2;
//            }
//            else
//            {
                cal_dy = cal_dy+distancey1;
//            }
//            if(abs(distancey1-img1.rows) <abs(distancey2-img1.rows))
//            {

//                if(distancey1 > img1.rows)
//                {
//                    qDebug()<<distancey1 << distancey2<<" up";
//                    cal_dy = cal_dy+distancey2;
//                }
//                else
//                {
//                    qDebug()<<distancey1 << distancey2<<" down";
//                    cal_dy = cal_dy+distancey1;
//                }
//            }
//            else
//            {
//                qDebug()<<distancey1 << distancey2<<"up";
//                cal_dy = cal_dy+distancey2;
//            }
        }
    }
//    qDebug()<<"Other ";

//    int tcal_dx = 0;
//    int tcal_dy = 0;
//    double tavg_dx = 0 ;
//    double tavg_dy = 0;
//    for(int i = 0; i < good_matches.size(); i++)
//    {
//        cv::DMatch& m = good_matches[i];

//        if(i == good_matches.size()-1)
//        {
//            qDebug()<<"1";

//            tavg_dx = tcal_dx/(good_matches.size()-1);
//            //x.push_back(int(avg_dx));
//            //qDebug()<<cal_dx<<" "<<good_matches.size()<<" "<<avg_dx;

//            tavg_dy = tcal_dy/(good_matches.size()-1);

//            //qDebug()<<cal_dy<<" "<<good_matches.size()<<" "<<avg_dy;
//            //y.push_back(int(avg_dy));
//        }
//        else
//        {
//            //qDebug()<<"2";
//            int mid_x2 = img2.cols/2;
//            int mid_y2 = img2.rows/2;

//            double fL2 = FL2*112;
//            double theta2 = atan((f2[m.trainIdx].xHat[0]-mid_x2)/fL2);
//            double h2 = (f2[m.trainIdx].xHat[1]-mid_y2)/pow(pow((f2[m.trainIdx].xHat[0]-mid_x2),2)+pow(fL2,2),0.5);
//            double x2 = fL2*theta2+mid_x2;
//            double y2 = fL2*h2+mid_y2;


//            int mid_x1 = img1.cols/2;
//            int mid_y1 = img1.rows/2;

//            double fL1 = FL1*112;
//            double theta1 = atan((f1[m.queryIdx].xHat[0]-mid_x1)/fL1);
//            double h1 = (f1[m.queryIdx].xHat[1]-mid_y1)/pow(pow((f1[m.queryIdx].xHat[0]-mid_x1),2)+pow(fL1,2),0.5);
//            double x1 = fL1*theta1+mid_x1+img2.cols;
//            double y1 = fL1*h1+mid_y1;
//            int distance = x1 - x2;
//            int distancey = y1 + img2.rows-y2;
//            //qDebug()<<"img1.rows = "<< img1.rows<<" dy = "<<distancey<<" ===";
//            tcal_dx = tcal_dx+distance;
//            tcal_dy = tcal_dy+distancey;


//        }
//    }
//    x.push_back(avg_dx);
//    qDebug()<<" t "<<tavg_dy <<" n "<<avg_dy;


//    if(abs(tavg_dy - img1.rows) < abs(avg_dy - img1.rows))
//    {

//        y.push_back(tavg_dy);
//        qDebug()<<"img1.rows = "<< img1.rows<<" dy = "<<tavg_dy<<" up ";
//    }
//    else
//    {
//        y.push_back(avg_dy);
//        qDebug()<<"img1.rows = "<< img1.rows<<" dy = "<<avg_dy<<" down";
//    }


}
