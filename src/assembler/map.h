//
// Created by Christian Schliz on 16.11.21.
//

#ifndef MIMA_ASSEMBLER_MAP_H
#define MIMA_ASSEMBLER_MAP_H

struct map;

struct map* map_new();
void map_add(char* key, void* val, struct map* map);
void map_dyn_add(char* key, void* val, struct map* map);
void* map_get(char* key, struct map* map);
void map_close(struct map* map);

#endif //MIMA_ASSEMBLER_MAP_H
