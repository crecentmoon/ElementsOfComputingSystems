#include <string.h>
#include "symbol_table.h"

void add_entry(symbol_t *table, int size, char *symbol, int address) {
    // すでに登録されているシンボルの場合はアドレスを更新する
    for (int i = 0; i < size; i++) {
        if (strcmp(table[i].symbol, symbol) == 0) {
            table[i].address = address;
            return;
        }
    }
    // 新しいシンボルを追加する
    strcpy(table[size].symbol, symbol);
    table[size].address = address;
}

int get_address(symbol_t *table, int size, char *symbol) {
    // シンボルテーブルからアドレスを検索する
    for (int i = 0; i < size; i++) {
        if (strcmp(table[i].symbol, symbol) == 0) {
            return table[i].address;
        }
    }
    // シンボルが見つからない場合は、新しいシンボルとして登録する
    add_entry(table, size, symbol, size);
    return size;
}
