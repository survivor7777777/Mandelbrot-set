#!/bin/bash

dir=$(dirname $0)
cmd=$(basename $0)

Usage() {
    echo Usage: $cmd [ --help ] parameter-file.dat
}

Abort() {
    echo "$@" 1>&2
    exit 1
}

while [ $# -gt 0 ]; do
    case "$1" in
    --help|-h)
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


#default parameters
CENTER_RE=-0.743291890852430203178864079732928781048879471415530370083313150
CENTER_IM=0.1312405523087976045493474233902208417186606174208137220225662074
RANGE_S=1e-2
RANGE_E=1e-5
THETA_S=3.14159265358979
TURNS=-2
FRAMES=100
IMAGE_WIDTH=320
IMAGE_HEIGHT=180
LOOP=0

# read parameters
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
ffmpeg -y -r 30 -i mandelbrot-%04d.png -vcodec libx264 -pix_fmt yuv420p -r 30 "$VIDEO" || Abort "Error in ffmpeg"

echo "$cmd: successfully generated $VIDEO"
