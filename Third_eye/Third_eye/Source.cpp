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
#define MAX_CLUSTERS (200) /* number of cluster */
#define DEBUG_MODE (1)/*If u input 0,this program runs as runnning mode. 1 is debug mode*/

bool findNumVector(int findnum, std::vector<int> nums) {

    for (int num : nums) {
        if (findnum == num) {
            return true;
        }
    }

    return false;

}

cv::Mat* BackgroundSubtraction(cv::Mat nowImage, cv::Mat backgroundImage) {

    //θl
    const char th = 50;

    cv::Mat diff, gry, dst;

    //wi·ͺ@
    cv::absdiff(nowImage, backgroundImage, diff);//wiζΖΜ·ͺπζΎ


    cv::cvtColor(diff, gry, CV_BGR2GRAY);//·ͺζπOCXP[Ι

    threshold(gry, dst, th, 255, cv::THRESH_BINARY);//ρl»ζ

    //cv::erode(dst, dst, cv::Mat(), cv::Point(-1, -1), 1);

    //cv::dilate(dst, dst, cv::Mat(), cv::Point(-1, -1), 5);

    cv::Mat *resultImage = new cv::Mat();

    *resultImage = dst.clone();

    return resultImage;

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

cv::Mat* cvKMeansProcessing_AND_BackgroundSubtraction(cv::Mat nowImage, cv::Mat SubtractionImage) {

    //IplImage temp = SubtractionImage;
    //IplImage* SubImage = &temp;

    int i, j, size;
    IplImage *src_img = 0, *dst_img = 0;
    CvMat tmp_header;
    CvMat *clusters, *points, *tmp;
    CvMat *count = cvCreateMat(MAX_CLUSTERS, 1, CV_32SC1);
    CvMat *centers = cvCreateMat(MAX_CLUSTERS, 3, CV_32FC1);
    const char *imagename;
    //set background color
    int background_color[BACKGROND_COLOR_CHANNELS] = { 0,0,0 };

    IplImage temp;
    temp = nowImage;
    src_img = &temp;

    if (src_img == 0) {
        return nullptr;
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

    std::vector<int> FillClusters;
    FillClusters.clear();

    int clustersNum[MAX_CLUSTERS];

    for (int i = 0; i < MAX_CLUSTERS; i++) {
        clustersNum[i] = 0;
    }

    for (int y = 0; y < SubtractionImage.rows; ++y) {
        for (int x = 0; x < SubtractionImage.cols; ++x) {
            // ζΜ`lͺΎ―[vBΜκΝ1ρAJ[ΜκΝ3ρ@@@@@
            for (int c = 0; c < SubtractionImage.channels(); ++c) {
                if (static_cast<int>(SubtractionImage.data[y * SubtractionImage.step + x * SubtractionImage.elemSize() + c]) == 255) {
                    int idx = clusters->data.i[y * clusters->step + x];
                    clustersNum[idx]++;
                    if (!findNumVector(idx, FillClusters)) {
                        FillClusters.push_back(idx);
                    }
                }
                break;
            }
        }
    }


    for (i = 0; i < size; i++)
    {
        int idx = clusters->data.i[i];
        if (findNumVector(idx, FillClusters) && clustersNum[idx] < 50 && clustersNum[idx] >10)
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


    cv::Mat* tem = new cv::Mat();

    *tem = cv::cvarrToMat(dst_img, true);

    //cv::imshow("test", mat);

    //cvSaveImage(file, dst_img, 0);
    //printf("cluster %d image save completed.\n", j);


    //cvReleaseImage(&SubImage);
    //cvReleaseImage(&src_img);
    cvReleaseImage(&dst_img);
    cvReleaseMat(&clusters);
    cvReleaseMat(&points);
    cvReleaseMat(&count);

    return tem;

}




int main() {

    //cv::VideoCapture cap(1);

    cv::VideoCapture cap = cv::VideoCapture("768x576.avi");


    cv::Mat frm, bg, diff, gry;

    cv::Mat *dst1, *dst2;


    dst1 = new cv::Mat();



    //wiζπζΎ
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
        //GbWo
        double lowThreshold = 40.0;
        double highThreshold = 200.0;
        cv::Mat destination;
        cv::Canny(frm, edgeDst, lowThreshold, highThreshold);
        */

        dst1 = BackgroundSubtraction(frm, bg);

        dst2 = cvKMeansProcessing_AND_BackgroundSubtraction(frm, *dst1);

        //pointPolygonTestͺJMπ¬ιH
        //dst1 wi·ͺ@Μζ
        //dst2 Μζͺ΅½ζBΜζΚΕvectorΕΗ


        imshow("BackgroundSubtraction", *dst1);
        imshow("cvKMeansProcessing", *dst2);

        imshow("frm", frm);

        //cv::imshow("bg", bg);

        cnt++;
        if (cnt > interval) {
            frm.copyTo(bg);
            cnt = 0;
        }

    }

    /*
    cv::imwrite("0.png", *dst2[0]);
    cv::imwrite("1.png", *dst2[1]);
    cv::imwrite("2.png", *dst2[2]);
    cv::imwrite("3.png", *dst2[3]);
    cv::imwrite("4.png", *dst2[4]);
    */

    return 0;

}




