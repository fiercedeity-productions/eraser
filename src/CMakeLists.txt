cmake_minimum_required(VERSION 2.6)
set(PROJECT_NAME goodbye)
project(${PROJECT_NAME})

if (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
set(CMAKE_CXX_FLAGS_DEBUG "/MDd -ZI -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
set(CMAKE_CXX_FLAGS_RELEASE "/MD -Ox -fp:fast -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
set(CMAKE_CXX_FLAGS_MINSIZEREL "/MD -Ox -fp:fast -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -Wall -ggdb -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -s -Wall -Ofast -pedantic")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-std=c++17 -s -Wall -Ofast -pedantic")
endif()

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/app)


file(GLOB LIB_HEADERS ${CMAKE_SOURCE_DIR}/lib/*.h)
file(GLOB LIB_SOURCES ${CMAKE_SOURCE_DIR}/lib/*.cpp)

file(GLOB APP_HEADERS ${CMAKE_SOURCE_DIR}/app/*.h)
file(GLOB APP_SOURCES ${CMAKE_SOURCE_DIR}/app/*.cpp)
file(GLOB APP_RESOURCES ${CMAKE_SOURCE_DIR}/app/*.rc)

add_library(${PROJECT_NAME} SHARED ${LIB_HEADERS} ${LIB_SOURCES})
add_executable(start-${PROJECT_NAME} ${APP_HEADERS} ${APP_SOURCES} ${APP_RESOURCES})

target_link_libraries(start-${PROJECT_NAME} ${PROJECT_NAME})
set_target_properties(start-${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if (${CMAKE_SYSTEM_NAME} STREQUAL Linux)
set(wxWidgets_USE_STATIC OFF)
set(wxWidgets_USE_UNICODE ON)
endif()

find_package(wxWidgets 3.1 COMPONENTS core base adv REQUIRED)
include(${wxWidgets_USE_FILE})
target_link_libraries(start-${PROJECT_NAME} ${wxWidgets_LIBRARIES})


if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
target_link_libraries(${PROJECT_NAME} stdc++fs)
target_link_libraries(start-${PROJECT_NAME} stdc++fs)
endif()

if (${CMAKE_C_COMPILER_ID}  STREQUAL MSVC)
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
set_target_properties(start-${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D_CRTDBG_MAP_ALLOC /EHsc")
elseif(${CMAKE_BUILD_TYPE} STREQUAL Release OR ${CMAKE_BUILD_TYPE} STREQUAL MinSizeRel)
set_target_properties(start-${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-DNDEBUG -DwxDEBUG_LEVEL=0 /EHsc" LINK_FLAGS "-SUBSYSTEM:WINDOWS -NXCOMPAT -DYNAMICBASE -LTCG")
endif()
endif()
