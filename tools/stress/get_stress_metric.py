import time
import argparse
import re
import numpy as np
import collections


class Meter:
    def __init__(self, size=200):
        self.queue = collections.deque(maxlen=size)
        self.t = 0

    def put(self, value):
        value = float(value)
        if value is float('nan'):
            return
        self.queue.append(value)

    def avg(self):
        if len(self.queue) == 0:
            return 0
        return np.array(self.queue).mean()

    def min(self):
        if len(self.queue) == 0:
            return 0
        return np.array(self.queue).min()

    def max(self):
        if len(self.queue) == 0:
            return 0
        return np.array(self.queue).max()

    def tic(self):
        self.t = time.time()

    def toc(self):
        self.put(time.time() - self.t)


def generate_markdown_table_head(items):
    strcol = "|"
    strflag = "|"
    for i in range(len(items)):
        strcol += items[i] + "|"
        if i == 0:
            strflag += "--------|"
        elif i == len(items) - 1:
            # strflag += ":----:|"
            strflag += "----|"
        else:
            # strflag += "-----: |"
            strflag += "-----|"
    return strcol + "\n" + strflag



def format2f(x):
    if isinstance(x, str):
        x = float(x)
    return '{:.2f}'.format(x)


def generate_rows(d_list):
    d_cpu_usage, d_tpu_usage, d_system_memory, d_system_memory_peak, d_tpu_memory, d_tpu_memory_peak, d_peak_fps, d_avg_fps, \
        d_vpp_used, d_vpp_rate, d_vpp_peak, \
        d_npu_used, d_npu_rate, d_npu_peak, \
        d_vpu_used, d_vpu_rate, d_vpu_peak, d_extra = d_list

    rows = []
    key_list = list(d_cpu_usage.keys())
    for k in key_list:
        route = k[0]
        algo_thread = k[1:]

        cpu_usage = d_cpu_usage[k]
        tpu_usage = d_tpu_usage[k]
        system_memory = d_system_memory[k]
        sys_mem_peak = d_system_memory_peak[k]
        tpu_memory = d_tpu_memory[k]
        tpu_memory_peak = d_tpu_memory_peak[k]
        peak_fps = d_peak_fps[k]
        avg_fps = d_avg_fps[k]

        vpp_used = d_vpp_used[k]
        vpp_rate = d_vpp_rate[k]
        vpp_peak = d_vpp_peak[k]

        npu_used = d_npu_used[k]
        npu_rate = d_npu_rate[k]
        npu_peak = d_npu_peak[k]

        vpu_used = d_vpu_used[k]
        vpu_rate = d_vpu_rate[k]
        vpu_peak = d_vpu_peak[k]

        extra = d_extra[k]
        rows.append([
            str(route),
            algo_thread,
            format2f(cpu_usage),
            format2f(system_memory),
            format2f(sys_mem_peak),
            format2f(tpu_usage),
            format2f(tpu_memory),
            format2f(tpu_memory_peak),
            format2f(avg_fps),
            format2f(peak_fps),
            format2f(npu_rate),
            format2f(npu_used),
            format2f(npu_peak),
            format2f(vpp_rate),
            format2f(vpp_used),
            format2f(vpp_peak),
            format2f(vpu_rate),
            format2f(vpu_used),
            format2f(vpu_peak),
            extra
            ])

    return rows


# generate table
def generate_table(d_list):
    # d_cpu_usage, d_tpu_usage, d_system_memory, d_system_memory_peak, d_tpu_memory, d_tpu_memory_peak, d_peak_fps, d_avg_fps,
    # d_vpp_used, d_vpp_rate, d_vpp_peak, 
    # d_npu_used, d_npu_rate, d_npu_peak,
    # d_vpu_used, d_vpu_rate, d_vpu_peak,
    # d_extra
    s_list = []
    table_head_list = [
        "路数",
        "算法线程数",
        "CPU利用率(%)",
        "系统内存(M)",
        "系统内存峰值(M)",
        "TPU利用率(%)",
        "设备内存(M)",
        "设备内存峰值(M)",
        "平均fps",
        "峰值fps",
        "NPU利用率(%)",
        "NPU内存(M)",
        "NPU峰值内存(M)",
        "VPP利用率(%)",
        "VPP内存(M)",
        "VPP峰值内存(M)",
        "VPU利用率(%)",
        "VPU内存(M)",
        "VPU峰值内存(M)",
        "说明",
        ]
    head_str = generate_markdown_table_head(table_head_list)
    s_list.append(head_str)
    
    rows = generate_rows(d_list)


    for row in rows:
        row_str = '|' + '|'.join(list(map(str, row))) + '|'
        s_list.append(row_str)
    for s in s_list:
        print(s)


