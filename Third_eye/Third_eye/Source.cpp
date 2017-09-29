#include <opencv2/opencv.hpp>


int main() {

    cv::VideoCapture cap(1);

    //cv::VideoCapture cap = cv::VideoCapture("768x576.avi");


    cv::Mat frm, bg, dst, diff, gry,edgeDst;

    //è‡’l
    const char th = 35;

    //”wŒi‰æ‘œ‚ðŽæ“¾
    cap >> frm;
    frm.copyTo(bg);
    //cv::imshow("bg", bg);
    
    int interval = 0;
    int cnt = 0;

    while (cv::waitKey(1) == -1) {
        cap >> frm;
        if (frm.empty())
            break;

        
        //ƒGƒbƒW’Šo
        double lowThreshold = 40.0;
        double highThreshold = 200.0;
        cv::Mat destination;
        cv::Canny(frm, edgeDst, lowThreshold, highThreshold);
        

        
        //”wŒi·•ª–@
        absdiff(frm, bg, diff);//”wŒi‰æ‘œ‚Æ‚Ì·•ª‚ðŽæ“¾

        cvtColor(diff, gry, CV_BGR2GRAY);//·•ª‰æ‘œ‚ðƒOƒŒƒCƒXƒP[ƒ‹‚É

        threshold(gry, dst, th, 255, cv::THRESH_BINARY);//“ñ’l‰»‰æ‘œ

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