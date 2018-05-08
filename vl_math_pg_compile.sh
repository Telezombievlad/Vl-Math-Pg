#!/bin/sh
# VlMathPg compiler call:
# vl_math_pg_compile <file.vmpg> <res_folder>

BIN_FOLDER="/Users/vladislav_aleinik/Dropbox/Programming/2017-2018/Vl-Math-PG/bin"
FILE_NAME=`basename $1 .vmpg`

echo `${BIN_FOLDER}/valang_translate.out $1 $2/${FILE_NAME}.valang`
echo `${BIN_FOLDER}/valang_assemble.out $2/${FILE_NAME}.valang --std=1 $2/${FILE_NAME}.vacode`