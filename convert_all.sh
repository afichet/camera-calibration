#!/bin/bash


folder_in=/home/afichet/Documents/Research/Measurements/Calibration_CHECKERBOARD_Oct_2020/LED/

folder_out=output

files_in=$(find $folder_in -name "*.txt" | uniq)


for file in ${files_in}
do
    filename=$(basename ${file})
    
    dir=${file%${filename}}
    dir=${folder_out}/${dir:${#folder_in}:${#dir}}

    if [ ! -d "$dir" ]; then
        mkdir -p ${dir}
    fi
    
    path_filename_out=${dir}/${filename}.exr
    echo ${file} >> in_list
    echo ${path_filename_out} >> out_list
    
done

parallel --xapply -a in_list -a out_list ./build/bin/derawzinator
