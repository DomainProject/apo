#!/bin/bash

libs="GKlib METIS"

for lib in $libs; do
  git clone https://github.com/KarypisLab/$lib.git
  cd $lib
  make config shared=1 i64=1 r64=1
  make
  make install
  cd ..
done



