cmake_minimum_required(VERSION 3.19)
project(llm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
  replxx
  GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git
  GIT_TAG release-0.0.4
)
FetchContent_MakeAvailable(replxx)

add_executable(llm llm.cpp)
target_link_libraries(llm PRIVATE replxx)
