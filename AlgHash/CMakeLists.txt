cmake_minimum_required(VERSION 3.22)

project(Algorithm_Hash)

set(CMAKE_CXX_STANDARD 20)

set(CORE_SOURCE src/core/SHA-1Service.cpp src/core/MD5Service.cpp src/core/bcryptService.cpp src/core/Argon2Service.cpp)

set(QT_SOURCES src/qt/mainwindow.cpp)

set(SOURCES src/main.cpp ${CORE_SOURCE} ${QT_SOURCES})

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(OpenSSL REQUIRED)
find_package(OpenCL REQUIRED)
# bcrypt
find_library(BCRYPT_LIB bcrypt PATHS /usr/local/lib)
find_path(BCRYPT_INCLUDE bcrypt/bcrypt.h PATHS /usr/local/include)
qt_add_resources(RESOURCES resources.qrc)

add_executable(AlgHash ${SOURCES} ${RESOURCES})
target_include_directories(AlgHash PRIVATE ${BCRYPT_INCLUDE})
target_compile_options(AlgHash PRIVATE -Wall -Werror -Wextra -Wreturn-type)
target_link_libraries(AlgHash PRIVATE Qt6::Core Qt6::Widgets OpenSSL::SSL OpenSSL::Crypto OpenCL::OpenCL argon2 ${BCRYPT_LIB})

find_program(VALGRIND_EXECUTABLE valgrind)
add_custom_target(
    valgrind
    COMMAND ${VALGRIND_EXECUTABLE}
        --tool=memcheck
        --leak-check=full
        --show-leak-kinds=all
        --track-origins=yes
        --log-file=valgrind.log
        $<TARGET_FILE:AlgHash>
    DEPENDS AlgHash
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
)

find_program(CPPCHECK_EXECUTABLE cppcheck)

add_custom_target(
    cppcheck
    COMMAND ${CPPCHECK_EXECUTABLE}
        --enable=all
        --std=c++20
        --inconclusive
        --quiet
        --inline-suppr
        --suppress=missingIncludeSystem
        --suppress=unusedFunction
        ${CMAKE_SOURCE_DIR}/src
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)