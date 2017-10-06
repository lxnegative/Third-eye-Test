#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/core/cvdef.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

#include <vector>

#define DEBUG_MODE_OFF (0)
#define DEBUG_MODE_ON (1)
#define BACKGROND_COLOR_CHANNELS (3)
#define BACKGROND_COLOR_RED (0)
#define BACKGROND_COLOR_GREEN (1)
#define BACKGROND_COLOR_BLUE (2)

#define RESULT_FILE_WORDS (200)
#define MAX_CLUSTERS (5) /* number of cluster */
#define DEBUG_MODE (1)/*If u input 0,this program runs as runnning mode. 1 is debug mode*/


cv::Mat BackgroundSubtraction(cv::Mat nowImage,cv::Mat backgroundImage) {

    //閾値
    const char th = 35;

    cv::Mat diff, gry, dst;

    //背景差分法
    cv::absdiff(nowImage, backgroundImage, diff);//背景画像との差分を取得


    cv::cvtColor(diff, gry, CV_BGR2GRAY);//差分画像をグレイスケールに

    threshold(gry, dst, th, 255, cv::THRESH_BINARY);//二値化画像

    //cv::erode(dst, dst, cv::Mat(), cv::Point(-1, -1), 1);

    cv::dilate(dst, dst, cv::Mat(), cv::Point(-1, -1), 5);

    return dst;

}

std::vector<cv::Mat*> cvKMeansProcessing(cv::Mat nowImage) {

    std::vector<cv::Mat*> result;
    result.clear();

    int i, j, size;
    IplImage *src_img = 0, *dst_img = 0;
    CvMat tmp_header;
    CvMat *clusters, *points, *tmp;
    CvMat *count = cvCreateMat(MAX_CLUSTERS, 1, CV_32SC1);
    CvMat *centers = cvCreateMat(MAX_CLUSTERS, 3, CV_32FC1);
    const char *imagename;
    //set background color
    int background_color[BACKGROND_COLOR_CHANNELS] = { 0,0,0 };

    IplImage temp = nowImage;

    src_img = &temp;
    if (src_img == 0) {
        result.clear();
        return result;
    }

    size = src_img->width * src_img->height;
    dst_img = cvCloneImage(src_img);
    clusters = cvCreateMat(size, 1, CV_32SC1);
    points = cvCreateMat(size, 1, CV_32FC3);


    for (i = 0; i < size; i++) {
        points->data.fl[i * 3 + 0] = (uchar)src_img->imageData[i * 3 + 0];
        points->data.fl[i * 3 + 1] = (uchar)src_img->imageData[i * 3 + 1];
        points->data.fl[i * 3 + 2] = (uchar)src_img->imageData[i * 3 + 2];
    }

    cvKMeans2(points, MAX_CLUSTERS, clusters,
        cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
        1, 0, 0, centers, 0);

    for (j = 0; j < MAX_CLUSTERS; j++)
    {
       
        for (i = 0; i < size; i++)
        {
            int idx = clusters->data.i[i];
            if (j == idx)
            {
                /*
                dst_img->imageData[i * 3 + 0] = (char)centers->data.fl[idx * 3 + 0];
                dst_img->imageData[i * 3 + 1] = (char)centers->data.fl[idx * 3 + 1];
                dst_img->imageData[i * 3 + 2] = (char)centers->data.fl[idx * 3 + 2];
                */
                
                dst_img->imageData[i * 3 + 0] = (char)255;
                dst_img->imageData[i * 3 + 1] = (char)255;
                dst_img->imageData[i * 3 + 2] = (char)255;
                
            }
            else
            {
                dst_img->imageData[i * 3 + 0] = background_color[BACKGROND_COLOR_BLUE];
                dst_img->imageData[i * 3 + 1] = background_color[BACKGROND_COLOR_GREEN];
                dst_img->imageData[i * 3 + 2] = background_color[BACKGROND_COLOR_RED];
            }
        }

        cv::Mat* temp = new cv::Mat();

        *temp = cv::cvarrToMat(dst_img, true);

        //cv::imshow("test", mat);

        result.push_back(temp);

        //cvSaveImage(file, dst_img, 0);
        //printf("cluster %d image save completed.\n", j);
    }

    //cvReleaseImage(&src_img);
    cvReleaseImage(&dst_img);
    cvReleaseMat(&clusters);
    cvReleaseMat(&points);
    cvReleaseMat(&count);

    return result;

}

