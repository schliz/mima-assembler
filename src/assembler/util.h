//
// Created by Christian Schliz on 16.11.21.
//

#ifndef MIMA_ASSEMBLER_UTIL_H
#define MIMA_ASSEMBLER_UTIL_H

#include "map.h"

int fcountln(char* filename);
char *trim(char *str);
unsigned int parse_int(char* str_bound);
unsigned int address(char* string, struct map* memory_map);
char *str_replace(char *orig, char *rep, char *with);

#endif //MIMA_ASSEMBLER_UTIL_H
