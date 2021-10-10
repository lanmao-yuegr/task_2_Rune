#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    Mat frame;
    namedWindow("frame", WINDOW_AUTOSIZE);

    // 读取视频文件
    VideoCapture capture("能量机关_1.avi");
    if (!capture.isOpened())
    {
        printf("无法打开视频文件！");
        return -1;
    }

    //逐帧处理
    while (capture.read(frame))
    {
        Mat fpsImage;
        fpsImage = frame;

        //预处理
        cvtColor(fpsImage, fpsImage, CV_BGR2GRAY);

        float c = 10;
        for (int i = 0; i < fpsImage.rows; i++)
        {
            for (int j = 0; j < fpsImage.cols; j++)
            {
                Scalar tmp = fpsImage.at<uchar>(i, j);
                if (tmp[0] > c)
                {
                    tmp[0] = 255;
                }
                else
                {
                    tmp[0] = 0;
                }
                fpsImage.at<uchar>(i, j) = tmp[0];
            }
        }

        Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
        morphologyEx(fpsImage, fpsImage, 2, kernel, Point(-1, -1), 1);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(fpsImage, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));

        //选择轮廓：统计各轮廓是否有父轮廓，有则相应父轮廓的子轮廓数+1；最终选择仅有一个子轮廓的轮廓，该子轮廓即是所求轮廓；
        int contour[50] = {0};
        for (int i = 0; i < contours.size(); i++)
        {
            //部分点集过小的子轮廓选择忽略不计，并在之后进行筛除；
            if (hierarchy[i][3] != -1 && contours[i].size() > 20)
            {
                contour[hierarchy[i][3]]++;
            }
        }

        Mat fpsImage_tmp = fpsImage;
        int contour_required = -1;
        int break_ = -1;
        for (int i = 0; i < 50 && i < contours.size(); i++)
        {
            if (contour[i] == 1)
            {
                for (int j = i + 1; j < hierarchy[i][0]; j++)
                {
                    RotatedRect rect = minAreaRect(contours[j]);
                    //通过神符的形态特征进行轮廓的选择
                    if (contours[j].size() > 10 && contours[j].size() < 90 && ((rect.size.height / rect.size.width > 1 && rect.size.height / rect.size.width < 2.5) || (rect.size.width / rect.size.height > 1 && rect.size.width / rect.size.height < 2.5)))
                    {
                        contour_required = j;
                        break_ = 1;
                        break;
                    }
                }
            }
            if (break_ == 1)
            {
                break;
            }
        }

        //绘制矩形框
        fpsImage = frame;
        if (contour_required != -1)
        {
            RotatedRect rect_tmp = minAreaRect(contours[contour_required]);
            Scalar color_1(0, 0, 255);
            Scalar color_2(0, 255, 0);
            Point2f vertices[4];
            rect_tmp.points(vertices);
            for (int i = 0; i < 4; i++)
            {
                circle(fpsImage, vertices[i], 5, color_1, 2, 8, 0);
                line(fpsImage, vertices[i], vertices[(i + 1) % 4], color_2, 3, 8, 0);
            }
            circle(fpsImage, rect_tmp.center, 5, color_1, 2, 8, 0);
        }

        imshow("frame", fpsImage);
        waitKey(40);
    }
}