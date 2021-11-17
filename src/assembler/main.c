//
// Created by Christian Schliz on 16.11.21.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include "util.h"
#include "map.h"
#include "../opcodes.h"

#define WHITESPACES " \t\n\v\f\r"
#define IAR_DEFAULT_VALUE 10
#define PROMPT_LENGTH 256
#define OUTPUT_LENGTH 256

int prompts;
unsigned int prompt[PROMPT_LENGTH]; // prompts, ends with -1
int outputs;
unsigned int output[OUTPUT_LENGTH]; // outputs, ends with -1

int line_count;
int next_address;
int text_address;

struct map* m_def; // replacement definitions
struct map* m_mem; // memory map

char* output_name;
FILE* fp_out;

//
// PREPROCESSOR PART
//

void add_def(char* argument) {
    fprintf(stdout, "WARNING: definitions are currently not supported!");
    char* key = strtok(argument, WHITESPACES);
    char* value = strtok(argument, "\0");
    map_add(key, value, m_def);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
void add_mem_many(char* argument) {
    int count = 1;
    for (char* s = argument; s[0] != 0; ++s) if (s[0] == ',') count++;

    char* keys[count];
    char* p = strtok(argument, ",");
    int i = 0;

    while (p != NULL) {
        keys[i++] = trim(p);
        p = strtok(NULL, ",");
    }

    for (int j = 0; j < count; ++j) {
        map_add(keys[j], (unsigned int*) next_address++, m_mem);
    }
}
#pragma clang diagnostic pop

void add_prompt(unsigned int address) {
    for (int i = 0; i < PROMPT_LENGTH - 1; ++i) {
        if (prompt[i] == -1) {
            prompt[i] = address;
            prompt[i + 1] = -1;
            prompts++;
            return;
        }
    }

    fprintf(stderr, "Error: cannot define more than %d prompts", PROMPT_LENGTH - 1);
    exit(1);
}

void add_output(unsigned int address) {
    for (int i = 0; i < OUTPUT_LENGTH - 1; ++i) {
        if (output[i] == -1) {
            output[i] = address;
            output[i + 1] = -1;
            outputs++;
            return;
        }
    }

    fprintf(stderr, "Error: cannot define more than %d outputs", OUTPUT_LENGTH - 1);
    exit(1);
}

void parse_data_line(char* line) {
    line = trim(line);
    char* token = strtok(line, WHITESPACES);
    char* argument = strtok(NULL, "\0");

    if (!token || !argument) return;

    if (strcasecmp(token, "mem") == 0) {
        add_mem_many(argument);
    } else if (strcasecmp(token, "ask") == 0) {
        add_prompt(address(argument, m_mem));
    } else if (strcasecmp(token, "out") == 0) {
        add_output(address(argument, m_mem));
    } else if (strcasecmp(token, "def") == 0) {
        add_def(argument);
    } else {
        fprintf(stderr, "Error: invalid token %s in data segment!", token);
    }
}

void parse_data_segment(FILE* file) {
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        line = strtok(line, ";");

        if (line == NULL) {
            fprintf(stderr, "Error: begin of text segment not found before EOF");
            fclose(file);
            exit(1);
        } else if (strncmp(line, ".text", strlen(".text")) == 0) {
            return;
        } else {
            parse_data_line(line);
        }
    }
}

//
// ASSEMBLER PART
//

void fail() {
    fflush(fp_out);
    fclose(fp_out);
    remove(output_name);
}

void write_word(int word) {
    char c1 = (word & 0xFF0000) >> 16;
    fwrite(&c1, 1, sizeof(c1), fp_out);
    char c2 = (word & 0xFF00) >> 8;
    fwrite(&c2, 1, sizeof(c2), fp_out);
    char c3 = word & 0xFF;
    fwrite(&c3, 1, sizeof(c3), fp_out);
}

int word_from_instruction(char* instruction) {
    if (strcmp(instruction, "LDC") == 0) return LDC << 20;
    if (strcmp(instruction, "LDV") == 0) return LDV << 20;
    if (strcmp(instruction, "STV") == 0) return STV << 20;
    if (strcmp(instruction, "ADD") == 0) return ADD << 20;
    if (strcmp(instruction, "AND") == 0) return AND << 20;
    if (strcmp(instruction, "OR") == 0) return OR << 20;
    if (strcmp(instruction, "XOR") == 0) return XOR << 20;
    if (strcmp(instruction, "EQL") == 0) return EQL << 20;
    if (strcmp(instruction, "JMP") == 0) return JMP << 20;
    if (strcmp(instruction, "JMN") == 0) return JMN << 20;
    if (strcmp(instruction, "LDIV") == 0) return LDIV << 20;
    if (strcmp(instruction, "STIV") == 0) return STIV << 20;
    if (strcmp(instruction, "JMS") == 0) return JMS << 20;
    if (strcmp(instruction, "JIND") == 0) return JIND << 20;

    if (strcmp(instruction, "HALT") == 0) return HALT << 16;
    if (strcmp(instruction, "NOT") == 0) return NOT << 16;
    if (strcmp(instruction, "RAR") == 0) return RAR << 16;

    fprintf(stderr, "Error at instruction %d: invalid token \"%s\"!\n", text_address, instruction);
    exit(1);
}

