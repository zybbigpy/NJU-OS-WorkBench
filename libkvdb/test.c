#include "kvdb.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

int main() {
    kvdb_t db;
    
    kvdb_open(&db, "good.db");
    kvdb_put(&db, "nice", "os");
    kvdb_put(&db, "os", "a-mazing-course");
    char* value1 = kvdb_get(&db, "good");
    printf("key is %s, and the value is %s.\n", "nice", value1);
    char* value2 = kvdb_get(&db, "jyy");
    printf("key is %s, and the value is %s.\n", "os", value2);
    
    

    free(value1);
    free(value2);
    kvdb_close(&db);
    return 0;
}