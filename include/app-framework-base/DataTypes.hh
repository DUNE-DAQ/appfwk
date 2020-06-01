
#ifndef APP_FRAMEWORK_BASE_DATATYPES_HH_
#define APP_FRAMEWORK_BASE_DATATYPES_HH_

#include <string>
#include <vector>

namespace appframework {
struct NonCopyableType
{
  int data;
  explicit NonCopyableType(int d)
    : data(d)
  {}
  NonCopyableType(NonCopyableType const&) = delete;
  NonCopyableType(NonCopyableType&& i) { data = i.data; }
  NonCopyableType& operator=(NonCopyableType const&) = delete;
  NonCopyableType& operator=(NonCopyableType&& i)
  {
    data = i.data;
    return *this;
  }
};

 template<typename T>
std::string
GetClassName(std::string base)
{
  return "UNKNWON" + base;
}
 template<>
std::string
GetClassName<std::vector<int>>(std::string base)
{
  return "VectorInt" + base;
}
 template<>
std::string
GetClassName<std::string>(std::string base)
{
  return "String" + base;
}

template<>
std::string
GetClassName<NonCopyableType>(std::string base)
{
  return "NonCopyableType" + base;
}

}

#endif // APP_FRAMEWORK_BASE_DATATYPES_HH_