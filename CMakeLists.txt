cmake_minimum_required(VERSION 2.8.11)

add_subdirectory(Facechat)

project(Facebot)

include_directories(
    /home/utilisateur/Documents/Projects/Facebot/
    /home/utilisateur/Documents/Projects/Facebot/
    /home/utilisateur/Documents/Projects/Facebot/Facechat/extlib/cpr/include

)

# Linker options
set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS}  ")

# Library path
#set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} -L\".\" -L\"./Facechat/bin\" -L\"./Facechat/bin/lib\" ")

# Define the CXX sources
set ( CXX_SRCS
    main.cpp
    Utility.cpp
    IRClient.cpp
    IDPool.cpp
    Facebot.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -O0 -Wall -std=c++11")

add_executable(Facebot ${CXX_SRCS} ${C_SRCS})

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC ${PROJECT_NAME} "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${PROJECT_NAME} "${CMAKE_THREAD_LIBS_INIT}")
endif()

target_link_libraries(Facebot
    Facechat
    cpr
    curl
    sfml-network
    sfml-system
)

