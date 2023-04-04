#!/bin/bash


if [ -z "$ILCSOFT" ]; then
   echo " You should source an init_ilcsoft.sh script"
   exit
fi

CreateBuildDir()
{
    if [ -d build ]; then
	rm -r build
    fi
    mkdir build
}

CreateMakefile()
{
    echo "============================================================"
    echo "=                 Build Makefile for compilation           ="
    echo "============================================================"    
    echo "  Create a new Makefile  .. "
    cmake -C ${ILCSOFT}/ILCSoft.cmake ..
    ls -lthr  
}

Compile()
{
    echo "============================================================"
    echo "=                      Compilation                         ="
    echo "============================================================"
    make install
}

#The script

WHAT=$1

if [ "$WHAT" == "Full" ]; then
    #generateRootDict
    CreateBuildDir
    cd build
    CreateMakefile
    cd ..
fi

cd build
Compile
cd ..
echo " Compilation done"


