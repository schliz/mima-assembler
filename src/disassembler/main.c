//
// Created by Christian Schliz on 16.11.21
//     based on "MIMA-Bytecode Interpreter" by Thorsten Rapp
//

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include "../opcodes.h"

int mem[0x0FFFFF];
int print[256];
int ask[256];

int IAR;
int file_mem_size;

void load_file(char* file) {
    FILE *ptr = fopen(file, "rb");

    int i;

    char mimacheck[10];
    fgets(mimacheck, 5, ptr);

    if ((strcmp(mimacheck, "MIMA") != 0) | (fgetc(ptr) != 0)) {
        printf("Error: missing \"MIMA\\0\" at start of file\n");
        exit(1);
    }

    IAR = fgetc(ptr) * 256 * 256 + fgetc(ptr) * 256 + fgetc(ptr);

    int input_length = fgetc(ptr);
    int output_length = fgetc(ptr);

    for (i = 0; i < input_length; i++) {
        ask[i] = fgetc(ptr) * 256 * 256 + fgetc(ptr) * 256 + fgetc(ptr);
    }

    ask[i] = -1;

    for (i = 0; i < output_length; i++) {
        print[i] = fgetc(ptr) * 256 * 256 + fgetc(ptr) * 256 + fgetc(ptr);
    }

    print[i] = -1;

    // mem[] herstellen
    i = 0;
    int c1, c2, c3;
    while ((c1 = fgetc(ptr)) != EOF) {
        c2 = fgetc(ptr);
        c3 = fgetc(ptr);
        mem[i] = c1 * 256 * 256 + c2 * 256 + c3;
        i++;
    }
    file_mem_size = i;

    fclose(ptr);
}

char* data_to_opcode(int data) {
    int OP1 = data & 0xF00000; // first opcode bit
    OP1 >>= 20;

    int OP2 = data & 0x0F0000; // second opcode bit (used if applies)
    OP2 >>= 16;

    switch (OP1) {
        case (LDC):  return "LDC     ";
        case (LDV):  return "LDV     ";
        case (STV):  return "STV     ";
        case (ADD):  return "ADD     ";
        case (AND):  return "AND     ";
        case (OR):   return "OR      ";
        case (XOR):  return "XOR     ";
        case (EQL):  return "EQL     ";
        case (JMP):  return "JMP     ";
        case (JMN):  return "JMN     ";
        case (LDIV): return "LDIV    ";
        case (STIV): return "STIV    ";
        case (JMS):  return "JMS     ";
        case (JIND): return "JIND    ";
        case (0xF):
            switch (OP2) {
                case (HALT & 0x0F): return "HALT    ";
                case (NOT & 0x0F):  return "NOT     ";
                case (RAR & 0x0F):  return "RAR     ";
                default: return "ERR     ";
            }
        default: return "ERR     ";
    }
}

size_t parse_bound(char* str_bound) {
    if (strcasecmp(str_bound, "IAR") == 0) {
        return IAR;
    } else if (strcasecmp(str_bound, "EOF") == 0) {
        return 0x0FFFFF;
    } else {
        if (strncmp(str_bound, "0x", strlen("0x")) == 0) {
            return strtol(str_bound + 2, (char**) NULL, 16);
        } else if (strncmp(str_bound, "0", strlen("0")) == 0) {
            return strtol(str_bound + 1, (char**) NULL, 8);
        } else {
            return strtol(str_bound, (char**) NULL, 10);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 1 || !argv[1]) {
        printf("Usage: midas <*.mic>\n");
        return 1;
    }

    load_file(argv[1]);

    size_t lower_bound = 0;
    size_t upper_bound = file_mem_size;

    if (argc >= 2 && argv[2]) lower_bound = parse_bound(argv[2]);
    if (argc >= 3 && argv[3]) upper_bound = parse_bound(argv[3]);

    printf("IAR: 0x%X\n", IAR);
    printf("Reading from 0x%zX to 0x%zX\n\n", lower_bound, upper_bound);

    size_t i = lower_bound;
    while (i < file_mem_size && i <= upper_bound) {
        printf("[%5zX] %6X   %s %5X\n", i, mem[i], data_to_opcode(mem[i]), mem[i] >> 4);
        i++;
    }

    return 0;
}