//
// Created by Christian Schliz on 16.11.21.
//

#include "util.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SIZE 1024

#pragma clang diagnostic ignored "-Wparentheses"
#pragma clang diagnostic ignored "-Wvoid-pointer-to-int-cast"
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"

int fcountln(char* filename) {
    FILE *in_file;
    char buffer[SIZE + 1], lastchar = '\n';
    size_t bytes;
    int lines = 0;

    if (NULL == (in_file = fopen(filename, "r"))) {
        perror(filename);
        return EXIT_FAILURE;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer) - 1, in_file))) {
        lastchar = buffer[bytes - 1];
        for (char *c = buffer; (c = memchr(c, '\n', bytes - (c - buffer))); c++) {
            lines++;
        }
    }
    if (lastchar != '\n') {
        lines++;  /* Count the last line even if it lacks a newline */
    }
    if (ferror(in_file)) {
        perror(filename);
        fclose(in_file);
        return EXIT_FAILURE;
    }

    fclose(in_file);

    return lines;
}

char *trim(char *str) {
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

unsigned int parse_int(char* str_bound) {
    if (strncmp(str_bound, "0x", strlen("0x")) == 0) {
        return strtol(str_bound + 2, (char**) NULL, 16);
    } else if (strncmp(str_bound, "0", strlen("0")) == 0) {
        return strtol(str_bound + 1, (char**) NULL, 8);
    } else {
        return strtol(str_bound, (char**) NULL, 10);
    }
}

unsigned int address(char* string, struct map* memory_map) {
    unsigned int map_value = (unsigned int) map_get(string, memory_map);
    if (map_value == 0) {
        return parse_int(string);
    } else {
        return map_value;
    }
}

char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
