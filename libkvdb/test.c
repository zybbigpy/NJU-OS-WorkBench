#include "kvdb.h"
#include "stdio.h"
#include "unistd.h"

int main() {
    kvdb_t db;
    char* value;
    kvdb_open(&db, "good.db");
    kvdb_put(&db, "good", "try");
    kvdb_get(&db, "good");

    printf("key is %s, and the value is %s .\n", "good", value);
    kvdb_close(&db);
    return 0;
}