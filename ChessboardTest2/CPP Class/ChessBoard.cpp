//
//  ChessBoard.cpp
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/10.
//  Copyright © 2020 sungphill. All rights reserved.
//

#include "ChessBoard.h"

void ChessBoard::drawChessboard(cv::Mat& img){
    cv::Mat gray = img.clone();
    
    // If the image was alread grayscale, return it
    if (gray.channels() != 1){
        cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    }
    
    Size patternsize(9,6); //interior number of corners
    vector<Point2f> corners; //this will be filled by the detected corners
    
    bool patternfound = findChessboardCorners(gray, patternsize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
    + CALIB_CB_FAST_CHECK);
    
    if(patternfound){
//        cv::cornerSubPix(gray, corners, Size(11,11), Size(-1,-1), TermCriteria());
        drawChessboardCorners(img, patternsize, Mat(corners), patternfound);
    }
}

void ChessBoard::drawMarker(cv::Mat& inputImage){
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    
    cvtColor(inputImage, inputImage, cv::COLOR_BGRA2BGR);
    
    cv::Mat gray = inputImage.clone();
    if (gray.channels() != 1){
        cvtColor(gray, gray, cv::COLOR_BGR2GRAY);
    }

    cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::aruco::detectMarkers(gray, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
    
    if(markerIds.size() == 0)
        return;
    
    cv::aruco::drawDetectedMarkers(inputImage, markerCorners, markerIds);
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::aruco::estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoef, rvecs, tvecs);
    // draw axis for each marker
    
    cv::Vec3d objCenter = cv::Vec3d();
    cv::Vec3d objRot = cv::Vec3d();
    std::map<int, cv::Vec3d> idMapT, idMapR;
    
    for(int i=0; i<markerIds.size(); i++){
        int id = markerIds.at(i);
        if(id == 62
           || id == 203
           || id == 23
           || id == 40){
            objCenter += tvecs[i] / 4;
            idMapT.insert(std::make_pair(id, tvecs[i]));
            idMapR.insert(std::make_pair(id, rvecs[i]));
        }
        
        cv::aruco::drawAxis(inputImage, cameraMatrix, distCoef, rvecs[i], tvecs[i], 0.1);
    }
    
    if(idMapT.size() != 4)
        return;
    
    cv::Vec3d t1 = idMapT.find(23)->second;
    cv::Vec3d t2 = idMapT.find(62)->second;
    cv::Vec3d t3 = idMapT.find(203)->second;
    
    cv::Vec3d v1 = t3 - t1;
    cv::Vec3d v2 = t2 - t1;
    
    cv::Vec3d x, y, z;
    z = v2.cross(v1);
    z = makeUnitVec(z);

    cv::Mat R = calcAverRMat(idMapR);

    cv::Vec3d xAver, yAver, zAver;
    RMatToxyz(R, xAver, yAver, zAver);
    
    y = z.cross(xAver);
    y = makeUnitVec(y);
    
    x = y.cross(z);
    x = makeUnitVec(x);
    
    R = xyzToRMat(x, y, z);
    
    cv::Rodrigues(R, objRot);
    cv::aruco::drawAxis(inputImage, cameraMatrix, distCoef, objRot, objCenter, 0.2);
}

ChessBoard* ChessBoard::instance_ = nullptr;

ChessBoard& ChessBoard::instance() {
    if(instance_ == nullptr)
    {
        instance_ = new ChessBoard();
    }
    return *instance_;
}

void ChessBoard::setPath(string path){
    filepath = path;
    
    cv::FileStorage fsFrontRead(filepath, cv::FileStorage::READ);
    fsFrontRead["mtx"] >> cameraMatrix;
    fsFrontRead["dist"] >> distCoef;
    fsFrontRead.release();
}

void ChessBoard::setDataPath(string path){
    datasetPath = path;
}


cv::Mat ChessBoard::xyzToRMat(cv::Vec3d x, cv::Vec3d y, cv::Vec3d z){
    cv::Mat R = cv::Mat::zeros(3,3,CV_64FC1);
    
    R.at<double>(0,0) = x[0];
    R.at<double>(1,0) = x[1];
    R.at<double>(2,0) = x[2];
    
    R.at<double>(0,1) = y[0];
    R.at<double>(1,1) = y[1];
    R.at<double>(2,1) = y[2];
    
    R.at<double>(0,2) = z[0];
    R.at<double>(1,2) = z[1];
    R.at<double>(2,2) = z[2];
    
    return R.clone();
}

void ChessBoard::RMatToxyz(cv::Mat& src, cv::Vec3d& x, cv::Vec3d& y, cv::Vec3d& z){
    x[0] = src.at<double>(0,0);
    x[1] = src.at<double>(1,0);
    x[2] = src.at<double>(2,0);
    
    y[0] = src.at<double>(0,1);
    y[1] = src.at<double>(1,1);
    y[2] = src.at<double>(2,1);
    
    z[0] = src.at<double>(0,2);
    z[1] = src.at<double>(1,2);
    z[2] = src.at<double>(2,2);
}

cv::Mat ChessBoard::calcAverRMat(std::map<int, cv::Vec3d>& rMap){
    cv::Mat R = cv::Mat::zeros(3,3, CV_64FC1);
    
    for(auto r:rMap){
        cv::Mat Rt;
        
        cv::Rodrigues(r.second, Rt);
        R = R + Rt;
    }
    
    R = R / rMap.size();
    
    return R.clone();
}

cv::Vec3d ChessBoard::makeUnitVec(cv::Vec3d src){
    return src / sqrt(src[0] * src[0] + src[1] * src[1] + src[2] * src[2]);
}
