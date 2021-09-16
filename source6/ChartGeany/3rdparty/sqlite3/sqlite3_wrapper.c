#if defined (__GNUC__) && __GNUC__ > 5
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif /* __GNUC__ */

#define SQLITE_OMIT_PROGRESS_CALLBACK
#define SQLITE_OMIT_DEPRECATED
#define SQLITE_OMIT_TCL_VARIABLE
#define SQLITE_TEMP_STORE					2			/* MEMORY */
#define SQLITE_DEFAULT_AUTOMATIC_INDEX		1
#define SQLITE_DEFAULT_SYNCHRONOUS			3			/* FULL SYNCHRONOUS */
#define SQLITE_DEFAULT_LOCKING_MODE			1 			/* EXCLUSIVE */
#define SQLITE_MAX_COLUMN                   256
#define SQLITE_MAX_SQL_LENGTH               20971520
#define SQLITE_MAX_EXPR_DEPTH               256
#define SQLITE_MAX_COMPOUND_SELECT          128
#define SQLITE_THREADSAFE                   1			/* SERIALIZE */
#define SQLITE_TEMP_STORE                   2
#define SQLITE_DEFAULT_PAGE_SIZE            4096
#define SQLITE_MAX_PAGE_COUNT				4294967291
#define SQLITE_DEFAULT_MMAP_SIZE			1048576		/* 1M */
#define SQLITE_ENABLE_MEMORY_MANAGEMENT
#define SQLITE_ENABLE_MEMSYS5
#define PBKDF2_ITER                         4000
// #define SQLITE_HAS_CODEC 

#include "sqlite3.c"

#if defined (__GNUC__) && __GNUC__ > 5
#pragma GCC diagnostic push 
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif /* __GNUC__ */
