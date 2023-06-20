alg_path=$1
alg_name=$(basename "$alg_path")
alg_folder=$(dirname "$alg_path")

alg_log=$2
tpu_log=$3
host_log=$4
dev_log=$5
dir_name=$6


monitor_host() {
  while [ 1 ]
  do
    ps -p $(pgrep -f "./${alg_name}"|head -n1) -o %cpu -o rss  |tail -n1
    sleep 1
  done
}
export -f monitor_host

monitor_bm() {
  local dev_log="$1"
  while [ 1 ]
  do
      sudo cat /sys/kernel/debug/ion/bm_npu_heap_dump/summary | head -2 >> "$dev_log"
      sudo cat /sys/kernel/debug/ion/bm_vpu_heap_dump/summary | head -2 >> "$dev_log"
      sudo cat /sys/kernel/debug/ion/bm_vpp_heap_dump/summary | head -2 >> "$dev_log"
      sleep 1
  done
}
# export -f monitor_bm

kill_monitor() {
    echo "kill monitor"
    ps aux |grep 'bm-smi --file' |awk -F' ' '{print $2}' | xargs kill -9
    ps aux |grep 'monitor_host' |awk -F' ' '{print $2}' | xargs kill -9
    ps aux |grep 'monitor_bm' |awk -F' ' '{print $2}' | xargs kill -9
}

trap ctrl_c INT
function ctrl_c() {
    echo "trap ctrl-c"
    kill_monitor
}

# sudo -i
sudo echo 0 > /sys/kernel/debug/ion/bm_npu_heap_dump/peak
sudo echo 0 > /sys/kernel/debug/ion/bm_vpu_heap_dump/peak
sudo echo 0 > /sys/kernel/debug/ion/bm_vpp_heap_dump/peak
rm -rf $dev_log
mkdir -p $alg_name/$dir_name
export alg_name=${alg_name}
export dev_log=${dev_log}

nohup bash -c monitor_host "${alg_name}" > ${host_log} 2>&1 &
monitor_bm "${dev_log}" &
func_pid=$!

nohup bm-smi --file $tpu_log > /dev/null 2>&1 &

echo "to demo path: " $alg_folder
pushd $alg_folder
./${alg_name} > $alg_log 2>&1
kill_monitor
kill "$func_pid"
popd

mv $alg_folder/$alg_log $alg_name/$dir_name
mv $tpu_log $alg_name/$dir_name
mv $host_log $alg_name/$dir_name
mv $dev_log $alg_name/$dir_name


# ./stress.sh ../../samples/yolov5/build/yolov5_demo alg.log tpu.log host.log dev.log 1111