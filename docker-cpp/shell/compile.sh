#!/bin/bash

# ./compile.sh <filename> [options...]

export TZ=Asia/Tokyo

PROJECT_ROOT="/$WORKDIR"
LOG_FILE="logs/compile_log.txt"

OPT_F=""
OPT_V=""
OPT_G=""
RUN_PROGRAM=f
# SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
FIND_ROOT_DIR=./

# 関数ファイルを読み込み
. get_filepath.sh

# option indexの位置をリセット (". compile.sh"を使っている場合getopts使用済みのためOPTINDの値がずれている可能性がある)
OPTIND=2
# option解釈
while getopts :vgrfd: option
do
    case $option in
        v)
            OPT_V="-v";;
        g)
            OPT_G="-g";;
        r)
            RUN_PROGRAM=t;;
        f)
            OPT_F="--with_path";;
        d)
            FIND_ROOT_DIR=./*/$OPTARG;;
        :)
            echo "Error: Option $OPTARG requires an argument."
            exit 0;;
        \?)
            echo "Error: You ran this script with invalid option."
            exit 0;;
    esac
done

# compile_fileの設定
COMPILE_FILE=$(get_filepath $1 .cpp $FIND_ROOT_DIR $OPT_F)

if [ "$COMPILE_FILE" == "error" ]; then
    echo  "Error: The number of found file << $1 >> was not one."
    exit 1
fi


{
    echo "---------------------------------------------------------------------------------------------"
    echo "                                     Start Compile                                           "
    echo "---------------------------------------------------------------------------------------------"
    date

    # -pthreadがないとstd::threadで例外が発生する
    if [ "${COMPILER}" = "gcc" ]; then
        echo "Compile << ${COMPILE_FILE}.cpp >> using g++"
        g++ -std=c++17 -pthread --include-directory ${PROJECT_ROOT} ${OPT_V} ${OPT_G} ${COMPILE_FILE}.cpp -o ${COMPILE_FILE}
        echo "Finished."
    elif [ "${COMPILER}" = "clang" ]; then
        echo "Compile << ${COMPILE_FILE}.cpp >> using clang++"
        clang++ -std=c++17 -pthread --include-directory ${PROJECT_ROOT} ${OPT_V} ${OPT_G} ${COMPILE_FILE}.cpp -o ${COMPILE_FILE}
        echo "Finished."
    fi
} 2>&1 | tee -a ${LOG_FILE}

echo -e "---------------------------------------------------------------------------------------------\n"
echo -e "\n\n" &>> ${LOG_FILE}

if [ "${RUN_PROGRAM}" = "t" ]; then
    echo "Run Program..."
    ./${COMPILE_FILE}
    echo -e "\n"
fi
