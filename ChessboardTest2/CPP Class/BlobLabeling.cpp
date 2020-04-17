//
//  BlobLabeling.cpp
//  ChessboardTest2
//
//  Created by sungphill on 2020/04/17.
//  Copyright © 2020 sungphill. All rights reserved.
//

#include "BlobLabeling.h"

#define _DEF_MAX_LABEL          100

BlobLabeling::BlobLabeling(void)
{
    m_nThreshold    = 0;
    m_vPoint = nullptr;
}

BlobLabeling::~BlobLabeling(void)
{
    m_recBlobs.clear();
}

void BlobLabeling::SetParam(cv::Mat& image, int nThreshold)
{
    m_Image = image.clone();
    m_nThreshold    = nThreshold;
    
    if( m_Image.channels() != 1 ){
        cv::cvtColor(m_Image, m_Image, cv::COLOR_BGR2GRAY);
        cv::threshold(m_Image, m_Image, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    }
}

void BlobLabeling::DoLabeling()
{
    Labeling(m_Image, m_nThreshold);
}

int BlobLabeling::Labeling(cv::Mat& image, int nThreshold)
{
    if( image.channels() != 1 )
        return 0;

    int nNumber;
    
    unsigned char* tmpBuf = new unsigned char [image.rows * image.cols];

    for(int j = 0; j < image.rows; ++j)
        for(int i = 0; i < image.cols; ++i)
            tmpBuf[j * image.cols + i] = image.at<unsigned char>(j,i);
    
    // 레이블링을 위한 포인트 초기화
    InitvPoint(image.cols, image.rows);

    // 레이블링
    nNumber = _Labeling(tmpBuf, image.cols, image.rows, nThreshold);

    // 포인트 메모리 해제
    DeletevPoint();

    if( nNumber != 0 )    DetectLabelingRegion(nNumber, tmpBuf, image.cols, image.rows);

    for(int j = 0; j < image.rows; ++j)
        for(int i = 0; i < image.cols; ++i)
            image.at<unsigned char>(j,i) = tmpBuf[j * image.cols + i];

    delete[] tmpBuf;
    return nNumber;
}

// m_vPoint 초기화 함수
void BlobLabeling::InitvPoint(int nWidth, int nHeight)
{
    int nX, nY;

    m_vPoint = new Visited [nWidth * nHeight];

    for(nY = 0; nY < nHeight; nY++)
    {
        for(nX = 0; nX < nWidth; nX++)
        {
            m_vPoint[nY * nWidth + nX].bVisitedFlag       = false;
            m_vPoint[nY * nWidth + nX].ptReturnPoint.x    = nX;
            m_vPoint[nY * nWidth + nX].ptReturnPoint.y    = nY;
        }
    }
}

void BlobLabeling::DeletevPoint()
{
    delete m_vPoint;
}

// Size가 nWidth이고 nHeight인 DataBuf에서
// nThreshold보다 작은 영역을 제외한 나머지를 blob으로 획득
int BlobLabeling::_Labeling(unsigned char *DataBuf, int nWidth, int nHeight, int nThreshold)
{
    int num = 0;
    int k, l;
    int StartX , StartY, EndX , EndY;
    
    // Find connected components
    for(int nY = 0; nY < nHeight; ++nY)
    {
        for(int nX = 0; nX < nWidth; ++nX)
        {
            if(DataBuf[nY * nWidth + nX] == 255)        // Is this a new component?, 255 == Object
            {
                num++;

                DataBuf[nY * nWidth + nX] = num;
                
                StartX = nX;
                StartY = nY;
                EndX = nX;
                EndY= nY;

                __NRFIndNeighbor(DataBuf, nWidth, nHeight, nX, nY, &StartX, &StartY, &EndX, &EndY);

                if(__Area(DataBuf, StartX, StartY, EndX, EndY, nWidth, num) < nThreshold)
                {
                     for(k = StartY; k <= EndY; k++)
                    {
                        for(l = StartX; l <= EndX; l++)
                        {
                            if(DataBuf[k * nWidth + l] == num)
                                DataBuf[k * nWidth + l] = 0;
                        }
                    }
                    --num;

                    if(num > 250)
                        return  0;
                }
            }
        }
    }

    return num;
}

// Blob labeling해서 얻어진 결과의 rec을 얻어냄
void BlobLabeling::DetectLabelingRegion(int nLabelNumber, unsigned char *DataBuf, int nWidth, int nHeight)
{
    int nX, nY;
    int nLabelIndex ;
    
    m_recBlobs.resize(nLabelNumber);

    bool bFirstFlag[255] = {false,};
    
    for(nY = 1; nY < nHeight - 1; nY++)
    {
        for(nX = 1; nX < nWidth - 1; nX++)
        {
            nLabelIndex = DataBuf[nY * nWidth + nX];

            if(nLabelIndex != 0)    // Is this a new component?, 255 == Object
            {
                if(bFirstFlag[nLabelIndex] == false)
                {
                    m_recBlobs[nLabelIndex-1].x            = nX;
                    m_recBlobs[nLabelIndex-1].y            = nY;
                    m_recBlobs[nLabelIndex-1].width        = 0;
                    m_recBlobs[nLabelIndex-1].height    = 0;
                
                    bFirstFlag[nLabelIndex] = true;
                }
                else
                {
                    int left    = m_recBlobs[nLabelIndex-1].x;
                    int right    = left + m_recBlobs[nLabelIndex-1].width;
                    int top        = m_recBlobs[nLabelIndex-1].y;
                    int bottom    = top + m_recBlobs[nLabelIndex-1].height;

                    if( left   >= nX )    left    = nX;
                    if( right  <= nX )    right    = nX;
                    if( top    >= nY )    top        = nY;
                    if( bottom <= nY )    bottom    = nY;

                    m_recBlobs[nLabelIndex-1].x            = left;
                    m_recBlobs[nLabelIndex-1].y            = top;
                    m_recBlobs[nLabelIndex-1].width        = right - left;
                    m_recBlobs[nLabelIndex-1].height    = bottom - top;

                }
            }
                
        }
    }
    
}

// Blob Labeling을 실제 행하는 function
int BlobLabeling::__NRFIndNeighbor(unsigned char *DataBuf, int nWidth, int nHeight, int nPosX, int nPosY, int *StartX, int *StartY, int *EndX, int *EndY )
{
    cv::Point CurrentPoint;
    
    CurrentPoint.x = nPosX;
    CurrentPoint.y = nPosY;

    m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x].bVisitedFlag    = true;
    m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x].ptReturnPoint.x = nPosX;
    m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x].ptReturnPoint.y = nPosY;
            
    while(1)
    {
        if( (CurrentPoint.x != 0) && (DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x - 1] == 255) )   // -X 방향
        {
            if( m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x - 1].bVisitedFlag == false )
            {
                DataBuf[CurrentPoint.y  * nWidth + CurrentPoint.x  - 1]                    = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];    // If so, mark it
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x - 1].bVisitedFlag    = true;
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x - 1].ptReturnPoint    = CurrentPoint;
                CurrentPoint.x--;
                
                if(CurrentPoint.x <= 0)
                    CurrentPoint.x = 0;

                if(*StartX >= CurrentPoint.x)
                    *StartX = CurrentPoint.x;

                continue;
            }
        }

        if( (CurrentPoint.x != nWidth - 1) && (DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x + 1] == 255) )   // -X 방향
        {
            if( m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x + 1].bVisitedFlag == false )
            {
                DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x + 1]                    = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];    // If so, mark it
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x + 1].bVisitedFlag    = true;
                m_vPoint[CurrentPoint.y * nWidth +  CurrentPoint.x + 1].ptReturnPoint    = CurrentPoint;
                CurrentPoint.x++;

                if(CurrentPoint.x >= nWidth - 1)
                    CurrentPoint.x = nWidth - 1;
                
                if(*EndX <= CurrentPoint.x)
                    *EndX = CurrentPoint.x;

                continue;
            }
        }

        if( (CurrentPoint.y != 0) && (DataBuf[(CurrentPoint.y - 1) * nWidth + CurrentPoint.x] == 255) )   // -X 방향
        {
            if( m_vPoint[(CurrentPoint.y - 1) * nWidth +  CurrentPoint.x].bVisitedFlag == false )
            {
                DataBuf[(CurrentPoint.y - 1) * nWidth + CurrentPoint.x]                    = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];    // If so, mark it
                m_vPoint[(CurrentPoint.y - 1) * nWidth +  CurrentPoint.x].bVisitedFlag    = true;
                m_vPoint[(CurrentPoint.y - 1) * nWidth +  CurrentPoint.x].ptReturnPoint = CurrentPoint;
                CurrentPoint.y--;

                if(CurrentPoint.y <= 0)
                    CurrentPoint.y = 0;

                if(*StartY >= CurrentPoint.y)
                    *StartY = CurrentPoint.y;

                continue;
            }
        }
    
        if( (CurrentPoint.y != nHeight - 1) && (DataBuf[(CurrentPoint.y + 1) * nWidth + CurrentPoint.x] == 255) )   // -X 방향
        {
            if( m_vPoint[(CurrentPoint.y + 1) * nWidth +  CurrentPoint.x].bVisitedFlag == false )
            {
                DataBuf[(CurrentPoint.y + 1) * nWidth + CurrentPoint.x]                    = DataBuf[CurrentPoint.y * nWidth + CurrentPoint.x];    // If so, mark it
                m_vPoint[(CurrentPoint.y + 1) * nWidth +  CurrentPoint.x].bVisitedFlag    = true;
                m_vPoint[(CurrentPoint.y + 1) * nWidth +  CurrentPoint.x].ptReturnPoint = CurrentPoint;
                CurrentPoint.y++;

                if(CurrentPoint.y >= nHeight - 1)
                    CurrentPoint.y = nHeight - 1;

                if(*EndY <= CurrentPoint.y)
                    *EndY = CurrentPoint.y;

                continue;
            }
        }
        
        if(        (CurrentPoint.x == m_vPoint[CurrentPoint.y * nWidth + CurrentPoint.x].ptReturnPoint.x)
            &&    (CurrentPoint.y == m_vPoint[CurrentPoint.y * nWidth + CurrentPoint.x].ptReturnPoint.y) )
        {
            break;
        }
        else
        {
            CurrentPoint = m_vPoint[CurrentPoint.y * nWidth + CurrentPoint.x].ptReturnPoint;
        }
    }

    return 0;
}

// 영역중 실제 blob의 칼라를 가진 영역의 크기를 획득
int BlobLabeling::__Area(unsigned char *DataBuf, int StartX, int StartY, int EndX, int EndY, int nWidth, int nLevel)
{
    int nArea = 0;

    for (int nY = StartY; nY < EndY; ++nY)
        for (int nX = StartX; nX < EndX; ++nX)
            if (DataBuf[nY * nWidth + nX] == nLevel)
                ++nArea;

    return nArea;
}


void BlobLabeling::DrawLabel(cv::Mat &img, cv::Scalar RGB){
    for(auto& rect:m_recBlobs){
        cv::rectangle(img, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), RGB);
    }
}


cv::Mat BlobLabeling::getInputImage(){
    return m_Image.clone();
}
