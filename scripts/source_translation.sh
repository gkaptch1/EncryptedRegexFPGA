#!/bin/sh

echo "$LLVM_BUILD set to "
echo $LLVM_BUILD
echo "\n"

echo "$REPO_HOME set to "
echo $REPO_HOME
echo "\n"

export $CUR_DIR=$(pwd)

echo "Stripping out the problematic chars...\n"
#$LLVM_BUILD/bin/clean-opencl-modifiers $1 > $1.clean

echo "Generating custom dot file....\n"
$LLVM_BUILD/bin/custom-dot-gen $CUR_DIR/$1.clean > $CUR_DIR/$1.dot

echo "Rebalancing AST...\n"
$REPO_HOME/graph-tool/graph-load.py $CUR_DIR/$1.dot > $CUR_DIR/$1.dot

echo "Regenerating AutoESL compatible C...\n"
