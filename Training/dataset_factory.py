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

# def ImageLoadProcess(id, procCount, imgCnt, q, buffersize, ImgList, batchsize):
def ImageLoadProcess(args):
    id = args[0]
    procCount = args[1]
    imgCnt = args[2]
    q = args[3]
    buffersize = args[4]
    ImgList = args[5]
    batchsize = args[6]

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
    loadseglist = []

    while True:
        while ImgQueue.qsize() > buffersize:
            sleep(1)

        idx = (cur_count * procCount + id)
        cur_count += 1

        if idx >= imgcount:
            idx -= imgcount
            cur_count = 0

        if idx == 0:
            random.shuffle(ImgList)

        path = ImgList[idx]
        img = cv2.imread(path['rgb'])
        if img is None:
            continue

        img = img[...,::-1]   # bgr to rgb

        seg = None
        if 'seg' in path and path['seg']:
            seg = np.load(path['seg'])

        img, seg = random_crop(img, seg)
        img = ImgNormalize(img, 255.)

        loadimglist.append(img)
        if seg is not None:
            seg = segmentLabeling(seg)
            seg = ImgNormalize(seg, 3.)  # -1 에서  1사이의 값을 가지도록
            loadseglist.append(seg)

        if len(loadimglist) >= batchsize:
            imgarray = np.array(loadimglist)
            segarray = None
            if len(loadseglist) >= batchsize:
                segarray = np.array(loadseglist)
                seg_shape = segarray.shape
                segarray = np.reshape(segarray, (seg_shape[0], seg_shape[1], seg_shape[2], 1))

            ImgQueue.put([id, imgarray, segarray])
            loadimglist = []
            loadseglist = []

    # print('process end :', os.getpid())
    # return 1

class MemoryLoadBass:
    __metaclass__ = ABCMeta

    def __init__(self, batchsize, numprocess):
        super().__init__()
        self._DataQueue = queue.Queue()
        self._procList = []

        self._batchsize = batchsize
        self._numProcess = numprocess

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
    def __init__(self, rootpath, numProcess, buffersize, batchsize):
        super().__init__(batchsize, numProcess)

        self._rootpath = rootpath
        self._buffersize = buffersize

        ImgList, self._imgcount = self._ReadAllImgPath()
        self._ImgList = ImgList

        assert not (self._imgcount == 0), 'Load data count is ZERO.'

    def _ReadAllImgPath(self):
        img_list = []

        for (path, dir, files) in os.walk(self._rootpath):
            for filename in files:
                namepair = dict()
                ext = os.path.splitext(filename)[-1]
                
                if ext == '.txt':
                    full_filename = os.path.join(path, filename)
                    
                    f = open(full_filename, 'r')
                    totalCnt = f.readline()
                    
                    for i in range(int(totalCnt)):
                        ImgPath = f.readline()
                        objCenter = f.readline()
                        objRot = f.readline()
                        
                        objCenter = np.fromstring(objCenter[1:-2], sep=', ')
                        objRot = np.fromstring(objRot[1:-2], sep=', ')
                        
                        namepair['rgb'] = ImgPath
                        namepair['center'] = objCenter
                        namepair['rotation'] = objRot
                        
                        img_list.append(namepair)
                    
                    f.close()

        return img_list, len(img_list)

    def StartLoadData(self):
        random.shuffle(self._ImgList)
        self.RunProcess(ImageLoadProcess, proc_args=(self._numProcess,
                                                     self._imgcount, self._DataQueue,
                                                     self._buffersize, self._ImgList,
                                                     self._batchsize))

    def getDataCnt(self):
        return self._imgcount


def random_crop(img, seg):
    target_crop_w = 472
    target_crop_h = 472

    h, w, c = img.shape
    c_x = w / 2 + 30
    c_y = h / 2
    crop_cx = random.randint(0, 21) - 10
    crop_cy = random.randint(0, 7) - 3
    crop_w = [int(c_x + crop_cx - target_crop_w / 2), int(c_x + crop_cx + target_crop_w / 2)]
    crop_h = [int(c_y + crop_cy - target_crop_h / 2), int(c_y + crop_cy + target_crop_h / 2)]

    img = img[crop_h[0]:crop_h[1], crop_w[0]:crop_w[1], :]
    if seg is not None:
        seg = seg[crop_h[0]:crop_h[1], crop_w[0]:crop_w[1]]

    return img, seg

