#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/core/cvdef.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>

#include <regex>

#define CAMERANUMBER 0
#define MAXCONTOURS 1
#define MINCONTOURS  2
#define MAXAREA  3
#define MINAREA  4
#define THRESHOLD  5
#define DIFFERENCEPOINTLIMIT  6
#define INTERVAL   7
#define DISTANCELIMIT   8
#define JUDGNUM    9

struct PeoplePoint {

    cv::Point2f point;  //�ʒu���
    int deleteNum;      //���ꂪ���l�𒴂��������

};


std::vector<std::string> CSVLoader(std::string fileName) {

    std::string str2 = fileName;

    std::vector<std::string> result;

    //��������|�C���g���̓ǂݍ���
    //�t�@�C���̓ǂݍ���
    std::ifstream ifs(str2);
    if (ifs.fail()) {
        result.clear();
        return result;
    }

    //csv�t�@�C����1�s���ǂݍ���
    std::string str;
    while (getline(ifs, str)) {
        std::string::size_type index = str.find("#");  // "#"������
                                                       //"#"�������Ă����s�͔�΂�
        if (index != std::string::npos) {
            continue;
        }

        std::string token;
        std::istringstream stream(str);

        //1�s�̂����A������ƃR���}�𕪊�����
        while (getline(stream, token, ',')) {

            result.push_back(token);

        }

    }

    return result;

}

bool findNumVector(int findnum, std::vector<int> nums) {

    for (int num : nums) {
        if (findnum == num) {
            return true;
        }
    }

    return false;

}

cv::Mat* BackgroundSubtraction(cv::Mat nowImage, cv::Mat backgroundImage,int Threshold) {

    //臒l
    const char th = Threshold;

    cv::Mat diff, gry, dst;

    //�w�i�����@
    cv::absdiff(nowImage, backgroundImage, diff);//�w�i�摜�Ƃ̍������擾


    cv::cvtColor(diff, gry, CV_BGR2GRAY);//�����摜���O���C�X�P�[����

    threshold(gry, dst, th, 255, cv::THRESH_BINARY);//��l���摜

    cv::erode(dst, dst, cv::Mat(), cv::Point(-1, -1), 1);

    //cv::dilate(dst, dst, cv::Mat(), cv::Point(-1, -1), 5);

    cv::Mat *resultImage = new cv::Mat();

    *resultImage = dst.clone();

    return resultImage;

}

