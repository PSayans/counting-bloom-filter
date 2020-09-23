#pruebas para ambos algoritmos
for n in {100..600..100} 
do
    for m in 2048 3072 4096 5120 
    do
        for k in 2 3 4 
        do
            echo "./lookup.o 10000 1 $n $m $k $1 geometrico 0.1"
            ./lookup.o 10000 1 $n $m $k $1 geometrico 0.1
        done
    done
done
exit
