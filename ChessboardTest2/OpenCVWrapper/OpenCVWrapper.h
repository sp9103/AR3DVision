//
//  OpenCVWrapper.h
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/09.
//  Copyright © 2020 sungphill. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenCVWrapper : NSObject

+(UIImage *) makeGrayImage:(UIImage *) image;
+(UIImage *) makeChessboardImage:(UIImage *) image;
+(UIImage *) makeMarkerImage:(UIImage *) image;
+(void) initDescManager:(NSString*) path dataPath:(NSString*) datapath;

@end

NS_ASSUME_NONNULL_END