def parse_alg(filepath):
    """
    demo] tmp_fps: 233.694, avg_fps: 207.991
    """
    tmp_fps_list = []
    avg_fps_list = []
    with open(filepath, 'r') as f:
        for i, l in enumerate(f):
            if 'tmp_fps' in l:
                re_res = re.search('demo] tmp_fps: (\d+[.]\d+), avg_fps: (\d+[.]\d+)', l)
                if(re_res):
                    if(float(re_res.group(1)) > 0):
                        tmp_fps_list.append(float(re_res.group(1)))
                    if(float(re_res.group(2)) > 0):
                        avg_fps_list.append(float(re_res.group(2)))
        tmp_fps = np.max(tmp_fps_list)
        avg_fps = np.mean(avg_fps_list)
    return tmp_fps, avg_fps


def parser_top(filepath):
    with open(filepath, 'r') as f:
        res_list = []
        cpu_list = []
        for i, l in enumerate(f):
            ls = l.split()
            if len(ls) != 2: 
                break
            res_list.append(float(ls[1])/1000)
            cpu_list.append(float(ls[0]))
        res_mean = np.mean(res_list)
        res_peak = np.max(res_list)
        cpu_mean = np.mean(cpu_list)
        cpu_peak = np.max(cpu_list)
        return res_mean, res_peak, cpu_mean, cpu_peak


def parse_tpu(filepath):
    """
    可能类型
    | 0  1684-SOC     SOC               N/A | 0    N/A     N/A     N/A     N/A  N/A    N/A         94% |
    |   N/A   N/A   N/A   75M     550M   N/A|        N/A Active    550M       N/A  2694MB/ 7983MB      |

    | 0 1684X-SOC     SOC      N/A          | 0    N/A     N/A     N/A     N/A  N/A    N/A          8% |
    |   N/A   N/A   N/A   75M    1000M   N/A| N/A        Active   1000M       N/A     0MB/ 9070MB      |
    """
    meter_tpu_usage = Meter()
    meter_tpu_memory = Meter()
    tpu_usage = '0'
    tpu_memory = '0'
    tpu_memory_max = -1
    with open(filepath, 'r') as f:
        for i, l in enumerate(f):
            if '1684-SOC' in l or '1684X-SOC'  in l:
                p_list = [
                    '\| 0  1684\-SOC     SOC               N/A \| 0    N/A     N/A     N/A     N/A  N/A    N/A         ([\d]{1,})% \|',
                    '\| 0 1684X\-SOC     SOC      N/A          \| 0    N/A     N/A     N/A     N/A  N/A    N/A         ([\d]{1,})% \|',
                    '\| 0 1684X\-SOC     SOC      N/A          \| 0    N/A     N/A     N/A     N/A  N/A    N/A       ([\d]{1,})%  \|',
                ]
                for p in p_list:
                    re_res = re.search(p, l)
                    if re_res:
                        tpu_usage = re_res.group(1)
                        if float(tpu_usage)>0:
                            meter_tpu_usage.put(tpu_usage)
                        break
            if 'Active' in l:
                p_list = [
                    '\|   N/A   N/A   N/A   \d{1,}M     \d{1,}M   N/A\|        N/A Active    550M       N/A([\d ]{5,})MB/ (\d{1,})MB      \|',
                    '\|   N/A   N/A   N/A   \d{1,}M    \d{1,}M   N/A\| N/A        Active   1000M       N/A([\d ]{5,})MB/ (\d{1,})MB      \|',
                ]
                for p in p_list:
                    re_res = re.search(p, l)
                    if re_res:
                        tpu_memory = re_res.group(1).strip(' ')
                        if(float(tpu_usage) > 0):
                            meter_tpu_memory.put(tpu_memory)
                            tpu_memory = float(tpu_memory)
                            if tpu_memory > tpu_memory_max:
                                tpu_memory_max = tpu_memory
                            break
                    
    return meter_tpu_usage.avg(), meter_tpu_memory.avg(), tpu_memory_max


