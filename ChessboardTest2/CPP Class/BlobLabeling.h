//
//  BlobLabeling.hpp
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/17.
//  Copyright © 2020 sungphill. All rights reserved.
//

#ifndef BlobLabeling_hpp
#define BlobLabeling_hpp

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

typedef struct
{
    bool    bVisitedFlag;
    cv::Point ptReturnPoint;
} Visited;

class  BlobLabeling
{
public:
    BlobLabeling(void);
public:
    ~BlobLabeling(void);

public:
    cv::Mat    m_Image;
    int            m_nThreshold;
    Visited*    m_vPoint;
    std::vector<cv::Rect>       m_recBlobs;


public:
    void SetParam(cv::Mat& image, int nThreshold = 1);

    // Run labeling
    void DoLabeling();

    void DrawLabel(cv::Mat &img, cv::Scalar RGB);
    
    cv::Mat getInputImage();
    // If labelpts is inside the bounding box, the mask corresponding to the bounding box is calculated.
    cv::Mat getMask(std::vector<cv::Point2f>& labelPts);

private:
    int     Labeling(cv::Mat& image, int nThreshold);

    void     InitvPoint(int nWidth, int nHeight);
    void     DeletevPoint();

    void     DetectLabelingRegion(int nLabelNumber, unsigned char *DataBuf, int nWidth, int nHeight);

    int        _Labeling(unsigned char *DataBuf, int nWidth, int nHeight, int nThreshold);
    
    int        __NRFIndNeighbor(unsigned char *DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY );
    int        __Area(unsigned char *DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel);
};

#endif /* BlobLabeling_hpp */
