CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(LinuxHomework)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

FIND_PACKAGE(Curses)
IF (CURSES_FOUND)
    message(STATUS "FOUND Curses, Link it.")
    message(STATUS ${CURSES_LIBRARIES})
    INCLUDE_DIRECTORIES(${CURSES_INCLUDE_DIR})
    LINK_LIBRARIES(${CURSES_LIBRARIES})
ELSE()
    message(FATAL_ERROR "Lib not found.")
ENDIF()


# define lib source cpp file
file(GLOB_RECURSE CPP_SRC_LIB ${CMAKE_CURRENT_LIST_DIR}/src*/*.lib.cpp)

macro(BATCH_COMPILE_TO_EXECUTABLE SOURCE_FILE)
    string(REGEX REPLACE ".exec.cpp" "" SOURCE_PATH ${SOURCE_FILE})
    string(REGEX REPLACE ${CMAKE_CURRENT_LIST_DIR}/src/ "" FILE_PATH ${SOURCE_PATH})
    add_executable(bin/${FILE_PATH}.bin ${SOURCE_FILE} ${CPP_SRC_LIB})
endmacro()


SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out)

# make out dir
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/out/bin)
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/out/bin)
endif()

file(GLOB_RECURSE FILE_LIST LIST_DIRECTORIES true  ${CMAKE_CURRENT_LIST_DIR}/src*/*)

foreach(CHILD ${FILE_LIST})
   if(IS_DIRECTORY ${CHILD})
        string(REGEX REPLACE ${CMAKE_CURRENT_LIST_DIR}/src "bin" BIN_PATH ${CHILD})
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out/${BIN_PATH})
   endif()
endforeach()

# define exec source file
file(GLOB_RECURSE CPP_SRC_LIST ${CMAKE_CURRENT_LIST_DIR}/src*/*.exec.cpp)

# compile all source file to executable file
foreach(SUBFILE ${CPP_SRC_LIST})
    BATCH_COMPILE_TO_EXECUTABLE(${SUBFILE})
endforeach()