#!/bin/bash

t_optimo=50

rm results.txt
#pruebas para calcular T
for x in 10 20 50 100 200 500 
    do
    for y in 2 3 4
        do
        for z in {100..600..100}
        do
        echo "ejecutando lookup con args $x $z y $y"
        ./lookup 10000 $x $z 2048 $y
        done
    done
done

rm results_prediction.txt
#pruebas para ambos algoritmos
for n in 10 20 50 100 200 500 600 do
    for m in 2048 3072 4096 5120 do
        for k in 2 3 4 do
            echo "ejecutando algoritmo delta mejorado con args $n $m y $k"
            ./improve_delta 10000 $t_optimo $n $m $k 
        done
    done
done

rm results_prediction.txt
#pruebas para ambos algoritmos
for n in 10 20 50 100 200 500 600 do
    for m in 2048 3072 4096 5120 do
        for k in 2 3 4 do
            echo "ejecutando algoritmo prediccion con args $n $m y $k"
            ./prediction_algorithm 10000 $t_optimo $n $m $k 
        done
    done
done
