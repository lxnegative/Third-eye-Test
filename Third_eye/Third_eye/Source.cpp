#include <opencv2/opencv.hpp>


int main() {

    cv::VideoCapture cap(1);

    //cv::VideoCapture cap = cv::VideoCapture("768x576.avi");


    cv::Mat frm, bg, dst, diff, gry,edgeDst;

    //臒l
    const char th = 35;

    //�w�i�摜���擾
    cap >> frm;
    frm.copyTo(bg);
    //cv::imshow("bg", bg);
    
    int interval = 0;
    int cnt = 0;

    while (cv::waitKey(1) == -1) {
        cap >> frm;
        if (frm.empty())
            break;

        
        //�G�b�W���o
        double lowThreshold = 40.0;
        double highThreshold = 200.0;
        cv::Mat destination;
        cv::Canny(frm, edgeDst, lowThreshold, highThreshold);
        

        
        //�w�i�����@
        absdiff(frm, bg, diff);//�w�i�摜�Ƃ̍������擾

        cvtColor(diff, gry, CV_BGR2GRAY);//�����摜���O���C�X�P�[����

        threshold(gry, dst, th, 255, cv::THRESH_BINARY);//��l���摜

        //cv::erode(dst, dst, cv::Mat(), cv::Point(-1, -1), 1);

        cv::dilate(dst, dst, cv::Mat(), cv::Point(-1, -1), 5);
        
        //cv::
        
        add(dst, edgeDst, dst);

        imshow("dst", dst);

        imshow("frm", frm);

        //cv::imshow("bg", bg);

        cnt++;
        if (cnt > interval) {
            frm.copyTo(bg);            
            cnt = 0;
        }

    }

    return 0;

}