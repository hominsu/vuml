//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_LOGGER_H_
#define VUML_INCLUDE_VUML_LOGGER_H_

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <sstream>

#include "non_copyable.h"
#include "vuml/vuml.h"

#define LOG_LEVEL(VUML) \
  VUML(TRACE)           \
  VUML(DEBUG)           \
  VUML(INFO)            \
  VUML(WARN)            \
  VUML(ERROR)           \
  VUML(FATAL)           \
  //

namespace vuml::logger {

enum Level {
#define LOG_NAME(_name) _name,
  LOG_LEVEL(LOG_NAME)
#undef LOG_NAME
};

extern Level log_level;
extern FILE *log_file;

void set_log_level(Level _level);
void set_log_file(FILE *_file);

void timestamp(char *_buf, ::std::size_t size);

inline const char *level_str(Level _level) {
  const static char *level_str_table[] = {
#define LOG_STR(_name) #_name,
      LOG_LEVEL(LOG_STR)
#undef LOG_STR
  };

  VUML_ASSERT(_level >= 0 && _level < VUML_LENGTH(level_str_table));
  return level_str_table[_level];
}

} // namespace logger

#undef LOG_LEVEL

#define LOG_BASE(_log_file_, _level_, _file_, _line_, _abort_, _formatter_, ...) \
  do {                                                                           \
    char buf[32]{0};                                                             \
    vuml::logger::timestamp(buf, sizeof(buf));                                   \
    int err = fprintf((_log_file_), "[%s] [%s] "#_formatter_" - %s:%d\n", vuml::logger::level_str((_level_)), buf, ##__VA_ARGS__, strrchr((_file_), '/') + 1, (_line_)); \
    if (err == -1) {                                                             \
      fprintf(stderr, "log failed");                                             \
    }                                                                            \
    if ((_abort_)) {                                                             \
      abort();                                                                   \
    }                                                                            \
  } while (0)                                                                    \
  //

#define LOG_SYS(_log_file_, _file_, _line_, _abort_, _formatter_, ...) \
  do {                                                                 \
    char buf[32]{0};                                                   \
    vuml::logger::timestamp(buf, sizeof(buf));                         \
    fprintf((_log_file_), "[%s] [%s] "#_formatter_": %s - %s:%d\n", (_abort_) ? "SYSFA" : "SYSER", buf, ##__VA_ARGS__, strerror(errno), strrchr((_file_), '/') + 1, (_line_)); \
    if ((_abort_)) {                                                   \
      abort();                                                         \
    }                                                                  \
  } while (0)                                                          \
  //

#define TRACE(_formatter_, ...) \
  do {                          \
    if (vuml::logger::log_level <= vuml::logger::Level::TRACE) { \
      LOG_BASE(vuml::logger::log_file, vuml::logger::Level::TRACE, __FILE__, __LINE__, false, _formatter_, ##__VA_ARGS__); \
    }                           \
  } while (0)                   \
  //

#define DEBUG(_formatter_, ...) \
  do {                          \
    if (vuml::logger::log_level <= vuml::logger::Level::DEBUG) { \
      LOG_BASE(vuml::logger::log_file, vuml::logger::Level::DEBUG, __FILE__, __LINE__, false, _formatter_, ##__VA_ARGS__); \
    }                           \
  } while (0)                   \
  //

#define INFO(_formatter_, ...) \
  do {                         \
    if (vuml::logger::log_level <= vuml::logger::Level::INFO) { \
      LOG_BASE(vuml::logger::log_file, vuml::logger::Level::INFO, __FILE__, __LINE__, false, _formatter_, ##__VA_ARGS__); \
    }                          \
  } while (0)                  \
  //

#define WARN(_formatter_, ...) \
  do {                         \
    if (vuml::logger::log_level <= vuml::logger::Level::WARN) { \
      LOG_BASE(vuml::logger::log_file, vuml::logger::Level::WARN, __FILE__, __LINE__, false, _formatter_, ##__VA_ARGS__); \
    }                          \
  } while (0)                  \
  //

#define ERROR(_formatter_, ...) LOG_BASE(vuml::logger::log_file, vuml::logger::Level::ERROR, __FILE__, __LINE__, false, _formatter_, ##__VA_ARGS__)

#define FATAL(_formatter_, ...) LOG_BASE(vuml::logger::log_file, vuml::logger::Level::FATAL, __FILE__, __LINE__, true, _formatter_, ##__VA_ARGS__)

#define SYSERR(_formatter_, ...) LOG_SYS(vuml::logger::log_file, __FILE__, __LINE__, false, _formatter_, ##__VA_ARGS__)

#define SYSFATAL(_formatter_, ...) LOG_SYS(vuml::logger::log_file, __FILE__, __LINE__, true, _formatter_, ##__VA_ARGS__)

#endif //VUML_INCLUDE_VUML_LOGGER_H_
