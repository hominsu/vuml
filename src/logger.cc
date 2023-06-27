//
// Created by Homin Su on 2023/4/28.
//

#include "vuml/logger.h"

#include <unistd.h>

#include "vuml/timestamp.h"

namespace vuml::logger {

#ifndef NDEBUG
Level log_level = DEBUG;
#else
Level log_level = INFO;
#endif

int log_fd = STDOUT_FILENO;

void set_log_level(Level _level) {
  VUML_ASSERT((_level >= TRACE && _level <= FATAL) && "log level out of range");
  log_level = _level;
}

void set_log_fd(int _fd) {
  VUML_ASSERT(_fd > 0 && "file descriptor should greater than 0");
  log_fd = _fd;
}

void timestamp(char *_buf, std::size_t _size) {
  clock::to_string(_buf, _size);
}

} // namespace vuml::logger
