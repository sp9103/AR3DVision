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

+(bool) saveData:(UIImage *) image forKey:(NSString*) key;
+(void) writeData;
+(int) getDataCount;

@end

NS_ASSUME_NONNULL_END
