#include "kvdb.h"
#include "stdio.h"
#include "unistd.h"

int main() {
    kvdb_t db;
    kvdb_open(&db, "good.db");
    kvdb_put(&db, "good", "try");

    kvdb_close(&db);
    return 0;
}