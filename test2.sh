#!/bin/bash

echo ${BASH_VERSION}

    for y in 2 3 4
        do
        for z in {100..600..100}
            do
            echo "ejecutando lookup con args $x $z y $y"
            ./lookup 1000 200 $z 2048 $y
            done
        done
