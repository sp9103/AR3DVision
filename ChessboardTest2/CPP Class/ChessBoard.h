//
//  ChessBoard.h
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/10.
//  Copyright © 2020 sungphill. All rights reserved.
//

#ifndef ChessBoard_h
#define ChessBoard_h

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>

using namespace std;
using namespace cv;

class ChessBoard
{
public:
    ChessBoard() {}
    
    void drawChessboard(cv::Mat& img);
    void drawMarker(cv::Mat& img);
    
private:
    cv::Mat cameraMatrix;
    cv::Mat distCoef;
};

#endif /* ChessBoard_h */
