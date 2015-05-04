#include "getF.h"
#include <fstream>
#include <iomanip>
#include <QDebug>

void GetF::getFocalL(QStringList &list,std::vector<double> &focalL)
{
    QString *name = new QString[list.count()];

    for(int i = 0;i<list.count();i++)
    {
        name[i] = list.at(i);
        name[i].replace(QString("\\"),QString("\\\\" ));
    }
    for(int i=0;i<list.count();i++)
    {
        QByteArray ba = name[i].toLatin1();
        const char *c_str2 = ba.data();
        //FILE *fp = fopen("D:\\Dropbox\\sen2\\ve\\test_exif_qt\\test_exif_qt\\0032.jpg", "rb");
        FILE *fp = fopen(c_str2, "rb");
        if (!fp) { printf("Can't open file.\n"); return; }
        fseek(fp, 0, SEEK_END);
        unsigned long fsize = ftell(fp);
        rewind(fp);
        unsigned char *buf = new unsigned char[fsize];
        if (fread(buf, 1, fsize, fp) != fsize) {
            printf("Can't read file.\n");
            return;
        }
        fclose(fp);

        // Parse exif
        EXIFInfo result;
        ParseEXIF(buf, fsize, result);
        if (result.focalLength)
        {
            printf("Lens focal length : %umm\n", result.focalLength);
            //FocalL.push_back(result.focalLength * 112);
            //focalL.push_back(result.focalLength * 112);
            FocalL.push_back(745);
            focalL.push_back(745);
        }
//        if (result.exposureTime)
//        {
//            //printf("Exposure          : 1/%gs\n", 1.0 / result.exposureTime);
//            //exposureTimes[i] = result.exposureTime;
//            exposureTimes.push_back(result.exposureTime);

//        }
        delete[] buf;
    }
//        for(int i=0;i<list.count();i++)
//        {
//            qDebug()<<name[i]<<" Focal Length = "<<FocalL[i];
//        }

    //Focal Length
}


void GetF::warping(std::vector<cv::Mat> &inputArrays,std::vector<double> FL2,std::vector<cv::Mat> &Output,std::vector<cv::Point> &upedge,std::vector<cv::Point> &downedge)
{
    //FL = FL;
    //std::vector<cv::Mat> Output;
    for(int j = 0; j < inputArrays.size(); j++)
    {
        cv::Mat image = inputArrays[j].clone();

        int mid_x = image.cols/2;
        int mid_y = image.rows/2;

        double FL = FL2[j]; //112


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
//                if(x<image.cols && x>=0 && y<image.rows && y>=0)
//                {
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

//                }
                //qDebug()<<" x "<<x<<" y "<<y;

            }
        }

        Output.push_back(temp);
        //cv::imshow(std::to_string(j),temp);
    }


}
