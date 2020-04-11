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
    
    ChessBoard cb;
    cv::Mat dst = imageMat.clone();
    cb.drawChessboard(dst);
    
    return MatToUIImage(dst);
}

+(UIImage *) makeMarkerImage:(UIImage *) image{
    cv::Mat imageMat;
    
    UIImageToMat(image, imageMat);
    
    ChessBoard cb;
    cv::Mat dst = imageMat.clone();
    cb.drawMarker(dst);
    
    return MatToUIImage(dst);
}

@end
