#include <opencv2/opencv.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/core/cvdef.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <regex>

#include <iomanip> //���Ԃ��擾���邽��
#include <sstream> //�l���ȒP�ɕ�����ɂ��邽��

#define CAMERANUMBER 0

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

int main(int argc, char *argv[]) {

    std::vector<std::string> FirstFrame;

    for (int i = 1; i < argc; i++) {
        FirstFrame.push_back(std::string(argv[i]));
    }

    std::vector<std::string> config = CSVLoader("imageSave_config.cg");
    if (config.empty()) {
        printf("Cannot read file\n");
        return -1;
    }

    cv::VideoCapture cap;

    std::string::size_type index = config[CAMERANUMBER].find("*");  // "*"������

    int fps = -1;

    //"*"�������Ă����ꍇ�͓����ǂݍ���
    if (index != std::string::npos) {
        std::regex regex("[\\*]");
        std::string tempConfigString = "";
        tempConfigString = std::regex_replace(config[CAMERANUMBER], regex, "");
        cap.open(tempConfigString);
        fps = cap.get(CV_CAP_PROP_FPS);
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

    while (!cap.isOpened()) {
        printf("Cannot read device or file\n");
        return -1;
    }

    if (fps >= 0 && FirstFrame.size() > 0) {
        cap.set(CV_CAP_PROP_POS_FRAMES, std::stod(FirstFrame[0]) * fps);
    }


    //cv::VideoCapture cap(1);

    //cv::VideoCapture cap = cv::VideoCapture("768x576.avi");


    cv::Mat frm;



    //�摜���擾
    cap >> frm;

    char date[64];
    time_t t = time(NULL);
    strftime(date, sizeof(date), "%Y_%m_%d_%a_%H_%M_%S", localtime(&t));

    std::string dateStr = date;

    cv::imwrite("data/image/" + dateStr + ".png", frm);

    printf("data/image/%s.png", dateStr.c_str());
    
    /*
    cv::imwrite("0.png", *dst2[0]);
    cv::imwrite("1.png", *dst2[1]);
    cv::imwrite("2.png", *dst2[2]);
    cv::imwrite("3.png", *dst2[3]);
    cv::imwrite("4.png", *dst2[4]);
    */

    return 0;

}




