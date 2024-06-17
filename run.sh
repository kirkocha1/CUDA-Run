#!/usr/bin/env bash
make clean build

make run ARGS="-input=data/woman.pgm -output=data/woman_filtered.pgm -filter=gauss_filter"