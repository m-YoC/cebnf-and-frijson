#!/bin/bash

function except_dir() {
    echo -not -path "*/$1/*"
}

function find_file() {
    file_name=$2
    find $1 -type f -name ${file_name} $(except_dir old) $(except_dir unused)
}

function exist_only_one_file() {
    find_file $1 $2 | wc -l
}

function get_filepath() {
    path=$1
    extension=$2
    root_path=${3:-./}
    with_path=$4 # fullpath or not
    
    basename=$(basename $path $extension)
    dirname=""
    if [ "$with_path" == "--with_path" ]; then
        dirname=$(dirname $path)/
    else
        found_file=$(find_file $root_path $basename$extension)
        dirname=$(dirname ${found_file:-not_found})/
    fi

    if [ $(exist_only_one_file $dirname $basename$extension) -ne 1 ]; then
        echo "error"
        return 0
    fi

    echo $dirname$basename
}
