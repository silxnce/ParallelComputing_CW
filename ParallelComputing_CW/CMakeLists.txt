cmake_minimum_required(VERSION 3.20)
project(ParallelComputing_CW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_library(core
        src/core/Tokenizer.cpp
        src/core/InvertedIndex.cpp
        src/core/IndexBuilder.cpp
        src/core/ThreadPool.cpp
)

add_library(utils
        src/utils/FileReader.cpp
)

add_library(server
        src/server/Protocol.cpp
        src/server/Server.cpp
)

if (WIN32)
    target_link_libraries(server ws2_32)
endif()

add_executable(ParallelComputing_CW
        src/main.cpp
)

target_link_libraries(ParallelComputing_CW
        core
        utils
        server
)

install(TARGETS ParallelComputing_CW
        RUNTIME DESTINATION bin
)