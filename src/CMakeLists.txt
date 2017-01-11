cmake_minimum_required(VERSION 2.8)
include_directories (${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/src
                     ${CMAKE_SOURCE_DIR}/src/libshared/src
                     ${RAT_INCLUDE_DIRS})

set (rat_SRCS rat.cpp
              Packrat.cpp Packrat.h
              PEG.cpp     PEG.h)

set (libshared_SRCS libshared/src/Args.cpp          libshared/src/Args.h
                    libshared/src/Datetime.cpp      libshared/src/Datetime.h
                    libshared/src/Duration.cpp      libshared/src/Duration.h
                    libshared/src/FS.cpp            libshared/src/FS.h
                    libshared/src/Lexer.cpp         libshared/src/Lexer.h
                    libshared/src/Pig.cpp           libshared/src/Pig.h
                    libshared/src/Timer.cpp         libshared/src/Timer.h
                    libshared/src/Tree.cpp          libshared/src/Tree.h
                    libshared/src/format.cpp        libshared/src/format.h
                    libshared/src/shared.cpp        libshared/src/shared.h
                    libshared/src/unicode.cpp       libshared/src/unicode.h
                    libshared/src/utf8.cpp          libshared/src/utf8.h
                    libshared/src/wcwidth6.cpp)

add_library (rat      STATIC ${rat_SRCS})
add_library (libshared STATIC ${libshared_SRCS})
add_executable (rat_executable rat.cpp)

target_link_libraries (rat_executable rat libshared ${RAT_LIBRARIES})

set_property (TARGET rat_executable PROPERTY OUTPUT_NAME "rat")

install (TARGETS rat_executable DESTINATION bin)

