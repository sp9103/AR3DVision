import os
import random
import cv2
from time import sleep
from concurrent.futures import ThreadPoolExecutor
import numpy as np
from abc import *
import queue
from datetime import datetime
import json

# def ImageLoadProcess(id, procCount, imgCnt, q, buffersize, ImgList, batchsize, bSuffle):
def ImageLoadProcess(args):
    id = args[0]
    procCount = args[1]
    imgCnt = args[2]
    q = args[3]
    buffersize = args[4]
    ImgList = args[5]
    batchsize = args[6]
    bSuffle = args[7]

    cur_count = 0
    imgcount = imgCnt
    ImgQueue = q
    buffersize = buffersize / batchsize
    ImgList = ImgList

    print('module name:', __name__)
    print('parent process:', os.getppid())
    print('process id:', os.getpid())

    if len(ImgList) == 0:
        # 읽어들인 이미지가 없음
        return 0

    loadimglist = []
    loadlabellist = []

    while True:
        while ImgQueue.qsize() > buffersize:
            sleep(1)

        idx = (cur_count * procCount + id)
        cur_count += 1

        if idx >= imgcount:
            idx -= imgcount
            cur_count = 0

        if idx == 0 and bSuffle is True:
            random.shuffle(ImgList)

        path = ImgList[idx]
        img = cv2.imread(path['rgb'])
        if img is None:
            continue

        img = img[...,::-1]   # bgr to rgb

        img = center_crop(img)
        img = cv2.resize(img, dsize=(224, 224), interpolation=cv2.INTER_AREA)
        img = ImgNormalize(img, 255.)

        label = np.concatenate((path['center'], path['rotation']), axis=None)
        loadimglist.append(img)
        loadlabellist.append(label)

        if len(loadimglist) >= batchsize:
            imgarray = np.array(loadimglist)
            labelarray = np.array(loadlabellist)

            ImgQueue.put([id, imgarray, labelarray])
            loadimglist = []
            loadlabellist = []

class MemoryLoadBass:
    __metaclass__ = ABCMeta

    def __init__(self, batchsize, numprocess):
        super().__init__()
        self._DataQueue = queue.Queue()
        self._procList = []

        self._batchsize = batchsize
        self._numProcess = numprocess
        self.pool = None

    def __del__(self):
        self.KillAllProcess()

    @abstractmethod
    def _ReadAllDataPath(self):
        raise NotImplementedError()

    @abstractmethod
    def StartLoadData(self):
        raise NotImplementedError()

    def RunProcess(self, func, proc_args):
        self.pool = ThreadPoolExecutor(self._numProcess)
        for i in range(self._numProcess):
            partial_args = (i,) + proc_args
            proc = self.pool.submit(func, partial_args)
            self._procList.append(proc)

    def getLoadedData(self):
        while True:
            Qsize = self.getCollectedSize()
            if Qsize > 0:
                break
            sleep(1)

        return self._DataQueue.get()

    def getCollectedSize(self):
        return self._DataQueue.qsize()

    def KillAllProcess(self):
        self.pool.shutdown(wait=True)

        while not self._DataQueue.empty():
            self._DataQueue.get()

class ImageCollector(MemoryLoadBass):
    def __init__(self, rootpath, numProcess, buffersize, batchsize, bSuffle):
        super().__init__(batchsize, numProcess)

        self._rootpath = rootpath
        self._buffersize = buffersize
        self._bSuffle = bSuffle

        ImgList, self._imgcount = self._ReadAllImgPath()
        self._ImgList = ImgList

        assert not (self._imgcount == 0), 'Load data count is ZERO.'

    def _ReadAllImgPath(self):
        img_list = []

        for (path, dir, files) in os.walk(self._rootpath):
            for filename in files:
                ext = os.path.splitext(filename)[-1]
                
                if ext == '.txt':
                    full_filename = os.path.join(path, filename)
                    
                    f = open(full_filename, 'r')
                    totalCnt = f.readline()
                    
                    for i in range(int(totalCnt)):
                        namepair = dict()
                        
                        ImgPath = f.readline()
                        objCenter = f.readline()
                        objRot = f.readline()
                        
                        ImgPath = os.path.join(path, ImgPath[:-1])
                        objCenter = np.fromstring(objCenter[1:-2], sep=', ')
                        objRot = np.fromstring(objRot[1:-2], sep=', ')
                        
                        namepair['rgb'] = ImgPath
                        namepair['center'] = objCenter
                        namepair['rotation'] = objRot
                        
                        img_list.append(namepair)
                    
                    f.close()

        return img_list, len(img_list)

    def StartLoadData(self):
        if self._bSuffle:
            random.shuffle(self._ImgList)
        self.RunProcess(ImageLoadProcess, proc_args=(self._numProcess,
                                                     self._imgcount, self._DataQueue,
                                                     self._buffersize, self._ImgList,
                                                     self._batchsize,
                                                     self._bSuffle))

    def getDataCnt(self):
        return self._imgcount

def center_crop(img):
    target_crop_w = 480
    target_crop_h = 480

    h, w, c = img.shape
    c_x = w / 2
    c_y = h / 2
    crop_w = [int(c_x - target_crop_w / 2), int(c_x + target_crop_w / 2)]
    crop_h = [int(c_y - target_crop_h / 2), int(c_y + target_crop_h / 2)]

    img = img[crop_h[0]:crop_h[1], crop_w[0]:crop_w[1], :]

    return img

def ImgNormalize(img, scale):
    dst = img.copy()
    dst = dst.astype('float') / (scale / 2.0)
    dst -= 1
    return dst

def ImgInverNormalize(img, scale):
    dst = img.copy()
    dst += 1
    dst *= (scale / 2.0)
    return dst
