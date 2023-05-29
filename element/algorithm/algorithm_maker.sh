#!/bin/bash

basedir=`dirname $0`
pushd ${basedir}

while [ "$1" ] ; do
    case $1 in
        --help)
            cat <<-EOF
Usage: algorithm_maker [OPTIONS] ALGORITHM_NAME
Create a ophon_stream algorithm element that subclasses Element.
Options:
  --help             Print this information
Example: './algorithm_maker.sh alg' will create the files
  that implement alg, a subclass of Element,
  as an element named alg.
EOF
            exit 0
            ;;
        -*)
            echo Unknown option: $1
            exit 1
            ;;
        *)
            if [ "$name" = "" ]; then
                name=$1
            else
                echo Ignored: $1
            fi
    esac
    shift
done

if [ "$name" = "" ]; then
    echo "Usage: algorithm_maker [OPTIONS] ALGORITHM_NAME"
    exit 1
fi

template_folder="./template"
source_files="${template_folder}/*.h ${template_folder}/*.cpp ${template_folder}/*.txt"
pattern="template"
Pattern="Template"


first_char="${name:0:1}"
remaining="${name:1}"
first_char_upper=$(echo "$first_char" | tr '[:lower:]' '[:upper:]')
Name="$first_char_upper$remaining"
# echo "$Name"
mkdir $name

for file in ${source_files}; do
    echo "${file}"
    output_file=$(echo "$file" | sed -e "s/${pattern}/${name}/g" -e "s/${Pattern}/${Name}/g")
    echo "${output_file}"
    sed -e "s/${pattern}/${name}/g" -e "s/${Pattern}/${Name}/g" "${file}" > "${output_file}"
done

popd