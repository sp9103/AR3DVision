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

struct SCENE
{
    string filename;
    cv::Vec3d objCenter;
    cv::Vec3d objRot;
};

class ChessBoard
{
public:
    static ChessBoard& instance();
    
    void drawChessboard(cv::Mat& img);
    void drawMarker(cv::Mat& img);
    void drawCorner(cv::Mat& img);
    
    void setPath(string path);
    void setDataPath(string path);
   
    void writeData();
    bool saveData(cv::Mat& src, string key);
    void clearData();
    
    int getDataCount();
    
    void prepareImgForNet(cv::Mat& img);
    
    // Cover marker
    void drawCoverMarker(cv::Mat& img);
    
private:
    string filepath;
    string datasetPath;
    cv::Mat cameraMatrix;
    cv::Mat distCoef;
    
    std::vector<SCENE> savedData;
    
    ChessBoard();
    static ChessBoard* instance_;
    
    cv::Ptr<cv::aruco::DetectorParameters> parameters;
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    
    cv::Mat xyzToRMat(cv::Vec3d x, cv::Vec3d y, cv::Vec3d z);
    cv::Mat calcAverRMat(std::map<int, cv::Vec3d>& rMap);
    void RMatToxyz(cv::Mat& src, cv::Vec3d& x, cv::Vec3d& y, cv::Vec3d& z);
    cv::Vec3d makeUnitVec(cv::Vec3d src);
    
    void findCenterRT(const std::vector<cv::Vec3d>& tvecs,
                      const std::vector<cv::Vec3d>& rvecs,
                      const std::vector<int>& markerIds,
                      cv::Vec3d& objCenter,
                      cv::Vec3d& objRot);
    
    bool estimateMarker(cv::Mat& inputImage,
                        std::vector<int>& markerIds,
                        std::vector<std::vector<cv::Point2f>>& markerCorners,
                        std::vector<std::vector<cv::Point2f>>& rejectedCandidates,
                        std::vector<cv::Vec3d>& tvecs,
                        std::vector<cv::Vec3d>& rvecs);
    
    cv::Mat center_crop(cv::Mat& src);
};

#endif /* ChessBoard_h */