def parse_dev(filepath):
    with open(filepath, 'r') as f:
        vpp_used_list = []
        vpp_rate_list = []
        vpp_peak_list = []
        npu_used_list = []
        npu_rate_list = []
        npu_peak_list = []
        vpu_used_list = []
        vpu_rate_list = []
        vpu_peak_list = []
        pattern = 'used:(\d+) bytes	usage rate:(\d+)%, memory usage peak (\d+) bytes'
        for i, l in enumerate(f):
            if 'Summary' not in l:
                re_res = re.search(pattern, l)
                if re_res:
                    if(float(re_res.group(2)) > 0):
                        if 'npu' in l:
                            npu_rate_list.append(float(re_res.group(2)))
                            npu_used_list.append(float(re_res.group(1))/1000000)
                            npu_peak_list.append(float(re_res.group(3))/1000000)
                        elif 'vpp' in l:
                            vpp_rate_list.append(float(re_res.group(2)))
                            vpp_used_list.append(float(re_res.group(1))/1000000)
                            vpp_peak_list.append(float(re_res.group(3))/1000000)
                        elif 'vpu' in l:
                            vpu_rate_list.append(float(re_res.group(2)))
                            vpu_used_list.append(float(re_res.group(1))/1000000)
                            vpu_peak_list.append(float(re_res.group(3))/1000000)
        vpp_used_mean = np.mean(vpp_used_list)
        vpp_rate_mean = np.mean(vpp_rate_list)
        vpp_peak = np.max(vpp_peak_list)
        npu_used_mean = np.mean(npu_used_list)
        npu_rate_mean = np.mean(npu_rate_list)
        npu_peak = np.max(npu_peak_list)
        vpu_used_mean = np.mean(vpu_used_list)
        vpu_rate_mean = np.mean(vpu_rate_list)
        vpu_peak = np.max(vpu_peak_list)

        return vpp_used_mean, vpp_rate_mean, vpp_peak, npu_used_mean, npu_rate_mean, npu_peak, vpu_used_mean, vpu_rate_mean, vpu_peak


def parse_args():
    # python3 get_stress_metric.py --alg_log ./yolov5_demo/1111/alg.log --tpu_log ./yolov5_demo/1111/tpu.log --host_log ./yolov5_demo/1111/host.log --dev_log ./yolov5_demo/1111/dev.log --channel_combination 1111
    parser = argparse.ArgumentParser()
    parser.add_argument('--alg_log', type=str, default='')
    parser.add_argument('--channel_combination', type=str, default='1111')
    parser.add_argument('--tpu_log', type=str, default='')
    parser.add_argument('--host_log', type=str, default='')
    parser.add_argument('--dev_log', type=str, default='')
    args = parser.parse_args()
    return args


def main():
    args = parse_args()

    eval_list = [
                [args.alg_log, args.host_log, args.tpu_log, args.dev_log, (args.channel_combination), ''], # 
              ]

    d_cpu_usage, d_system_memory = {}, {}
    d_system_memory_peak = {}
    d_cpu_usage_peak = {}
    d_tpu_usage, d_tpu_memory = {}, {}
    d_tpu_memory_peak = {}
    d_peak_fps = {}
    d_avg_fps = {}
    d_vpp_used, d_vpp_rate, d_vpp_peak = {}, {}, {}
    d_npu_used, d_npu_rate, d_npu_peak = {}, {}, {}
    d_vpu_used, d_vpu_rate, d_vpu_peak = {}, {}, {}

    d_extra = {}
    for file_alg, file_top, file_tpu, file_dev, k, extra in eval_list:
        tpu_usage, tpu_memory, tpu_memory_peak = parse_tpu(file_tpu)
        res_mean, res_peak, cpu_mean, cpu_peak = parser_top(file_top)
        peak_fps, avg_fps = parse_alg(file_alg)
        vpp_used_mean, vpp_rate_mean, vpp_peak, npu_used_mean, npu_rate_mean, npu_peak, vpu_used_mean, vpu_rate_mean, vpu_peak = parse_dev(file_dev)
        d_cpu_usage[k] = cpu_mean
        d_cpu_usage_peak[k] = cpu_peak
        d_system_memory[k] = res_mean
        d_system_memory_peak[k] = res_peak
        d_peak_fps[k] = peak_fps
        d_avg_fps[k] = avg_fps
        d_tpu_usage[k] = tpu_usage
        d_tpu_memory[k] = tpu_memory
        d_tpu_memory_peak[k] = tpu_memory_peak
        d_extra[k] = extra
        d_vpp_used[k] = vpp_used_mean
        d_vpp_rate[k] = vpp_rate_mean
        d_vpp_peak[k] = vpp_peak
        d_npu_used[k] = npu_used_mean
        d_npu_rate[k] = npu_rate_mean
        d_npu_peak[k] = npu_peak
        d_vpu_used[k] = vpu_used_mean
        d_vpu_rate[k] = vpu_rate_mean
        d_vpu_peak[k] = vpu_peak

    d_list = [
        d_cpu_usage, d_tpu_usage, d_system_memory, d_system_memory_peak, d_tpu_memory, d_tpu_memory_peak, d_peak_fps, d_avg_fps, 
        d_vpp_used, d_vpp_rate, d_vpp_peak, 
        d_npu_used, d_npu_rate, d_npu_peak,
        d_vpu_used, d_vpu_rate, d_vpu_peak,
        d_extra
    ]
    generate_table(d_list)

if __name__ == "__main__":
    main()
