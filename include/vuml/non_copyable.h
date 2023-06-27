//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_NON_COPYABLE_H_
#define VUML_INCLUDE_VUML_NON_COPYABLE_H_

namespace vuml {

class NonCopyable {
 public:
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;

 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

} // namespace vuml

#endif //VUML_INCLUDE_VUML_NON_COPYABLE_H_