std::vector<std::vector<cv::Point>> ContourExtraction(cv::Mat nowImage) {

    cv::Mat diff, gry, dst;

    /*
    cv::cvtColor(nowImage, gry, CV_BGR2GRAY);//�����摜���O���C�X�P�[����

    threshold(gry, dst, 20, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);//��l���摜
    */

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(nowImage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    
    return contours;
    
    /*
    // iterate through all the top-level contours,
    // draw each connected component with its own random color

    cv::Scalar color(255, 255, 255);
    std::vector<std::vector<cv::Point>> count;

    count.push_back(contours[4]);
    cv::drawContours(*dst2, count, 0, color, 1, 8);
    */
}

cv::Mat* EdgeDetection(cv::Mat nowImage) {

    cv::Mat gry;

    cv::cvtColor(nowImage, gry, CV_BGR2GRAY);//�����摜���O���C�X�P�[����

    cv::Mat* canny_img = new cv::Mat();

    *canny_img = cv::Mat::zeros(cv::Size(nowImage.cols, nowImage.rows), CV_8U);

    cv::Canny(nowImage, *canny_img, 30.0, 200.0, 3);

    threshold(*canny_img, *canny_img, 20, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);//��l���摜

    return canny_img;

}

bool IsAddPeoplePoint(cv::Point2f addPoint, std::vector<PeoplePoint*>* peoplePoint,double DistanceLimit) {

    double distanceLimit = DistanceLimit;

    for (int i = 0; i < peoplePoint->size(); i++) {
        double distance = sqrt(((addPoint.x - (*peoplePoint)[i]->point.x) * (addPoint.x - (*peoplePoint)[i]->point.x)) + ((addPoint.y - (*peoplePoint)[i]->point.y) * (addPoint.y - (*peoplePoint)[i]->point.y)));
        if (distance < distanceLimit) {
            //�����ɐl�����邩��������ꂸ�Ɍp��
            (*peoplePoint)[i]->deleteNum = 0;
            return false;
        }
    }

    return true;

}




int main() {

    std::vector<std::string> config = CSVLoader("config.csv");
    if (config.empty()) {
        printf("Cannot read file\n");
        return -1;
    }

    cv::VideoCapture cap;

    std::string::size_type index = config[CAMERANUMBER].find("*");  // "*"������

    //"*"�������Ă����ꍇ�͓����ǂݍ���
    if (index != std::string::npos) {
        std::regex regex("[\*]");
        std::string tempConfigString = "";
        tempConfigString = std::regex_replace(config[CAMERANUMBER], regex, "");
        cap.open(tempConfigString);
    }
    else {
        try {
            cap.open(std::stoi(config[CAMERANUMBER]));
        }
        catch (std::invalid_argument e) {
            printf("Error. config file is wrong\n");
            return -1;
        }
        catch (std::out_of_range e) {
            printf("Error. config file is wrong\n");
            return -1;
        }
        
    }

    while(!cap.isOpened()) {
        printf("Cannot read device or file\n");
        return -1;
    }

    

    cv::Mat frm, bg, diff, gry,dst;

    cv::Mat *dst1, *dst2,*edge;


    dst1 = new cv::Mat();

    int maxContours = 1000;
    int minContours = 40;
    

    int maxArea = 1000;
    int minArea = 50;

    int Threshold = 20;
    int differencePointLimit = 15000;

    //�w�i�摜���擾
    cap >> frm;
    while (frm.empty()) {
        cap >> frm;
    }
    bg = frm.clone();
    //cv::imshow("bg", bg);

    int interval = 0;
    int cnt = 0;

    double distanceLimit = 150;

    int JudgNum = 30;
    int Judgcnt = 0;

    double threshold1, threshold2;
    CvRect roi;
    
    //dst2 = new cv::Mat(cv::Size(frm.cols, frm.rows), CV_8U);

    std::vector<std::vector<cv::Point>> contours;
    contours.clear();

    std::vector<cv::Point> differencePoint;
    differencePoint.clear();

    std::vector<std::vector<cv::Point>> drawPoint;
    drawPoint.clear();
    

    std::vector<PeoplePoint*>* peoplePoint = new std::vector<PeoplePoint*>();
    peoplePoint->clear();

    std::regex regex("[^0-9]");
    std::string tempConfigString = "";

    try {
        tempConfigString = std::regex_replace(config[MAXCONTOURS], regex, "");
        maxContours = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[MINCONTOURS], regex, "");
        minContours = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[MAXAREA], regex, "");
        maxArea = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[MINAREA], regex, "");
        minArea = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[THRESHOLD], regex, "");
        Threshold = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[DIFFERENCEPOINTLIMIT], regex, "");
        differencePointLimit = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[INTERVAL], regex, "");
        interval = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[DISTANCELIMIT], regex, "");
        distanceLimit = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[JUDGNUM], regex, "");
        JudgNum = std::stoi(tempConfigString);
    }
    catch (std::invalid_argument e) {
        printf("Error. config file is wrong\n");
        return -1;
    }
    catch (std::out_of_range e) {
        printf("Error. config file is wrong\n");
        return -1;
    }

    


    while (cv::waitKey(1) == -1) {
        cap >> frm;
        if (frm.empty())
            break;

        //�w�i�����@�ŉ摜�𒊏o
        dst1 = BackgroundSubtraction(frm, bg, Threshold);

        differencePoint.clear();
        //�O�̉摜�ƈႤ�ꏊ���|�C���g���Ƃ��Ċi�[
        for (int y = 0; y < dst1->rows; ++y) {
            for (int x = 0; x < dst1->cols; ++x) {
                // �摜�̃`���l�������������[�v�B�����̏ꍇ��1��A�J���[�̏ꍇ��3��@�@�@�@�@
                for (int c = 0; c < dst1->channels(); ++c) {
                    if (static_cast<int>(dst1->data[y * dst1->step + x * dst1->elemSize() + c]) == 255) {
                        differencePoint.push_back(cv::Point(x, y));
                    }
                }
            }
        }

        //���܂�ɂ��Ⴄ�ꏊ�������i�J�������̂����������j�ꍇ�͏������X�L�b�v
        if (differencePoint.size() < differencePointLimit) {


            edge = new cv::Mat();

            *edge = cv::Mat::zeros(cv::Size(frm.cols, frm.rows), CV_8U);

            //�摜����G�b�W�𒊏o
            edge = EdgeDetection(frm);

            cv::Mat tem = edge->clone();

            contours.clear();

            //�G�b�W���o�����摜����֊s���𒊏o
            contours = ContourExtraction(tem);

            delete(edge);

            //���̎��_�ł̗֊s��񂪑������邽�ߗ֊s���ߎ������ɕύX�i���łɕȐ����j
            for (size_t i = 0; i < contours.size(); i++)
            {
                cv::Mat contour = cv::Mat(contours[i]);
                std::vector<cv::Point> approx;
                cv::approxPolyDP(contour, contours[i], 0.001 * cv::arcLength(contour, true), true);
                
            }

            //�]��ɂ��_�����Ȃ��A�܂��͓_�������֊s�������̑ΏۊO��
            for (int j = 0; j < contours.size(); j++) {

                if (contours[j].size() < minContours || contours[j].size() > maxContours) {
                    contours.erase(contours.begin() + j);
                    //�폜���Ĉ����邽�߁Aj��1���炷
                    j--;
                }

            }

            //�]��ɂ��֊s�̖ʐς����Ȃ��A�܂��͗֊s�̖ʐς������֊s�������̑ΏۊO��
            for (int j = 0; j < contours.size(); j++) {

                double a = cv::contourArea(contours[j], false);
                if (a > maxArea || a < minArea) {
                    contours.erase(contours.begin() + j);
                    //�폜���Ĉ����邽�߁Aj��1���炷
                    j--;
                }

            }


            drawPoint.clear();

            std::vector<int> temp;

            temp.clear();

            //�w�i��������o�͂��ꂽ�Ⴄ�|�C���g����ł��߂��֊s�𒊏o
            for (int i = 0; i < differencePoint.size(); i++) {

                double min = 100000.0;
                int minNum = 0;



                for (int j = 0; j < contours.size(); j++) {
                    double d = cv::pointPolygonTest(contours[j], differencePoint[i], true);

                    if (min > abs(d)) {
                        min = abs(d);
                        minNum = j;
                    }

                }
                //��ԋ߂��֊s�ɂ��ĉߋ��̏����Əd�����Ă��Ȃ���΂��̗֊s��l�Ƃ��Ċi�[
                if (!findNumVector(minNum, temp)) {
                    temp.push_back(minNum);
                }

            }

            for (int i = 0; i < temp.size(); i++) {
                //�ꎞ�ۑ��ϐ�����ڂ�
                drawPoint.push_back(contours[temp[i]]);

            }

            //�ŏI���ʉ摜��������
            dst2 = new cv::Mat();

            *dst2 = cv::Mat::zeros(cv::Size(frm.cols, frm.rows), CV_8U);

            cv::drawContours(*dst2, drawPoint, -1, cv::Scalar(255, 255, 255), CV_FILLED);


            imshow("BackgroundSubtraction", *dst1);
            imshow("test", *dst2);

            //imshow("edge", *edge);

        }

        cv::Point2f* mc;
        //deleteNum��JudgNum�𒴂��������
        for (int i = 0; i < peoplePoint->size(); i++) {
            (*peoplePoint)[i]->deleteNum++;
            if ((*peoplePoint)[i]->deleteNum > JudgNum) {
                peoplePoint->erase(peoplePoint->begin() + i);
                //�폜���Ĉ����邽�߁Aj��1���炷
                i--;
            }
        }

        //���o���ꂽ�֊s����d�S���v�Z
        for (int i = 0; i < drawPoint.size(); i++) {
            cv::Moments mu = cv::moments(drawPoint[i]);
            mc = new cv::Point2f();
            *mc = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

            //�d�S�����̓_�Ə\���ɗ���Ă���ꍇ�A�����ɐl������ƔF������
            if (IsAddPeoplePoint(*mc, peoplePoint, distanceLimit)) {
                PeoplePoint* tempP = new PeoplePoint();
                tempP->point = *mc;
                tempP->deleteNum = 0;
                peoplePoint->push_back(tempP);
            }
        }

        

        //cv::imshow("bg", bg);

        cnt++;
        if (cnt > interval) {
            bg = frm.clone();
            cnt = 0;
        }

        for (int i = 0; i < peoplePoint->size(); i++) {
            cv::circle(frm, (*peoplePoint)[i]->point, 10, cv::Scalar(0, 0, 0), -1);
        }

        imshow("frm", frm);

        printf("%d\n", peoplePoint->size());

        /*
        Judgcnt++;
        if (Judgcnt >= JudgNum) {
            printf("%d\n", peoplePoint->size());
            Judgcnt = 0;
            //peoplePoint.clear();
        }
        */

    }

    return 0;

}




