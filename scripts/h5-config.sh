#!/bin/bash

command=$(h5c++ -show \;)
compiler_args=$(echo ${command} | cut -d';' -f1 | cut -d' ' -f2-)
linker_args=$(echo ${command} | cut -d';' -f2)

case "${1}" in
    "--cflags")
        echo "${compiler_args}"
        ;;
    "--libs")
        echo "${linker_args}"
        ;;
esac

