#ifndef BASICC_DOWNLOADER_SQLITE_MANAGER_H_
#define BASICC_DOWNLOADER_SQLITE_MANAGER_H_

#include "basics.h"
#include <sqlite3.h>
#include <glib.h>

#define MAX_SQL_LENGTH

typedef void (*DataRetriever)(void *, sqlite3_stmt *);

/**
 * 查询单条结果
 */
typedef struct QuerySingleResult {
  size_t element_size;
  DataRetriever data_retriever;
  void *data;
} QuerySingleResult;

/**
 * 查询多条结果
 */
typedef struct QueryManyResult {
  size_t element_size;
  DataRetriever data_retriever;
  GPtrArray *array;
} QueryManyResult;

/**
 * 开启数据库
 *
 * @param path
 * @return
 */
int OpenDataBase(char *path);

/**
 * @param sql_format update task_info set status = $d where id = $d
 * @param ...  status, id
 * @return
 */
int ExecuteSql(char *sql_format, ...);

/**
 * @param sql_format select * from task_info where id = $d and status = $d
 * @param ...  id, status
 * @return
 */
int QuerySingle(QuerySingleResult *query_single_result, char *sql_format, ...);

int QueryMany(QueryManyResult *query_result, char *sql_format, ...);

sqlite3_int64 GetLastInsertRowId();

/**
 * 关闭数据库
 */
void CloseDataBase();

#endif //BASICC_DOWNLOADER_SQLITE_MANAGER_H_
