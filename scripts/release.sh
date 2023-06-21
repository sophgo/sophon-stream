
project_name=sophon-stream
shell_dir=$(dirname $(readlink -f "$0"))
basic_dir=$shell_dir/..

pushd ${basic_dir}

commit_id=$(git log -1 | awk 'NR==1 {print substr($2,0,8)}')
times=`date +%Y%m%d`
VERSION_PATH=$basic_dir/VERSION
echo $VERSION_PATH
exec < $VERSION_PATH
read -r line
version=$line
release_file_name="${project_name}_v${version}_${commit_id}_${times}"


result_dir=$basic_dir/${release_file_name}

if [ -d $result_dir ]; then
    rm -rf $result_dir
fi
mkdir -p $result_dir

cp -r framework $result_dir
cp -r element $result_dir
cp -r 3rdparty $result_dir
cp -r samples $result_dir
cp -r CMakeLists.txt $result_dir
cp -r docs $result_dir
cp -r Doxyfile $result_dir
cp -r README.md $result_dir
cp -r VERSION $result_dir

tar zcvf ${release_file_name}.tar.gz ${release_file_name}
popd