std::vector<cv::Mat*> cvKMeansProcessing_AND_BackgroundSubtraction(cv::Mat nowImage, cv::Mat SubtractionImage) {

    IplImage temp = SubtractionImage;
    IplImage* SubImage = &temp;


    std::vector<cv::Mat*> result;
    result.clear();



    int i, j, size;
    IplImage *src_img = 0, *dst_img = 0;
    CvMat tmp_header;
    CvMat *clusters, *points, *tmp;
    CvMat *count = cvCreateMat(MAX_CLUSTERS, 1, CV_32SC1);
    CvMat *centers = cvCreateMat(MAX_CLUSTERS, 3, CV_32FC1);
    const char *imagename;
    //set background color
    int background_color[BACKGROND_COLOR_CHANNELS] = { 0,0,0 };


    temp = nowImage;
    src_img = &temp;

    if (src_img == 0) {
        result.clear();
        return result;
    }

    size = src_img->width * src_img->height;
    dst_img = cvCloneImage(src_img);
    clusters = cvCreateMat(size, 1, CV_32SC1);
    points = cvCreateMat(size, 1, CV_32FC3);


    for (i = 0; i < size; i++) {
        points->data.fl[i * 3 + 0] = (uchar)src_img->imageData[i * 3 + 0];
        points->data.fl[i * 3 + 1] = (uchar)src_img->imageData[i * 3 + 1];
        points->data.fl[i * 3 + 2] = (uchar)src_img->imageData[i * 3 + 2];
    }

    cvKMeans2(points, MAX_CLUSTERS, clusters,
        cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
        1, 0, 0, centers, 0);

    for (j = 0; j < MAX_CLUSTERS; j++)
    {

        for (i = 0; i < size; i++)
        {
            int idx = clusters->data.i[i];
            if (j == idx)
            {
                /*
                dst_img->imageData[i * 3 + 0] = (char)centers->data.fl[idx * 3 + 0];
                dst_img->imageData[i * 3 + 1] = (char)centers->data.fl[idx * 3 + 1];
                dst_img->imageData[i * 3 + 2] = (char)centers->data.fl[idx * 3 + 2];
                */

                dst_img->imageData[i * 3 + 0] = (char)255;
                dst_img->imageData[i * 3 + 1] = (char)255;
                dst_img->imageData[i * 3 + 2] = (char)255;

            }
            else
            {
                dst_img->imageData[i * 3 + 0] = background_color[BACKGROND_COLOR_BLUE];
                dst_img->imageData[i * 3 + 1] = background_color[BACKGROND_COLOR_GREEN];
                dst_img->imageData[i * 3 + 2] = background_color[BACKGROND_COLOR_RED];
            }
        }

        cv::Mat* temp = new cv::Mat();

        *temp = cv::cvarrToMat(dst_img, true);

        //cv::imshow("test", mat);

        result.push_back(temp);

        //cvSaveImage(file, dst_img, 0);
        //printf("cluster %d image save completed.\n", j);
    }

    //cvReleaseImage(&SubImage);
    //cvReleaseImage(&src_img);
    cvReleaseImage(&dst_img);
    cvReleaseMat(&clusters);
    cvReleaseMat(&points);
    cvReleaseMat(&count);

    return result;

}


int main() {

    //cv::VideoCapture cap(1);

    cv::VideoCapture cap = cv::VideoCapture("768x576.avi");


    cv::Mat frm, bg, dst1, diff, gry;

    std::vector<cv::Mat*> dst2;
    



    

    //背景画像を取得
    cap >> frm;
    frm.copyTo(bg);
    //cv::imshow("bg", bg);
    
    int interval = 0;
    int cnt = 0;

    int level = 4;
    double threshold1, threshold2;
    CvRect roi;


    while (cv::waitKey(1) == -1) {
        cap >> frm;
        if (frm.empty())
            break;

        /*
        //エッジ抽出
        double lowThreshold = 40.0;
        double highThreshold = 200.0;
        cv::Mat destination;
        cv::Canny(frm, edgeDst, lowThreshold, highThreshold);
        */

        dst1 = BackgroundSubtraction(frm, bg);

        dst2 = cvKMeansProcessing_AND_BackgroundSubtraction(frm, dst1);

        //pointPolygonTestがカギを握る？
        //dst1 背景差分法の画像
        //dst2 領域分割した画像。領域別でvectorで管理


        imshow("BackgroundSubtraction", dst1);
        imshow("cvKMeansProcessing", *dst2[0]);

        imshow("frm", frm);

        //cv::imshow("bg", bg);

        cnt++;
        if (cnt > interval) {
            frm.copyTo(bg);
            cnt = 0;
        }

    }

    cv::imwrite("0.png", *dst2[0]);
    cv::imwrite("1.png", *dst2[1]);
    cv::imwrite("2.png", *dst2[2]);
    cv::imwrite("3.png", *dst2[3]);
    cv::imwrite("4.png", *dst2[4]);

    return 0;

}




