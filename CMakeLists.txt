
cmake_minimum_required(VERSION 3.15)
project(mute)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

if (MSVC)
  set(COMPILE_OPTIONS
    /Wall
    /wd4820
    /wd4456
    /wd5045
    /wd4061
    /D_CRT_SECURE_NO_WARNINGS)
else()
  set(COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wpedantic
    -Wno-strict-prototypes
    -Wno-deprecated-declarations)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(BUILD_TYPE _DEBUG)
else()
  set(BUILD_TYPE NDEBUG)
endif()

add_executable(${PROJECT_NAME}
  ${CMAKE_CURRENT_LIST_DIR}/src/main.c
  ${CMAKE_CURRENT_LIST_DIR}/src/token.c
  ${CMAKE_CURRENT_LIST_DIR}/src/parser.c
  ${CMAKE_CURRENT_LIST_DIR}/src/utils.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${BUILD_TYPE})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS})
