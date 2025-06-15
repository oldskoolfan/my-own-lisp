files := parsing.c eval.c mpc/mpc.c
# files := $(wildcard *.c)

build: $(files)
	gcc $(files) -ledit -lm -o app

run: build
	./app

debug: $(files)
	gcc -g $(files) -ledit -lm -o app && gdb ./app
