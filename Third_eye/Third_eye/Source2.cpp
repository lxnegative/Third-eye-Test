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

#include <iomanip> //時間を取得するため
#include <sstream> //値を簡単に文字列にするため


#define CAMERANUMBER 0
#define DEBUGFLAG    1
#define MAXCONTOURS 2
#define MINCONTOURS  3
#define MAXAREA  4
#define MINAREA  5
#define THRESHOLD  6
#define DIFFERENCEPOINTLIMIT  7
#define INTERVAL   8
#define DISTANCELIMIT   9
#define JUDGNUM    10
#define COLOR_R    11
#define COLOR_G    12
#define COLOR_B    13
#define Radius    14
#define PROCESSZONE_X    15
#define PROCESSZONE_Y    16
#define PROCESSZONE_WIDTH    17
#define PROCESSZONE_HEIGHT    18
#define MAGNIFICATION    19
#define LINENUMLIMIT    20



struct PeoplePoint {

    cv::Point2f point;  //位置情報
    int deleteNum;      //これが一定値を超えたら消去

};


std::vector<std::string> CSVLoader(std::string fileName) {

    std::string str2 = fileName;

    std::vector<std::string> result;

    //ここからポイント情報の読み込み
    //ファイルの読み込み
    std::ifstream ifs(str2);
    if (ifs.fail()) {
        result.clear();
        return result;
    }

    //csvファイルを1行ずつ読み込む
    std::string str;
    while (getline(ifs, str)) {
        std::string::size_type index = str.find("#");  // "#"を検索
                                                       //"#"が入っていた行は飛ばす
        if (index != std::string::npos) {
            continue;
        }

        std::string token;
        std::istringstream stream(str);

        //1行のうち、文字列とコンマを分割する
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

cv::Mat* BackgroundSubtraction(cv::Mat nowImage, cv::Mat backgroundImage, int Threshold) {

    //閾値
    const char th = Threshold;

    cv::Mat diff, gry, dst;

    //背景差分法
    cv::absdiff(nowImage, backgroundImage, diff);//背景画像との差分を取得


    cv::cvtColor(diff, gry, CV_BGR2GRAY);//差分画像をグレイスケールに

    threshold(gry, dst, th, 255, cv::THRESH_BINARY);//二値化画像

    cv::erode(dst, dst, cv::Mat(), cv::Point(-1, -1), 1);

    //cv::dilate(dst, dst, cv::Mat(), cv::Point(-1, -1), 5);

    cv::Mat *resultImage = new cv::Mat();

    *resultImage = dst.clone();

    return resultImage;

}

std::vector<std::vector<cv::Point>> ContourExtraction(cv::Mat nowImage) {

    cv::Mat diff, gry, dst;

    /*
    cv::cvtColor(nowImage, gry, CV_BGR2GRAY);//差分画像をグレイスケールに

    threshold(gry, dst, 20, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);//二値化画像
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

    cv::cvtColor(nowImage, gry, CV_BGR2GRAY);//差分画像をグレイスケールに

    cv::Mat* canny_img = new cv::Mat();

    *canny_img = cv::Mat::zeros(cv::Size(nowImage.cols, nowImage.rows), CV_8U);

    cv::Canny(nowImage, *canny_img, 30.0, 200.0, 3);

    threshold(*canny_img, *canny_img, 20, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);//二値化画像

    return canny_img;

}

bool IsAddPeoplePoint(cv::Point2f addPoint, std::vector<PeoplePoint*>* peoplePoint, double DistanceLimit) {

    double distanceLimit = DistanceLimit;

    for (int i = 0; i < peoplePoint->size(); i++) {
        double distance = sqrt(((addPoint.x - (*peoplePoint)[i]->point.x) * (addPoint.x - (*peoplePoint)[i]->point.x)) + ((addPoint.y - (*peoplePoint)[i]->point.y) * (addPoint.y - (*peoplePoint)[i]->point.y)));
        if (distance < distanceLimit) {
            //そこに人がいるから消去されずに継続
            (*peoplePoint)[i]->deleteNum = 0;
            return false;
        }
    }

    return true;

}




int main() {

    std::vector<std::string> config = CSVLoader("config.conf");
    if (config.empty()) {
        printf("Cannot read file\n");
        return -1;
    }

    cv::VideoCapture cap;

    std::string::size_type index = config[CAMERANUMBER].find("*");  // "*"を検索

    //"*"が入っていた場合は動画を読み込む
    if (index != std::string::npos) {
        std::regex regex("[\\*]");
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

    while (!cap.isOpened()) {
        printf("Cannot read device or file\n");
        return -1;
    }


    double R = 0;
    double G = 0;
    double B = 0;

    cv::Mat frm, bg, diff, gry, dst;

    cv::Mat *dst1, *dst2, *edge;


    dst1 = new cv::Mat();

    int maxContours = 1000;
    int minContours = 40;


    int maxArea = 1000;
    int minArea = 50;

    int Threshold = 20;
    int differencePointLimit = 15000;



    //背景画像を取得
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

    int radius = 10;

    int debugFlag = 0;



    int lineNum = 0;
    int lineNumLimit = 0;

    std::vector<std::string> temp = CSVLoader("result.txt");
    lineNum = temp.size();

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

    std::regex regex("[^0-9\\-\\+]");
    std::string tempConfigString = "";


    int ProcessZone_X = 0;
    int ProcessZone_Y = 0;
    int ProcessZone_Width = 0;
    int ProcessZone_Height = 0;

    double magnification = 0.0; //画像の拡大率

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

        tempConfigString = std::regex_replace(config[COLOR_R], regex, "");
        R = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[COLOR_G], regex, "");
        G = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[COLOR_B], regex, "");
        B = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[DEBUGFLAG], regex, "");
        debugFlag = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[Radius], regex, "");
        radius = std::stoi(tempConfigString);

        tempConfigString = std::regex_replace(config[PROCESSZONE_X], regex, "");
        ProcessZone_X = std::stoi(tempConfigString);

        if (ProcessZone_X < 0 || ProcessZone_X > frm.cols) {
            ProcessZone_X = 0;
        }

        tempConfigString = std::regex_replace(config[PROCESSZONE_Y], regex, "");
        ProcessZone_Y = std::stoi(tempConfigString);

        if (ProcessZone_Y < 0 || ProcessZone_Y > frm.rows) {
            ProcessZone_Y = 0;
        }

        tempConfigString = std::regex_replace(config[PROCESSZONE_WIDTH], regex, "");
        ProcessZone_Width = std::stoi(tempConfigString);

        if (ProcessZone_Width < 0 || ProcessZone_X + ProcessZone_Width > frm.cols) {
            ProcessZone_Width = frm.cols - ProcessZone_X;
        }

        tempConfigString = std::regex_replace(config[PROCESSZONE_HEIGHT], regex, "");
        ProcessZone_Height = std::stoi(tempConfigString);

        if (ProcessZone_Height < 0 || ProcessZone_Y + ProcessZone_Height > frm.rows) {
            ProcessZone_Height = frm.rows - ProcessZone_Y;
        }

        tempConfigString = std::regex_replace(config[MAGNIFICATION], regex, "");
        magnification = std::stoi(tempConfigString);

        if (magnification < 0) {
            magnification = 100;
        }

        tempConfigString = std::regex_replace(config[LINENUMLIMIT], regex, "");
        lineNumLimit = std::stoi(tempConfigString);



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

        cnt++;
        if (cnt > interval) {

            cv::Rect cutArea(ProcessZone_X, ProcessZone_Y, ProcessZone_Width, ProcessZone_Height);

            cv::Mat frm_CutOut = cv::Mat(frm, cutArea).clone();
            cv::Mat bg_CutOut = cv::Mat(bg, cutArea).clone();

            cv::resize(frm_CutOut, frm_CutOut, cv::Size(), magnification / 100, magnification / 100, cv::INTER_LINEAR);
            cv::resize(bg_CutOut, bg_CutOut, cv::Size(), magnification / 100, magnification / 100, cv::INTER_LINEAR);


            //背景差分法で画像を抽出
            dst1 = BackgroundSubtraction(frm_CutOut, bg_CutOut, Threshold);

            differencePoint.clear();
            //前の画像と違う場所をポイント情報として格納
            for (int y = 0; y < dst1->rows; ++y) {
                for (int x = 0; x < dst1->cols; ++x) {
                    // 画像のチャネル数分だけループ。白黒の場合は1回、カラーの場合は3回　　　　　
                    for (int c = 0; c < dst1->channels(); ++c) {
                        if (static_cast<int>(dst1->data[y * dst1->step + x * dst1->elemSize() + c]) == 255) {
                            differencePoint.push_back(cv::Point(x, y));
                        }
                    }
                }
            }

            //あまりにも違う場所が多い（カメラ自体が動いた等）場合は処理をスキップ
            if (differencePoint.size() < differencePointLimit) {


                edge = new cv::Mat();

                *edge = cv::Mat::zeros(cv::Size(frm_CutOut.cols, frm.rows), CV_8U);

                //画像からエッジを抽出
                edge = EdgeDetection(frm_CutOut);

                cv::Mat tem = edge->clone();

                contours.clear();

                //エッジ抽出した画像から輪郭情報を抽出
                contours = ContourExtraction(tem);

                //delete(edge);

                //この時点での輪郭情報が多すぎるため輪郭を近似直線に変更（ついでに閉曲線化）
                for (size_t i = 0; i < contours.size(); i++)
                {
                    cv::Mat contour = cv::Mat(contours[i]);
                    std::vector<cv::Point> approx;
                    cv::approxPolyDP(contour, contours[i], 0.001 * cv::arcLength(contour, true), true);

                }

                //余りにも点が少ない、または点が多い輪郭を処理の対象外に
                for (int j = 0; j < contours.size(); j++) {

                    if (contours[j].size() < minContours || contours[j].size() > maxContours) {
                        contours.erase(contours.begin() + j);
                        //削除して一個ずれるため、jを1減らす
                        j--;
                    }

                }

                //余りにも輪郭の面積が少ない、または輪郭の面積が多い輪郭を処理の対象外に
                for (int j = 0; j < contours.size(); j++) {

                    double a = cv::contourArea(contours[j], false);
                    if (a > maxArea || a < minArea) {
                        contours.erase(contours.begin() + j);
                        //削除して一個ずれるため、jを1減らす
                        j--;
                    }

                }


                drawPoint.clear();

                std::vector<int> temp;

                temp.clear();

                //背景差分から出力された違うポイントから最も近い輪郭を抽出
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
                    //一番近い輪郭について過去の処理と重複していなければその輪郭を人として格納
                    int qq = contours.size() - 1;
                    if (!findNumVector(minNum, temp) && qq > minNum) {
                        temp.push_back(minNum);
                    }

                }

                for (int i = 0; i < temp.size(); i++) {
                    //一時保存変数から移す
                    drawPoint.push_back(contours[temp[i]]);

                }

                if (debugFlag != 0) {

                    //最終結果画像を初期化
                    dst2 = new cv::Mat();

                    *dst2 = cv::Mat::zeros(cv::Size(frm_CutOut.cols, frm_CutOut.rows), CV_8U);

                    cv::drawContours(*dst2, drawPoint, -1, cv::Scalar(255, 255, 255), CV_FILLED);


                    imshow("BackgroundSubtraction", *dst1);
                    imshow("edge", *edge);
                    imshow("test", *dst2);
                }
                //imshow("edge", *edge);

            }

            cv::Point2f* mc;

            //抽出された輪郭から重心を計算
            for (int i = 0; i < drawPoint.size(); i++) {
                cv::Moments mu = cv::moments(drawPoint[i]);
                mc = new cv::Point2f();
                *mc = cv::Point2f(mu.m10 / mu.m00, mu.m01 / mu.m00);

                //ここで切り取り後の画像の座標から切り取り前の座標に変換する
                mc->x = mc->x / (magnification / 100);
                mc->y = mc->y / (magnification / 100);

                mc->x = mc->x + ProcessZone_X;
                mc->y = mc->y + ProcessZone_Y;


                //重心が他の点と十分に離れている場合、そこに人がいると認識する
                if (IsAddPeoplePoint(*mc, peoplePoint, distanceLimit)) {
                    PeoplePoint* tempP = new PeoplePoint();
                    tempP->point = *mc;
                    tempP->deleteNum = 0;
                    peoplePoint->push_back(tempP);
                }
            }


            //deleteNumがJudgNumを超えたら消去
            for (int i = 0; i < peoplePoint->size(); i++) {
                (*peoplePoint)[i]->deleteNum++;
                if ((*peoplePoint)[i]->deleteNum > JudgNum) {
                    peoplePoint->erase(peoplePoint->begin() + i);
                    //削除して一個ずれるため、jを1減らす
                    i--;
                }
            }



            cnt = 0;

        }

        bg = frm.clone();

        //cv::imshow("bg", bg);



        //人がいると思われる場所に円を書く
        for (int i = 0; i < peoplePoint->size(); i++) {
            cv::circle(frm, (*peoplePoint)[i]->point, radius, cv::Scalar(B, G, R), -1);
            if (debugFlag != 0) {
                cv::rectangle(frm, cv::Point(ProcessZone_X, ProcessZone_Y), cv::Point(ProcessZone_X + ProcessZone_Width, ProcessZone_Y + ProcessZone_Height), cv::Scalar(B, G, R), 2, 4);
                if (debugFlag == 1) {
                    cv::circle(frm, (*peoplePoint)[i]->point, distanceLimit, cv::Scalar(B, G, R), 1);
                }
                else if (debugFlag == 2) {
                    cv::circle(frm, (*peoplePoint)[i]->point, distanceLimit, cv::Scalar(B, G, R), -1);
                }
            }
            else {
                cv::circle(frm, (*peoplePoint)[i]->point, distanceLimit, cv::Scalar(B, G, R), -1);
            }
        }




        imshow("frm", frm);



        char date[64];
        time_t t = time(NULL);
        strftime(date, sizeof(date), "%Y/%m/%d %a %H:%M:%S", localtime(&t));

        std::string dateStr = date;

        printf("%s %d人\n", dateStr.c_str(), peoplePoint->size());

        std::string filename = "result.txt";

        std::ofstream writing_file;
        writing_file.open(filename, std::ios::app);

        writing_file << dateStr << " " << peoplePoint->size() << "人" << std::endl;
        lineNum++;

        writing_file.close();

        if (lineNum > lineNumLimit) {

            remove("result_old.txt");

            rename("result.txt", "result_old.txt");

            lineNum = 0;

        }


        /*
        Judgcnt++;
        if (Judgcnt >= JudgNum) {
            printf("%d\n", peoplePoint->size());
            Judgcnt = 0;
            //peoplePoint.clear();
        }
        */

    }

    //フレーム画像を保存する．
    //cv::imwrite("img.png", frm);



    return 0;

}




