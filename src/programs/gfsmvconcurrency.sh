#!/bin/bash

gfsmfile="$1"
gfsmshort=$(basename "$gfsmfile")
gfsmconcurrency=$(dirname $0)/gfsmconcurrency.sh

(
cat <<EOF;
  set xlabel "Depth";
  set ylabel "Number of States";
  set title "Concurrency / $gfsmshort";
  plot "-" w lp title "$gfsmshort";
EOF
$gfsmconcurrency -v $gfsmfile
) | gnuplot -persist
