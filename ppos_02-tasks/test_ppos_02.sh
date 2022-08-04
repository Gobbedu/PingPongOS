#!/bin/bash

make
make clean && make 
./tasks1 > saida1.txt
./tasks2 > saida2.txt
./tasks3 > saida3.txt

diff saida1.txt saida_esperada1.txt
diff saida2.txt saida_esperada2.txt
diff saida3.txt saida_esperada3.txt

make clean

rm saida1.txt saida2.txt saida3.txt