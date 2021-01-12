#!/bin/bash

MY_DIR="$(readlink -f "$(dirname "$0")")"
cd ${MY_DIR}/..

rm -rf bin
rm -rf obj
make -f Makefile