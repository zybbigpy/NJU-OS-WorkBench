#include "kvdb.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

int main() {
    kvdb_t db;
    
    kvdb_open(&db, "good.db");
    kvdb_put(&db, "good", "try");
    char* value= kvdb_get(&db, "good");

    printf("key is %s, and the value is %s .\n", "good", value);
    free(value);
    kvdb_close(&db);
    return 0;
}