#!/bin/bash
#echo "building .dot file"
~/Documents/llvm_build/build/bin/clang -Xclang -ast-view $1
#echo "creating png"
#dot -Tpng:quartz:quartz ($1).dot > ($1).png

