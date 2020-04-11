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
    
    Size patternsize(3,3); //interior number of corners
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
//    std::vector<cv::Vec3d> rvecs, tvecs;
//    cv::aruco::estimatePoseSingleMarkers(corners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);
//    // draw axis for each marker
//    for(int i=0; i<ids.size(); i++)
//        cv::aruco::drawAxis(inputImage, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);
}
