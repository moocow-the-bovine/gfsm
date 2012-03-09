#!/bin/bash

args=()
datamode=""
doplot="y"

while test $# -gt 0 ; do
  a="$1"
  shift
  case "$a" in
    -data|--data|-d)
      datamode=y
      ;;
    -plot|--plot|-p)
      doplot=y
      ;;
    -noplot|-no-plot|--noplot|--no-plot|-P)
      doplot=""
      ;;
    *)
      args=("${args[@]}" "$a")
      ;;
  esac
done

test -n "$doplot" && plotcmd=(gnuplot -persist) || plotcmd=(cat)
gfsmconcurrency=$(dirname $0)/gfsmconcurrency.sh

if test "${#args[@]}" -eq 0 ; then
  args=("-")
fi

(
 cat <<EOF;
  set xlabel "Depth";
  set ylabel "Number of States";
  set title "States by Depth";
  set style data l;
EOF
 echo -n " plot ";
 i=0 ;
 while test $i -lt ${#args[@]} ; do
    f="${args[$i]}";
    test $i -eq 0 || echo -n ", ";
    echo -n "\"-\" title \"$(basename $f)\"";
    let "i=$i+1";
 done
 echo -e ";\n"
  ##
  ##-- plot data
 for f in "${args[@]}" ; do
     test -n "$datamode" && cat "$f" || $gfsmconcurrency -v $f
     echo "e"
 done
) | "${plotcmd[@]}"
