cmake_minimum_required(VERSION 3.20)
project(GraphColoring)
set(CMAKE_CXX_STANDARD 20)
# DEBUG 用于打印执行信息，AUTO_SAVE用于每进行100次generation保存结果
# add_definitions(-DDEBUG -DAUTO_SAVE)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
file(GLOB HEAD ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(GraphColoring ${SRC} ${HEAD})
