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
    cv::Mat    m_Image;                // 레이블링을 위한 이미지
    int            m_nThreshold;            // 레이블링 스레스홀드 값
    Visited*    m_vPoint;                // 레이블링시 방문정보
    std::vector<cv::Rect>       m_recBlobs;                // 각 레이블 정보


public:
    // 레이블링 이미지 선택
    void SetParam(cv::Mat& image, int nThreshold = 1);

    // 레이블링(실행)
    void DoLabeling();

    // 레이블 그리기
    void DrawLabel(cv::Mat &img, cv::Scalar RGB);
    
    cv::Mat getInputImage();
    cv::Mat getMask(std::vector<cv::Point2f>& labelPts);

private:
    // 레이블링(동작)
    int     Labeling(cv::Mat& image, int nThreshold);

    // 포인트 초기화
    void     InitvPoint(int nWidth, int nHeight);
    void     DeletevPoint();

    // 레이블링 결과 얻기
    void     DetectLabelingRegion(int nLabelNumber, unsigned char *DataBuf, int nWidth, int nHeight);

    // 레이블링(실제 알고리즘)
    int        _Labeling(unsigned char *DataBuf, int nWidth, int nHeight, int nThreshold);
    
    // _Labling 내부 사용 함수
    int        __NRFIndNeighbor(unsigned char *DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY );
    int        __Area(unsigned char *DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel);
};

#endif /* BlobLabeling_hpp */
