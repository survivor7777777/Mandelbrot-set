#!/bin/bash

dir=$(dirname $0)
cmd=$(basename $0)

Usage() {
    echo Usage: $cmd [ --loop ] parameter-file.dat
}

Abort() {
    echo "$@" 1>&2
    exit 1
}

LOOP=0
while [ $# -gt 0 ]; do
    case "$1" in
    --loop)
	LOOP=1; shift;;
    -*)
	Usage; exit 1;;
    *)
	break;;
    esac
done

if [ $# -ne 1 ]; then
    Usage
    exit 1
fi

FILE="$1"; shift
if [ ! -r "$FILE" ]; then
    echo "$FILE: No such parameter file" 1>&2
    exit 1
fi

source "$FILE"

if [ -z "$CENTER_RE" -o -z "$CENTER_IM" -o \
    -z "$RANGE_S" -o -z "$RANGE_E" -o -z "$THETA_S" -o -z "$TURNS" -o \
    -z "$FRAMES" -o -z "$IMAGE_WIDTH" -o -z "$IMAGE_HEIGHT" ]; then
    Abort "ERROR: Some of the parameters are missing"
fi

(ls | grep -q 'mandelbrot-.*\.png') && Abort "ERROR: You have to remove mandelbrot-*.png first"

$dir/mandelbrot $CENTER_RE $CENTER_IM \
    $RANGE_S $RANGE_E $THETA_S $TURNS $FRAMES \
    $IMAGE_WIDTH $IMAGE_HEIGHT || Abort "ERROR in image generation"

if [ "$LOOP" -eq 1 ]; then
    $dir/loop.sh || Abort "ERROR in loop.sh"
fi

VIDEO="$(basename $FILE .dat).mp4"
rm -rf $VIDEO
ffmpeg -r 30 -i mandelbrot-%04d.png -vcodec libx264 -pix_fmt yuv420p -r 30 "$VIDEO" || Abort "Error in ffmpeg"

echo "$cmd: successfully generated $VIDEO"
