cmake_minimum_required(VERSION 2.6)
set(PROJECT_NAME eraser)
project(${PROJECT_NAME})

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/app)

file(GLOB LIB_HEADERS ${CMAKE_SOURCE_DIR}/lib/*.h)
file(GLOB LIB_SOURCES ${CMAKE_SOURCE_DIR}/lib/*.cpp)

file(GLOB APP_HEADERS ${CMAKE_SOURCE_DIR}/app/*.h)
file(GLOB APP_SOURCES ${CMAKE_SOURCE_DIR}/app/*.cpp)

add_library(${PROJECT_NAME} SHARED ${LIB_HEADERS} ${LIB_SOURCES})
add_executable(start-${PROJECT_NAME} ${APP_HEADERS} ${APP_SOURCES})
target_link_libraries(start-${PROJECT_NAME} ${PROJECT_NAME})