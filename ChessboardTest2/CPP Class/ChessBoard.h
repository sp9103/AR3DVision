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
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>

using namespace std;
using namespace cv;

class BlobLabeling;

struct SCENE
{
    string filename;
    Vec3d objCenter;
    Vec3d objRot;
};

struct SCENE_SURF
{
    string descname;
    Vec3d objCenter;
    Vec3d objRot;
    Mat desc;
    vector<KeyPoint> kpts;
};

class ChessBoard
{
public:
    static ChessBoard& instance();
    
    // Calibration chessboard detect
    void drawChessboard(cv::Mat& img);
    // aruco marker detect
    void drawMarker(cv::Mat& img);
    // Extract SURF
    void drawCorner(cv::Mat& img);
    // For Deploy training result
    void drawAxis(cv::Mat& img, cv::Vec3d objCenter, cv::Vec3d objRot);
    
    // set path for data read / write
    void setPath(string path);
    void setDataPath(string path);
    void setDescPath(string path);
   
    // data store / write method
    void writeData();
    bool saveData(cv::Mat& src, string key);
    bool saveSURFData(cv::Mat& src, string key);
    void clearData();
    
    int getDataCount();
    int getSURFDataCount();
    
    void prepareImgForNet(cv::Mat& img);
    
    // Image preprocessing method
    void drawCoverMarker(cv::Mat& img);         // remove aruco marker in image
    void drawMask(cv::Mat& img);                // Generate binary object mask for extract surf on object
    
    // Find position for SURF Net input
    void matchingBaseDesc(Mat& src, vector<Point2f>& dst);
    
private:
    string filepath;
    string datasetPath;
    string descDatasetPath;
    Mat cameraMatrix;
    Mat distCoef;
    Mat basedesc;
    
    vector<SCENE> savedData;
    vector<SCENE_SURF> savedSURFData;
    
    ChessBoard();
    static ChessBoard* instance_;
    
    cv::Ptr<cv::aruco::DetectorParameters> parameters;
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    cv::Ptr<cv::xfeatures2d::SURF> detector;
    
    Mat xyzToRMat(Vec3d x, Vec3d y, Vec3d z);
    Mat calcAverRMat(std::map<int, Vec3d>& rMap);
    void RMatToxyz(Mat& src, Vec3d& x, Vec3d& y, Vec3d& z);
    Vec3d makeUnitVec(Vec3d src);
    
    // Calculate object center position & orientation for record training data
    void findCenterRT(const std::vector<cv::Vec3d>& tvecs,
                      const std::vector<cv::Vec3d>& rvecs,
                      const std::vector<int>& markerIds,
                      cv::Vec3d& objCenter,
                      cv::Vec3d& objRot);
    
    // aruco marker detecting
    bool estimateMarker(cv::Mat& inputImage,
                        std::vector<int>& markerIds,
                        std::vector<std::vector<cv::Point2f>>& markerCorners,
                        std::vector<std::vector<cv::Point2f>>& rejectedCandidates,
                        std::vector<cv::Vec3d>& tvecs,
                        std::vector<cv::Vec3d>& rvecs);
    
    cv::Mat center_crop(cv::Mat& src);
    
    // detect surf on object
    void detectSURFObjOnly(const vector<vector<Point2f>>& markerCorners,
                           const Mat& gray,
                           BlobLabeling* blob,
                           Mat& desc,
                           vector<KeyPoint>& kpts,
                           Mat* pMask = nullptr);
};

#endif /* ChessBoard_h */