void check_pos_definitions(char* line) {
    if (strncmp(line, ":", strlen(":")) == 0) {
        fprintf(stderr, "Error in instruction %d: Line cannot begin with a colon. Always need instruction to jump to!\n", text_address);
        exit(1);
    }

    char _line[strlen(line)];
    strcpy(_line, line);
    char* beforeColon = strtok(_line, ":");

    if (strcmp(beforeColon, line) == 0) {
        return; // there are no colons in the line
    }

    char* tag = strtok(NULL, WHITESPACES);

    if (tag == NULL) {
        printf("Warning: colon without tag in line: %s", line);
    } else {
        map_add(tag, IAR_DEFAULT_VALUE + text_address, m_mem);
    }
}

void parse_text_line(char* line) {
    char* instruction = strtok(line, WHITESPACES);
    char* argument = strtok(NULL, "\0");
    int word = word_from_instruction(instruction);
    unsigned int arg = 0;

    // if the argument takes parameters
    if (word < 0xF00000) {
        arg = address(argument, m_mem);
        word += (arg & 0x0FFFFF);
    }

    write_word(word);
    text_address++;
}

void parse_text_segment(FILE* file) {
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        char* _line = malloc(len * sizeof(char));

        strcpy(_line, line);
        _line = strtok(_line, ";");
        _line = trim(_line);

        if (strcmp(_line, "") != 0) {
            parse_text_line(_line);
        }
    }
}

void write_file_header() {
    fwrite("MIMA\0", 5, sizeof(char), fp_out);
    int IAR = IAR_DEFAULT_VALUE;
    write_word(IAR);
    fwrite(&prompts, 1, sizeof(char), fp_out);
    fwrite(&outputs, 1, sizeof(char), fp_out);

    for (int i = 0; i < PROMPT_LENGTH - 1 && prompt[i] != -1; ++i) write_word(prompt[i]);
    for (int i = 0; i < OUTPUT_LENGTH - 1 && output[i] != -1; ++i) write_word(output[i]);

    for (int i = 0; i < IAR_DEFAULT_VALUE; ++i) {
        write_word(0x0);
    }
}

void jump_to_text_segment(FILE* file) {
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        char* _line = malloc(strlen(line) * sizeof(char));

        strcpy(_line, line);
        _line = strtok(_line, ";");
        _line = trim(_line);

        if (_line == NULL) {
            fprintf(stderr, "Error: begin of text segment not found before EOF");
            fclose(file);
            exit(1);
        } else if (strncmp(_line, ".text", strlen(".text")) == 0) {
            break;
        }
    }
}

void preprocess_text_segment(FILE* file) {
    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        char* _line = malloc(strlen(line) * sizeof(char));

        strcpy(_line, line);
        _line = strtok(_line, ";");
        _line = trim(_line);

        if (strcmp(_line, "") != 0) {
            check_pos_definitions(line);
            text_address++;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 1 || !argv[1]) {
        printf("Usage: miasm <*.miasm>\n");
        return 1;
    }

    output_name = strcat(str_replace(argv[1], ".miasm", ""), ".mic");

    if (argc >= 2 && argv[2]) {
        output_name = argv[2];
        if (access(output_name, F_OK) == 0) {
            fprintf(stderr, "Error: the output file %s already exists!", output_name);
            return 1;
        } else if (access(output_name, W_OK) != 0) {
            fprintf(stderr, "Error: the output path %s cannot be written to!", output_name);
            return 1;
        }
    }

    line_count = fcountln(argv[1]);
    next_address = IAR_DEFAULT_VALUE + line_count + 10;

    FILE *file = fopen(argv[1], "rb");

    m_def = map_new();
    m_mem = map_new();

    jump_to_text_segment(file);
    preprocess_text_segment(file);
    rewind(file);
    text_address = 0;

    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        line = strtok(line, ";");

        if (line == NULL) {
            fprintf(stderr, "Error: begin of data segment not found before EOF");
            fclose(file);
            return 1;
        } else if (strncmp(line, ".data", strlen(".data")) == 0) {
            break;
        }
    }

    prompts = 0;
    prompt[0] = -1;
    outputs = 0;
    output[0] = -1;

    // preprocessor
    parse_data_segment(file);

    // assembler
    fp_out = fopen(output_name, "wb");
    write_file_header();
    parse_text_segment(file);

    fclose(file);
    fflush(fp_out);
    fclose(fp_out);

    return 0;
}
