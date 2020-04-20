//
//  ChessBoard.cpp
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/10.
//  Copyright © 2020 sungphill. All rights reserved.
//
#include <fstream>

#include "ChessBoard.h"
#include "BlobLabeling.h"

ChessBoard::ChessBoard(){
    parameters = cv::aruco::DetectorParameters::create();
    dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    
    int minHessian = 400;
    detector = cv::xfeatures2d::SURF::create( minHessian );
}

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
    std::vector<cv::Vec3d> rvecs, tvecs;
    
    if(!estimateMarker(inputImage, markerIds, markerCorners, rejectedCandidates, tvecs, rvecs))
        return;
    
    cv::Vec3d objCenter = cv::Vec3d();
    cv::Vec3d objRot = cv::Vec3d();
    
    // draw axis for each marker
    cv::aruco::drawDetectedMarkers(inputImage, markerCorners, markerIds);
    for(int i=0; i<markerIds.size(); i++){
        cv::aruco::drawAxis(inputImage, cameraMatrix, distCoef, rvecs[i], tvecs[i], 0.1);
    }
    
    if(markerIds.size() < 4)
        return;
    
    findCenterRT(tvecs, rvecs, markerIds, objCenter, objRot);
    cv::aruco::drawAxis(inputImage, cameraMatrix, distCoef, objRot, objCenter, 0.2);
}

