#!/bin/bash

((N = `ls mandelbrot-*.png | wc -l`))
echo "# You have $N images."

((L = `ls mandelbrot-*.png | tail -1 | sed -e 's/^.*-0*//' -e 's/.png//'` + 1))

((i = L + N))
for src in mandelbrot-*.png; do
    ((i = i - 1))
    dst=$(printf mandelbrot-%04d.png $i)
    ln -s $src $dst
done
