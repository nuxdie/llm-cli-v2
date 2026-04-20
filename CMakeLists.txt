cmake_minimum_required(VERSION 3.19)
project(llm)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(replxx
  GIT_REPOSITORY https://github.com/AmokHuginnsson/replxx.git
  GIT_TAG release-0.0.4
)
FetchContent_MakeAvailable(replxx)

FetchContent_Declare(cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.14.2
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
)
FetchContent_MakeAvailable(json)

set(ENV_FILE "${CMAKE_SOURCE_DIR}/.env")
if(NOT EXISTS "${ENV_FILE}")
  message(FATAL_ERROR ".env file not found at ${ENV_FILE}")
endif()
file(STRINGS "${ENV_FILE}" ENV_LINES)
foreach(line IN LISTS ENV_LINES)
  if(line MATCHES "^OPENROUTER_API_KEY=(.*)$")
    set(OPENROUTER_API_KEY "${CMAKE_MATCH_1}")
  endif()
endforeach()
if(NOT DEFINED OPENROUTER_API_KEY OR OPENROUTER_API_KEY STREQUAL "")
  message(FATAL_ERROR "OPENROUTER_API_KEY not found in .env")
endif()

add_executable(llm llm.cpp)
target_link_libraries(llm PRIVATE
  replxx
  cpr::cpr
  nlohmann_json::nlohmann_json
)
target_compile_definitions(llm PRIVATE
  OPENROUTER_API_KEY="${OPENROUTER_API_KEY}"
)
