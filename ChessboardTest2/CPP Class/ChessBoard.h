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
    void drawChessboard(cv::Mat& img);
    void drawMarker(cv::Mat& img);
    
    bool saveData(cv::Mat& src, string key);
    
    static ChessBoard& instance();
    void setPath(string path);
    void setDataPath(string path);
    
private:
    string filepath;
    string datasetPath;
    cv::Mat cameraMatrix;
    cv::Mat distCoef;
    
    ChessBoard() {}
    static ChessBoard* instance_;
    
    cv::Mat xyzToRMat(cv::Vec3d x, cv::Vec3d y, cv::Vec3d z);
    cv::Mat calcAverRMat(std::map<int, cv::Vec3d>& rMap);
    void RMatToxyz(cv::Mat& src, cv::Vec3d& x, cv::Vec3d& y, cv::Vec3d& z);
    cv::Vec3d makeUnitVec(cv::Vec3d src);
    
    void findCenterRT(const std::vector<cv::Vec3d>& tvecs,
                      const std::vector<cv::Vec3d>& rvecs,
                      const std::vector<int>& markerIds,
                      cv::Vec3d& objCenter,
                      cv::Vec3d& objRot);
};

#endif /* ChessBoard_h */
