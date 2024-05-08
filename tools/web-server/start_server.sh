stream_path="${PWD%/*}"
stream_path="${stream_path%/*}"
export LD_LIBRARY_PATH=$stream_path/build/lib/:$LD_LIBRARY_PATH
python3 server.py --stream_path=$stream_path