#include <stdio.h>

void say_hello(int);

int main(int argc, char** argv) {
    //puts("Hello, world!");
    say_hello(5);

    return 0;
}

void say_hello(int num_times) {
    puts("Hello with for loop\n");
    for (int i = 0; i < num_times; i++) {
	puts("Hello, world!\n");
    }
    puts("Hello with while loop\n");
    int i = 0;
    while (i++ < num_times) {
	puts("Hello, world!\n");
    }
}

