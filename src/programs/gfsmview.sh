#!/bin/sh

GFSMDRAW=./gfsmdraw
DOTGV=dotgv.sh

exec ./gfsmdraw $* | $DOTGV
