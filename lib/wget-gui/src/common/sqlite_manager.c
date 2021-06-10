#include "common/sqlite_manager.h"
#include "utils/io_utils.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

static sqlite3 *db;

/**
 * 绑定参数与 SQL
 *
 * @param p_stmt
 * @param sql_format
 * @param args
 * @return
 */
static int Sqlite3PrepareWithParameters(sqlite3_stmt **p_stmt, char const *sql_format, va_list args) {
  // 复制 sql_format 到 sql（需要修改）。
  char *sql = strdup(sql_format);
  int sql_length = strlen(sql);

  // 参数类型、个数。
  char parameter_types[20];
  int parameter_type_count = 0;

  //
  for (int i = 0; i < sql_length; ++i) {

    // $c
    if (sql[i] == '$') {
      if (sql[i + 1] != '$') {
        parameter_types[parameter_type_count++] = sql[i + 1];
        // 替换为 ?
        sql[i] = '?';
      }
      sql[i + 1] = ' ';
    }
  }
  PRINTLNF("DB Execute: %s", sql);

  int err = sqlite3_prepare_v2(db, sql, -1, p_stmt, NULL);
  // 释放 sql。
  free(sql);

  // 解析失败，返回。
  if (err != SQLITE_OK) {
    PRINTLNF("prepare failed: %s", sqlite3_errmsg(db));
    return RESULT_FAILURE;
  }

  for (int i = 0; i < parameter_type_count; ++i) {
    switch (parameter_types[i]) {
      // 字符
      case 'c': {
        char parameter_holder[2] = {va_arg(args, int)};
        sqlite3_bind_text(*p_stmt, i + 1, parameter_holder, -1, NULL);
        break;
      }
        // 字符串
      case 's': {
        sqlite3_bind_text(*p_stmt, i + 1, va_arg(args, char *), -1, NULL);
        break;
      }
        // 整型
      case 'd': {
        sqlite3_bind_int64(*p_stmt, i + 1, va_arg(args, int));
        break;
      }
        // 浮点型
      case 'f': {
        sqlite3_bind_double(*p_stmt, i + 1, va_arg(args, double));
        break;
      }
        // 不支持的类型。
      default: {
        PRINTLNF("Unsupported parameter type: %c.", parameter_types[i]);
        exit(-1);
      }
    }
  }
  return RESULT_OK;
}

/**
 * 开启数据库
 *
 * @param path
 * @return
 */
int OpenDataBase(char *path) {
  if (sqlite3_open(path, &db) != SQLITE_OK) {
    PRINTLNF("Can't open database: %s", sqlite3_errmsg(db));
    CloseDataBase();
    return RESULT_FAILURE;
  }
  return RESULT_OK;
}

/**
 * 执行 SQL
 *
 * @param sql_format
 * @param ...
 * @return
 */
int ExecuteSql(char *sql_format, ...) {
  if (!db) {
    return RESULT_FAILURE;
  }

  int result;
  sqlite3_stmt *stmt;

  // 解析变长参数，并绑定 SQL。
  va_list args;
  va_start(args, sql_format);
  if (Sqlite3PrepareWithParameters(&stmt, sql_format, args) == RESULT_OK) {

    // 执行 SQL，返回提示。
    if (sqlite3_step(stmt) != SQLITE_DONE) {
      PRINTLNF("execution failed: %s", sqlite3_errmsg(db));
      result = RESULT_FAILURE;
    } else {
      result = RESULT_OK;
    }

    // 释放 statement。
    sqlite3_finalize(stmt);
  } else {
    result = RESULT_FAILURE;
  }
  va_end(args);
  return result;
}

/**
 * 执行查询 SQL
 *
 * @param query_single_result
 * @param sql_format
 * @param ...
 * @return
 */
int QuerySingle(QuerySingleResult *query_single_result, char *sql_format, ...) {
  if (db) {
    int result;
    sqlite3_stmt *stmt;

    va_list args;
    va_start(args, sql_format);
    if (Sqlite3PrepareWithParameters(&stmt, sql_format, args) == RESULT_OK) {

      // 返回数据行。
      if (sqlite3_step(stmt) == SQLITE_ROW) {
        query_single_result->data = malloc(query_single_result->element_size);
        query_single_result->data_retriever(query_single_result->data, stmt);
        result = RESULT_OK;
      } else {
        PRINTLNF("execution failed: %s", sqlite3_errmsg(db));
        result = RESULT_FAILURE;
      }
      sqlite3_finalize(stmt);
    } else {
      result = RESULT_FAILURE;
    }
    va_end(args);

    return result;
  } else {
    return RESULT_FAILURE;
  }
}

/**
 * 查询多条记录
 *
 * @param query_result
 * @param sql_format
 * @param ...
 * @return
 */
int QueryMany(QueryManyResult *query_result, char *sql_format, ...) {
  if (db) {
    int result;
    sqlite3_stmt *stmt;
    va_list args;
    va_start(args, sql_format);
    if (Sqlite3PrepareWithParameters(&stmt, sql_format, args) == RESULT_OK) {
      if (!query_result->array) {
        query_result->array = g_ptr_array_new_with_free_func(free);
      }

      while (1) {
        int sql_result = sqlite3_step(stmt);
        if (sql_result == SQLITE_ROW) {
          void *data = malloc(query_result->element_size);
          g_ptr_array_add(query_result->array, data);
          query_result->data_retriever(data, stmt);
        } else if (sql_result == SQLITE_ERROR) {
          PRINTLNF("execution failed: %s", sqlite3_errmsg(db));
          result = RESULT_FAILURE;
          break;
        } else {
          result = RESULT_OK;
          break;
        }
      }
      sqlite3_finalize(stmt);
    } else {
      result = RESULT_FAILURE;
    }
    va_end(args);
    return result;
  }
  return RESULT_FAILURE;
}

/**
 * 最近插入记录的行号
 *
 * @return
 */
sqlite3_int64 GetLastInsertRowId() {
  return sqlite3_last_insert_rowid(db);
}

/**
 * 关闭数据库
 */
void CloseDataBase() {
  if (db) {
    sqlite3_close(db);
    db = NULL;
  }
}