void ChessBoard::drawCorner(cv::Mat& img){
    Mat gray;
    Mat desc;
    vector<KeyPoint> kpts;
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::Vec3d objCenter = cv::Vec3d();
    cv::Vec3d objRot = cv::Vec3d();
    
    if(img.channels() != 1)
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
    
    BlobLabeling blob;
    blob.SetParam(img);
    blob.DoLabeling();
    
    if(!estimateMarker(img, markerIds, markerCorners, rejectedCandidates, tvecs, rvecs))
        return;
    
    if(markerIds.size() < 4)
        return;
    
    detectSURFObjOnly(markerCorners, gray, &blob, desc, kpts);

    drawKeypoints(img, kpts, img);
    
    // draw axis for each marker
    cv::aruco::drawDetectedMarkers(img, markerCorners, markerIds);
    for(int i=0; i<markerIds.size(); ++i){
        cv::aruco::drawAxis(img, cameraMatrix, distCoef, rvecs[i], tvecs[i], 0.1);
    }
    
    findCenterRT(tvecs, rvecs, markerIds, objCenter, objRot);
    cv::aruco::drawAxis(img, cameraMatrix, distCoef, objRot, objCenter, 0.2);
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

bool ChessBoard::saveData(cv::Mat& src, string key){
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::Vec3d objCenter, objRot;
    SCENE scene;
    
    if(!estimateMarker(src, markerIds, markerCorners, rejectedCandidates, tvecs, rvecs))
        return false;
    
    if(markerIds.size() < 4)
        return false;
    
    findCenterRT(tvecs, rvecs, markerIds, objCenter, objRot);
    
    scene.filename = key;
    scene.objCenter = objCenter;
    scene.objRot = objRot;
    
    savedData.push_back(scene);
    
    return true;
}

bool ChessBoard::saveSURFData(cv::Mat& src, string key){
    Mat gray;
    Mat desc;
    vector<KeyPoint> kpts;
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    std::vector<cv::Vec3d> rvecs, tvecs;
    cv::Vec3d objCenter = cv::Vec3d();
    cv::Vec3d objRot = cv::Vec3d();
    SCENE_SURF scene;
    
    if(src.channels() != 1)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    
    BlobLabeling blob;
    blob.SetParam(src);
    blob.DoLabeling();
    
    if(!estimateMarker(src, markerIds, markerCorners, rejectedCandidates, tvecs, rvecs))
        return false;
    
    if(markerIds.size() < 4)
        return false;
    
    findCenterRT(tvecs, rvecs, markerIds, objCenter, objRot);
    
    detectSURFObjOnly(markerCorners, gray, &blob, desc, kpts);
    
    scene.descname = key;
    scene.objCenter = objCenter;
    scene.objRot = objRot;
    scene.kpts = kpts;
    scene.desc = desc.clone();
    
    savedSURFData.push_back(scene);
    
    return true;
}

void ChessBoard::clearData(){
    savedData.clear();
}

void ChessBoard::findCenterRT(const std::vector<cv::Vec3d>& tvecs,
                  const std::vector<cv::Vec3d>& rvecs,
                  const std::vector<int>& markerIds,
                  cv::Vec3d& objCenter,
                  cv::Vec3d& objRot){
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
}

bool ChessBoard::estimateMarker(cv::Mat& inputImage,
                    std::vector<int>& markerIds,
                    std::vector<std::vector<cv::Point2f>>& markerCorners,
                    std::vector<std::vector<cv::Point2f>>& rejectedCandidates,
                    std::vector<cv::Vec3d>& tvecs,
                    std::vector<cv::Vec3d>& rvecs){
    cvtColor(inputImage, inputImage, cv::COLOR_BGRA2BGR);
    
    cv::Mat gray = inputImage.clone();
    if (gray.channels() != 1){
        cvtColor(gray, gray, cv::COLOR_BGR2GRAY);
    }

    cv::aruco::detectMarkers(gray, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
    
    if(markerIds.size() == 0)
        return false;
    
    cv::aruco::estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoef, rvecs, tvecs);
    
    return true;
}

void ChessBoard::writeData(){
    if(datasetPath.size() < 0)
        return;
    
    std::ofstream writeFile(datasetPath, ios::out);
    
    if(!writeFile.is_open())
        return;
    
    int size = (int)savedData.size();
    writeFile << size << endl;
    
    for(auto scene:savedData){
        writeFile << scene.filename << endl;
        writeFile << scene.objCenter << endl;
        writeFile << scene.objRot << endl;
    }
    
    writeFile.close();
}

int ChessBoard::getDataCount(){
    return (int)savedData.size();
}

int ChessBoard::getSURFDataCount(){
    return (int)savedSURFData.size();
}

void ChessBoard::prepareImgForNet(cv::Mat& img){
    img = center_crop(img);
}

cv::Mat ChessBoard::center_crop(cv::Mat& src){
    int target_crop_w = 480;
    int target_crop_h = 480;

    int h = src.rows;
    int w = src.cols;
    
    int c_x = w / 2;
    int c_y = h / 2;
    int crop_w1 = (int)(c_x - target_crop_w / 2);
    int crop_w2 = (int)(c_x + target_crop_w / 2);
    int crop_h1 = (int)(c_y - target_crop_h / 2);
    int crop_h2 = (int)(c_y + target_crop_h / 2);
    
    Rect rect(crop_w1, crop_h1, crop_w2-crop_w1, crop_h2-crop_h1);
    Mat subImage = src(rect);

    return subImage.clone();
}

void ChessBoard::drawCoverMarker(cv::Mat& img){
    std::vector<int> markerIds;
    std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
    std::vector<cv::Vec3d> rvecs, tvecs;
    std::vector<cv::Point2f> markerCenter;
    
    BlobLabeling blob;
    blob.SetParam(img);
    blob.DoLabeling();
    
    if(!estimateMarker(img, markerIds, markerCorners, rejectedCandidates, tvecs, rvecs))
        return;
    
    if(markerIds.size() < 4)
        return;
    
    for(auto& markerCorner:markerCorners){
        cv::Point2f center = cv::Point2f(0,0);
        
        for(auto& c:markerCorner){
            center += c;
        }
        center.x /= markerCorner.size();
        center.y /= markerCorner.size();
        
        markerCenter.push_back(center);
    }
    
    // calculate marker mask
    cv::Mat mask = blob.getMask(markerCenter);
    cv::inpaint(img, mask, img, 3, INPAINT_NS);
}

void ChessBoard::detectSURFObjOnly(const vector<vector<Point2f>>& markerCorners,
                                   const Mat& gray,
                                   BlobLabeling* blob,
                                   Mat& desc,
                                   vector<KeyPoint>& kpts){
    cv::Point2f center = cv::Point2f(0,0);
    std::vector<cv::Point2f> markerCenter;
    
    for(auto& markerCorner:markerCorners){
        cv::Point2f markCenter = cv::Point2f(0,0);
        
        for(auto& c:markerCorner){
            markCenter += c;
        }
        markCenter.x /= markerCorner.size();
        markCenter.y /= markerCorner.size();
        
        center += markCenter;
    }
    
    center.x /= markerCorners.size();
    center.y /= markerCorners.size();
    markerCenter.push_back(center);
    
    // calculate marker mask
    cv::Mat mask = blob->getMask(markerCenter);
    
    detector->detectAndCompute(gray, mask, kpts, desc);
}
