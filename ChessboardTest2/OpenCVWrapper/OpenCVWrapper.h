//
//  OpenCVWrapper.h
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/09.
//  Copyright © 2020 sungphill. All rights reserved.
// C++ Wrapper

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <CoreML/CoreML.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenCVWrapper : NSObject

// path setting to chessboard singlton
+(void) initDescManager:(NSString*) path dataPath:(NSString*) datapath descPath:(NSString*) descpath;

+(UIImage *) makeGrayImage:(UIImage *) image;
+(UIImage *) makeChessboardImage:(UIImage *) image;
+(UIImage *) makeMarkerImage:(UIImage *) image;
+(UIImage *) makeCoverMarkerImage:(UIImage *) image;
+(UIImage *) makeBlobLabelImage:(UIImage *) image;
+(UIImage *) makeCornerImage:(UIImage *) image;
+(UIImage *) maskSURFmaskImage:(UIImage *) image;
+(UIImage *) drawAxis:(UIImage *) image result:(MLMultiArray*) arr;

+(UIImage *) refineImage:(UIImage *) image;

+(MLMultiArray*) getSURFPos:(UIImage *) image;

+(bool) saveData:(UIImage *) image forKey:(NSString*) key;
+(bool) saveSURFData:(UIImage *) image forKey:(NSString*) key;
+(void) clearData;
+(void) writeData;
+(int) getDataCount;
+(int) getSURFDataCount;

@end

NS_ASSUME_NONNULL_END
