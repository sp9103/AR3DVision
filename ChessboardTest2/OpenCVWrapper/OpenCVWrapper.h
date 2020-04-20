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

+(void) initDescManager:(NSString*) path dataPath:(NSString*) datapath;

+(UIImage *) makeGrayImage:(UIImage *) image;
+(UIImage *) makeChessboardImage:(UIImage *) image;
+(UIImage *) makeMarkerImage:(UIImage *) image;
+(UIImage *) makeCoverMarkerImage:(UIImage *) image;
+(UIImage *) makeBlobLabelImage:(UIImage *) image;
+(UIImage *) makeCornerImage:(UIImage *) image;

+(UIImage *) refineImage:(UIImage *) image;

+(bool) saveData:(UIImage *) image forKey:(NSString*) key;
+(bool) saveSURFData:(UIImage *) image forKey:(NSString*) key;
+(void) clearData;
+(void) writeData;
+(int) getDataCount;

@end

NS_ASSUME_NONNULL_END
