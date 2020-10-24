    #pruebas para ambos algoritmos
    for m in 2048 3072 4096 5120 
    do
        for k in 2 3 4 
        do
            echo "./lookup.o 10000 50 1500 $m $k $1 normal 0"
            ./lookup.o 10000 50 1500 $m $k $1 normal 0
        done
    done
  exit
