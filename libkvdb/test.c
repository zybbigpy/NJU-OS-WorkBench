#include "kvdb.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

int main() {
    kvdb_t db;
    
    kvdb_open(&db, "good.db");
    kvdb_put(&db, "good", "try");
    kvdb_put(&db, "jyy", "a-good-guy");
    char* value1 = kvdb_get(&db, "good");
    char* value2 = kvdb_get(&db, "jyy");

    printf("key is %s, and the value is %s.\n", "good", value1);
    printf("key is %s, and the value is %s.\n", "jyy", value2);

    free(value1);
    free(value2);
    kvdb_close(&db);
    return 0;
}