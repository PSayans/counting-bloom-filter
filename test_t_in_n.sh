#pruebas para ambos algoritmos

    for m in 4096 5120 
    do
        for k in 2 3 4 
        do
            echo "./lookup.o 10000 1 600 $m $k $1 geometrico 1"
            ./lookup.o 10000 1 600 $m $k $1 geometrico 1
        done
    done
exit
