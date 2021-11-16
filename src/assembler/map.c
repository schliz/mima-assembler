//
// Created by Christian Schliz on 16.11.21.
//

#include "map.h"

#include <stdlib.h>
#include <string.h>

#define MAP_BY_VAL 0
#define MAP_BY_REF 1

int map_errno = 0;

typedef struct map_item {
    char *key;
    void *val;
    int type;
} MI;

typedef struct map {
    int size;
    MI *items;
} M;

M *map_new() {
    M *map;

    map = malloc(sizeof(M));
    map->size = 0;
    map->items = NULL;

    return map;
}

void map_add(char *key, void *val, M *map) {
    char *new_key;

    new_key = malloc(strlen(key) + 1);
    strcpy(new_key, key);

    if (map->size == 0) {
        map->items = malloc(sizeof(MI));
    } else {
        map->items = realloc(map->items, sizeof(MI) * (map->size + 1));
    }

    (map->items + map->size)->key = new_key;
    (map->items + map->size)->val = val;
    (map->items + map->size++)->type = MAP_BY_VAL;
}

void map_dyn_add(char *key, void *val, M *map) {
    map_add(key, val, map);
    (map->items + map->size - 1)->type = MAP_BY_REF;
}

void *map_get(char *key, M *map) {
    int i;

    for (i = 0; i < map->size; i++) {
        if (strcmp((map->items + i)->key, key) == 0) {
            return (map->items + i)->val;
        }
    }

    return NULL;
}

void map_close(M *map) {
    int i = 0;

    for (; i < map->size; i++) {
        free((map->items + i)->key);

        if ((map->items + i)->type == MAP_BY_REF) {
            free((map->items + i)->val);
        }
    }

    free(map->items);
    free(map);
}