#!/bin/bash

if test $# -gt 1 -a "$1" = "-v" ; then
  verbose=1
  shift
fi

exec gfsmdepths "$@" \
 | cut -d'	' -f 2 \
 | env - LC_ALL=C sort \
 | env - LC_ALL=C uniq -c \
 | awk '{print $2 "\t" $1}' \
 | sort -n \
 | perl -e 'BEGIN{$c=$nq=0;} while(<>) {($d,$n)=split(/\t/,$_,2); $c += $n**2; $nq += $n; print if ('$verbose');} END { print "#AVG=", $c/$nq, "\n"; };'
