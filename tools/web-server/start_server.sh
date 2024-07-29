stream_path="${PWD%/*}"
stream_path="${stream_path%/*}"
if [ -z "$1" ]; then
    is_single_process=False
else
    is_single_process=$1
fi
echo $is_single_process
export LD_LIBRARY_PATH=$stream_path/build/lib/:$LD_LIBRARY_PATH
python3 server.py --stream_path=$stream_path --is_single_process=$is_single_process