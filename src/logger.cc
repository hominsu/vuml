//
// Created by Homin Su on 2023/4/28.
//

#include "vuml/logger.h"

#include "vuml/timestamp.h"

namespace vuml::logger {

#ifndef NDEBUG
Level log_level = DEBUG;
#else
Level log_level = INFO;
#endif

FILE* log_file = stdout;

void set_log_level(Level _level) {
  VUML_ASSERT((_level >= TRACE && _level <= FATAL) && "log level out of range");
  log_level = _level;
}

void set_log_file(FILE *_file) {
  VUML_ASSERT(_file != nullptr && "file pointer should not be nullptr");
  log_file = _file;
}

void timestamp(char *_buf, std::size_t _size) {
  clock::to_string(_buf, _size);
}

} // namespace vuml::logger
