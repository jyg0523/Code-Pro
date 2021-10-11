cmake_minimum_required(VERSION 3.9)
project(NewProCon)

set(CMAKE_CXX_STANDARD 11)

#导入需要使用的boost库，如regex filesystem thread
#find_package命令用于搜索并加载外部工程，其隐含的变量用于标示是否搜索到所需的工程
find_package(Boost REQUIRED COMPONENTS regex thread system )
if (NOT Boost_FOUND)
    message("Not found Boost")
endif ()
#导入头文件
include_directories(${Boost_INCLUDE_DIRS})
message("${Boost_INCLUDE_DIRS}")
#导入库文件
link_libraries(${Boost_LIBRARIES})
message("${Boost_LIBRARIES}")

add_executable(NewProCon main.cpp)
target_link_libraries(NewProCon pthread)