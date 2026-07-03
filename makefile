TD4.x: main.o dbg.o dbg_cmd.o dbg_his.o mem.o cpu.o inifile.o crt.o
	gcc -s -O -o TD4.x -liocs -ldos -lbas main.o dbg.o dbg_cmd.o dbg_his.o mem.o cpu.o inifile.o crt.o

main.o: ./src/main.c
	gcc -c -O ./src/main.c -liocs -ldos -lbas 

dbg.o: ./src/dbg/dbg.c
	gcc -c -O ./src/dbg/dbg.c -liocs -ldos -lbas 

dbg_cmd.o: ./src/dbg/dbg_cmd.c
	gcc -c -O ./src/dbg/dbg_cmd.c -liocs -ldos -lbas 

dbg_his.o: ./src/dbg/dbg_his.c
	gcc -c -O ./src/dbg/dbg_his.c -liocs -ldos -lbas 

mem.o: ./src/mem/mem.c
	gcc -c -O ./src/mem/mem.c -liocs -ldos -lbas 

cpu.o: ./src/cpu/cpu.c
	gcc -c -O ./src/cpu/cpu.c -liocs -ldos -lbas 

inifile.o: ./src/lib/inifile.c
	gcc -c -O ./src/lib/inifile.c -liocs -ldos -lbas 

crt.o: ./src/lib/crt.c
	gcc -c -O ./src/lib/crt.c -liocs -ldos -lbas 
