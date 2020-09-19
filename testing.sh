#!/bin/bash

if [ $# -ne 2 ]; then
    echo "numero incorrecto de args"
    exit
fi

t_optimo=50

if [ $1 = "1" ]; then
    file=results/t_resuls_good_random.txt
    #pruebas para calcular T
    for x in 10 20 50 100 200 500
        do
        for y in 2 3 4
            do
            for z in {100..600..100}
            do
            echo "./lookup.o 10000 $x $z 2048 $y $file"
            ./lookup.o 10000 $x $z 2048 $y $file
            done
        done
    done
    exit
fi

if [ $1 = "2" ]; then
    #pruebas para ambos algoritmos
    for n in 10 20 50 100 200 500 600 
    do
        for m in 2048 3072 4096 5120 
        do
            for k in 2 3 4 
            do
                echo "./lookup.o 10000 $t_optimo $n $m $k $2"
                ./lookup.o 10000 $t_optimo $n $m $k $2
            done
        done
    done
    exit
fi

if [ $1 = "3" ]; then
    file=improve_delta_results.txt
    #pruebas para ambos algoritmos
    for n in 10 20 50 100 200 500 600 
    do
        for m in 2048 3072 4096 5120 
        do
            for k in 2 3 4 
            do
                echo "./improve_delta.o 10000 $t_optimo $n $m $k $2"
                ./improve_delta.o 10000 $t_optimo $n $m $k $2
            done
        done
    done
    exit
fi
#pruebas para ambos algoritmos
for n in 10 20 50 100 200 500 600 
do
    for m in 2048 3072 4096 5120 
    do
        for k in 2 3 4 
        do
            echo "./improve_delta.o 10000 $t_optimo $n $m $k $2"
                ./prediction_algorithm.o 10000 $t_optimo $n $m $k $2
        done
    done
done