def ImgNormalize(img, scale):
    img = img.astype('float') / (scale / 2.0)
    img -= 1
    return img

def ImgInverNormalize(img, scale):
    img += 1
    img *= (scale / 2.0)
    return img

def segmentLabeling(seg):
#    seg[seg == 1] = -1
#    seg[seg == 3] = 1
#    seg[seg == -1] = 3
    seg[seg > 3] = 4
# ground id : 0 / table id : 1 / tray id : 2 / robot id : 3 / obj id : 4 ~ 64
    seg[seg == -1] = 1      # 카메라 뷰 포인트로 안찍히는 곳은 테이블과 동일한 아이디로 보냄
    seg[seg == 0] = 1       # ground는 table과 동일한 아이디로
# 변경 후.
# ground id & table id : 1 / tray : 2 / robot:3 / obj : 4 ~64
# 0, -1번에는 아무것도 존재하지 않음.
    seg = seg - 1.0

    return seg

def WriteData(rootpath, prev_img, aftor_img, prev_seg, after_seg,
              action, prev_internal, after_internal, reward, terminal, idx, imgext = '.bmp'):
    pid = os.getpid()
    nowtime = datetime.now()
    strNowTime = nowtime.strftime("%y%m%d_%H%M%S")
    FileName = '%d_%s_%d' % (pid, strNowTime, idx)

    img_before_path, seg_before_path = ImgWrite(rootpath + "/before", FileName, prev_img, prev_seg, imgext)
    img_after_path, seg_after_path = ImgWrite(rootpath + "/after", FileName, aftor_img, after_seg, imgext)

    data_dict = {'img before': img_before_path,
                 'img after': img_after_path,
                 'seg before': seg_before_path,
                 'seg after': seg_after_path,
                 'action': action,
                 'internal state before': prev_internal,
                 'internal state after': after_internal,
                 'reward': reward,
                 'terminal': terminal}

    path = rootpath + '/file/' + FileName + '.txt'
    with open(path, 'w') as f:
        f.write(json.dumps(data_dict))

    return path

def ReadData(path):
    with open(path, 'r') as f:
        data_dict = json.load(f)

    if data_dict is None:
        raise NameError('data parsing fail')

    img_before_path = data_dict['img before']
    img_after_path = data_dict['img after']
    seg_before_path = data_dict['seg before']
    seg_after_path = data_dict['seg after']
    action = data_dict['action']
    prev_internal = data_dict['internal state before']
    after_internal = data_dict['internal state after']
    reward = data_dict['reward']
    terminal = data_dict['terminal']

    img_before = cv2.imread(img_before_path)
    img_after = cv2.imread(img_after_path)
    seg_before = np.load(seg_before_path)
    seg_after = np.load(seg_after_path)
    action = np.array(action)
    prev_internal = np.array(prev_internal)
    after_internal = np.array(after_internal)
    reward = np.array([reward])
    terminal = np.array([terminal])

    #image bgr to rgb
    img_before = img_before[..., ::-1]  # bgr to rgb
    img_after = img_after[..., ::-1]  # bgr to rgb

    img_before, seg_before = random_crop(img_before, seg_before)
    img_before = ImgNormalize(img_before, 255.)
    img_after, seg_after = random_crop(img_after, seg_after)
    img_after = ImgNormalize(img_after, 255.)

    seg_before = segmentLabeling(seg_before)
    seg_before = ImgNormalize(seg_before, 3.)  # -1 에서  1사이의 값을 가지도록
    seg_after = segmentLabeling(seg_after)
    seg_after = ImgNormalize(seg_after, 3.)  # -1 에서  1사이의 값을 가지도록

    data_dict = {'img before': img_before,
                 'img after': img_after,
                 'seg before': seg_before,
                 'seg after': seg_after,
                 'action': action,
                 'internal state before': prev_internal,
                 'internal state after': after_internal,
                 'reward': reward,
                 'terminal': terminal}

    return data_dict

def ImgWrite(rootpath, filename, rgb, seg, imgext = '.bmp'):
    img_path = rootpath + '/' + filename + imgext
    seg_path = rootpath + '/' + filename

    img = cv2.cvtColor(rgb, cv2.COLOR_RGB2BGR)
    cv2.imwrite(img_path, img)
    np.save(seg_path, seg)

    seg_path += '.npy'

    return img_path, seg_path