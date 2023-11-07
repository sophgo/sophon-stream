# -*- coding: utf-8 -*- 
import os
import time
import json
import cv2
import numpy as np
import argparse

import sophon.sail as sail
import logging
logging.basicConfig(level=logging.INFO)


class Resnet(object):
    def __init__(self, args):
        # load bmodel
        self.net = sail.Engine(args.bmodel, args.dev_id, sail.IOMode.SYSIO)
        self.graph_name = self.net.get_graph_names()[0]
        self.input_names = self.net.get_input_names(self.graph_name)
        self.input_shapes = [self.net.get_input_shape(self.graph_name, name) for name in self.input_names]
        self.output_names = self.net.get_output_names(self.graph_name)
        self.output_shapes = [self.net.get_output_shape(self.graph_name, name) for name in self.output_names]
        logging.debug("load {} success!".format(args.bmodel))
        logging.debug(str(("graph_name: {}, input_names & input_shapes: ".format(self.graph_name), self.input_names, self.input_shapes)))
        logging.debug(str(("graph_name: {}, output_names & output_shapes: ".format(self.graph_name), self.output_names, self.output_shapes)))
        self.input_name = self.input_names[0]
        self.input_shape = self.input_shapes[0]

        self.batch_size = self.input_shape[0]
        self.net_h = self.input_shape[2]
        self.net_w = self.input_shape[3]
        
        self.mean=[0.485, 0.456, 0.406]
        self.std=[0.229, 0.224, 0.225]

        self.preprocess_time = 0.0
        self.inference_time = 0.0
        self.postprocess_time = 0.0

        self.handle = self.net.get_handle()
        self.bmcv = sail.Bmcv(self.handle)
        self.input_dtype = self.net.get_input_dtype(self.graph_name, self.input_name)
        self.output_dtype = self.net.get_output_dtype(self.graph_name, self.output_names[0])
        self.img_dtype = self.bmcv.get_bm_image_data_format(self.input_dtype)

    def preprocess(self, img):
        h, w, _ = img.shape
        if h != self.net_h or w != self.net_w:
            img = cv2.resize(img, (self.net_w, self.net_h))
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        img = img.reshape((128, 128,1))
        img = img.transpose((2, 0, 1))
        img = img.astype(np.float32, copy=False)
        img -= 127.5
        img /= 127.5
    
        return img

    def predict(self, input_img):
        input_data = {self.input_name: input_img}
        outputs = self.net.process(self.graph_name, input_data)
        return list(outputs.values())[0]

    def postprocess(self, outputs):
        res = list()
        outputs_exp = np.exp(outputs)
        outputs = outputs_exp / np.sum(outputs_exp, axis=1)[:,None]
        predictions = np.argmax(outputs, axis = 1)
        for pred, output in zip(predictions, outputs):
            score = output[pred]
            res.append((pred.tolist(),float(score)))
        return res

    def __call__(self, img_list):
        img_num = len(img_list)
        img_input_list = []
        for img in img_list:
            start_time = time.time()
            img = self.preprocess(img)
            self.preprocess_time += time.time() - start_time
            img_input_list.append(img)
        
        if img_num == self.batch_size:
            input_img = np.stack(img_input_list)
            start_time = time.time()
            outputs = self.predict(input_img)
            self.inference_time += time.time() - start_time
        else:
            input_img = np.zeros(self.input_shape, dtype='float32')
            input_img[:img_num] = np.stack(img_input_list)
            start_time = time.time()
            outputs = self.predict(input_img)[:img_num]
            self.inference_time += time.time() - start_time
        res=outputs
        return res

    def get_time(self):
        return self.dt

def main(args):
    resnet = Resnet(args)
    batch_size = resnet.batch_size

    output_dir = "./results"
    if not os.path.exists(output_dir):
        os.mkdir(output_dir)
    
    if not os.path.isdir(args.input):
        # logging.error("input must be an image directory.")
        # return 0
        raise Exception('{} is not a directory.'.format(args.input))
    
    db_data_path=args.db_data
    index_label_path=args.index_label

    db_data = open(db_data_path, 'a')
    index_label = open(index_label_path, 'a')
    img_list=[] 
    filename_list=[]
    num_Sum=0;
     # 遍历根目录下的每个文件夹
    for folder_name in os.listdir(args.input):
        folder_path = os.path.join(args.input, folder_name)
        # 确保路径是一个文件夹而不是文件
        if os.path.isdir(folder_path):
            # if num_Sum>10:
            #     break;
            # 遍历当前文件夹中的图像文件
            for filename in os.listdir(folder_path):
                if os.path.splitext(filename)[-1] not in ['.jpg', '.png', '.jpeg', '.bmp', '.JPEG', '.JPG', '.BMP']:
                    continue
                full_path = os.path.join(folder_path, filename)
                src_img = cv2.imread(full_path)
                if src_img is None:
                    logging.error("{} imread is None.".format(full_path))
                    continue
                img_list.append(src_img)
                filename_list.append(full_path)
                if len(img_list) == batch_size:
                    feature_vector = (resnet(img_list))[0]
                    for i, filename in enumerate(filename_list):
                       parts = filename.split('/')
                       label = parts[-2]
                       print("real:"+label)
                       np.savetxt(db_data, np.array([feature_vector]))
                       index_label.write(label + '\n')  # 写入 label 并换行
                       num_Sum+=1
                    img_list = []
                    filename_list = []
    db_data.close()
    index_label.close()
                  
def argsparser():
    parser = argparse.ArgumentParser(prog=__file__)
    parser.add_argument('--input', type=str, default='./data/images/face_data_train', help='path of input, must be image directory')
    parser.add_argument('--bmodel', type=str, default='./data/models/BM1684X/resnet_arcface_fp32_1b.bmodel', help='path of bmodel')
    parser.add_argument('--db_data', type=str, default="faiss_db_data.txt", help='db_data')
    parser.add_argument('--index_label', type=str, default="faiss_index_label.name", help='index_label')
    parser.add_argument('--dev_id', type=int, default=0, help='tpu id')

    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = argsparser()
    main(args)

