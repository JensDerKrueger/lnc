main : main.o
  ln +Ccdb main.o -lwork:aztec/lib/mf -lwork:aztec/lib/c

main.o : main.c
  cc main.c -iwork:aztec/include -ff 

clean:
  delete main.o
  delete main
