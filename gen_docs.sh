#!/usr/bin/bash
# combine doc files using post processor directives
cd media/docs || exit; perl -ne 's/^\[(.+)\].*/`cat $1`/e;print' README.md > ../../README.md