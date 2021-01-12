#!/bin/bash

MY_DIR="$(readlink -f "$(dirname "$0")")"
cd ${MY_DIR}/../obj

${MY_DIR}/../bin/mc ${MY_DIR}/../test/test1.mc
${MY_DIR}/../bin/mc ${MY_DIR}/../test/test2.mc
${MY_DIR}/../bin/mc ${MY_DIR}/../test/test3.mc
${MY_DIR}/../bin/mc ${MY_DIR}/../test/test4.mc