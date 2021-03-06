//
//  OpenCVWrapper.m
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/09.
//  Copyright © 2020 sungphill. All rights reserved.
//

#import <opencv2/opencv.hpp>
#import <opencv2/imgcodecs/ios.h>

#import "OpenCVWrapper.h"
#import "ChessBoard.h"
#import "BlobLabeling.h"

@implementation OpenCVWrapper

+(UIImage *) makeGrayImage:(UIImage *) image {
    // Transform UIImage to cv::Mat
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    // If the image was alread grayscale, return it
    if (imageMat.channels() == 1)
        return image;
    
    // Transform the cv::mat color image to gray
    cv::Mat grayMat;
    cv::cvtColor(imageMat, grayMat, cv::COLOR_BGR2GRAY);
    
    // Transform grayMat to UIImage and return
    return MatToUIImage(grayMat);
}

+(UIImage *) makeChessboardImage:(UIImage *) image {
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    ChessBoard::instance().drawChessboard(dst);
    
    return MatToUIImage(dst);
}

+(UIImage *) makeMarkerImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    ChessBoard::instance().drawMarker(dst);
    
    return MatToUIImage(dst);
}

+(UIImage *) makeCoverMarkerImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    ChessBoard::instance().drawCoverMarker(dst);
    
    return MatToUIImage(dst);
}

+(UIImage *) makeBlobLabelImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    BlobLabeling blob;
    blob.SetParam(imageMat);
    blob.DoLabeling();
    blob.DrawLabel(dst, cv::Scalar(255, 0, 0, 255));
    
    return MatToUIImage(dst);
}

+(UIImage *) makeCornerImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    ChessBoard::instance().drawCorner(dst);
    
    return MatToUIImage(dst);
}

+(UIImage *) maskSURFmaskImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    ChessBoard::instance().drawMask(dst);
    
    return MatToUIImage(dst);
}

+(UIImage *) refineImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    ChessBoard::instance().prepareImgForNet(dst);
    cv::resize( dst, dst, cv::Size(224, 224), 0, 0, INTER_AREA );
    
    if (imageMat.channels() > 3)
        cv::cvtColor(dst, dst, cv::COLOR_BGRA2BGR);
    
    // normalize - TODO
    
    return MatToUIImage(dst);
}

+(void) initDescManager:(NSString*) path dataPath:(NSString*) datapath  descPath:(NSString*) descpath{
    std::string cppmtxpath = [path UTF8String];
    std::string cppdatapath = [datapath UTF8String];
    std::string cppdescpath = [descpath UTF8String];
    
    ChessBoard::instance().setPath(cppmtxpath);
    ChessBoard::instance().setDataPath(cppdatapath);
    ChessBoard::instance().setDescPath(cppdescpath);
}

+(bool) saveData:(UIImage *) image forKey:(NSString*) key {
    std::string cppKey = [key UTF8String];
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    return ChessBoard::instance().saveData(imageMat, cppKey);
}

+(bool) saveSURFData:(UIImage *) image forKey:(NSString*) key{
    std::string cppKey = [key UTF8String];
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    return ChessBoard::instance().saveSURFData(imageMat, cppKey);
}

+(void) writeData{
    ChessBoard::instance().writeData();
}

+(int) getDataCount{
    return ChessBoard::instance().getDataCount();
}

+(int) getSURFDataCount{
    return ChessBoard::instance().getSURFDataCount();
}

+(void) clearData{
    ChessBoard::instance().clearData();
}

+(MLMultiArray*) getSURFPos:(UIImage *) image{
    cv::Mat imageMat;
    std::vector<cv::Point2f> pts;
    NSError *error;
    MLMultiArray *inputArray = [[MLMultiArray alloc] initWithShape:@[@(1), @(14)] dataType:MLMultiArrayDataTypeFloat32
       error:&error];
    
    UIImageToMat(image, imageMat);
    
    ChessBoard::instance().matchingBaseDesc(imageMat, pts);
    
    for(int i = 0; i < pts.size(); ++i){
        cv::Point2f pt = pts.at(i);
        inputArray[i * 2 + 0] = @(pt.x);
        inputArray[i * 2 + 1] = @(pt.y);
    }
    
    return inputArray;
}

+(UIImage *) drawAxis:(UIImage *) image result:(MLMultiArray*) arr{
    cv::Mat imageMat;
    cv::Vec3d t, r;
    
    UIImageToMat(image, imageMat);
    
    cv::Mat dst = imageMat.clone();
    t[0] = [arr[0] doubleValue];
    t[1] = [arr[1] doubleValue];
    t[2] = [arr[2] doubleValue];
    
    r[0] = [arr[3] doubleValue];
    r[1] = [arr[4] doubleValue];
    r[2] = [arr[5] doubleValue];
    
    ChessBoard::instance().drawAxis(dst, t, r);
    
    return MatToUIImage(dst);
}

@end
