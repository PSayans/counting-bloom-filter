#!/bin/bash

echo ${BASH_VERSION}

for x in 10 20 50 100 500 
    do
    for y in 2 3 4
        do
        for z in {100..600..100}
            do
            echo "ejecutando lookup con args $x $z y $y"
            ./lookup 1000 $x $z 2048 $y
            done
        done
    done