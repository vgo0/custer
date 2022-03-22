cmake_minimum_required(VERSION 3.0.0)
project(custer VERSION 0.1.0)

file(GLOB SRC main.cpp util/*.h util/*.cpp requester/*.h requester/*.cpp)
add_executable(custer ${SRC})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(custer PRIVATE Threads::Threads)
find_package(OpenSSL REQUIRED)
message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(custer PRIVATE ${OPENSSL_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
