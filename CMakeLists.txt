cmake_minimum_required(VERSION 4.1)
project(balls)

set(CMAKE_CXX_STANDARD 23)

add_executable(balls main.cpp
        phys.cpp
        phys.h
        params.cpp
        params.h
        fonts.cpp
        fonts.h
        messages.cpp
        messages.h
        vector2i.h)

find_package(raylib CONFIG REQUIRED)
target_link_libraries(balls PRIVATE raylib)

set(CMAKE_INSTALL_BINDIR balls)
install(TARGETS balls RUNTIME ARCHIVE LIBRARY RUNTIME FRAMEWORK BUNDLE PUBLIC_HEADER RESOURCE)

install(TARGETS balls
        COMPONENT balls
        RUNTIME_DEPENDENCIES
        PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
        POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
        DIRECTORIES $<TARGET_FILE_DIR:balls>)

install(
        DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res"
        TYPE BIN
)
