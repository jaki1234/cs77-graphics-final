cmake_minimum_required(VERSION 2.8.3)

## project
project(03_animate)

option(USE_CLANG "Use clang compiler (if available)" OFF)

MESSAGE( STATUS "CMAKE_GENERATOR: " ${CMAKE_GENERATOR} )
MESSAGE( STATUS "APPLE: " ${APPLE} )
MESSAGE( STATUS "WIN32: " ${WIN32} )
MESSAGE( STATUS "UNIX:  " ${UNIX} )

#########################################################
## compiler,linker settings and output directories

## default setting
if(NOT CMAKE_GENERATOR)
    SET(CMAKE_GENERATOR "Unix Makefiles" FORCE)
endif()

if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    if(${APPLE})
        if(${USE_CLANG})
            # clang
            #set(CMAKE_C_COMPILER /usr/bin/clang)
            #set(CMAKE_CXX_COMPILER /usr/bin/clang++)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
        else()
            # mac-ports gcc
            #set(CMAKE_C_COMPILER /opt/local/bin/gcc-mp-4.8)
            #set(CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-4.8)
        endif(${USE_CLANG})
        MESSAGE(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER})
        MESSAGE(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
    else()
        #set(CMAKE_CXX_COMPILER /usr/bin/g++-4.7)
        #set(CMAKE_C_COMPILER /usr/bin/g++-4.7)
    endif(${APPLE})
    
    set(BIN_DIR_SUFFIX mk)
    set(MAKEFILES ON)
    set(CMAKE_VERBOSE_MAKEFILE false)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lGL -lGLU -lGLEW")
    
    if(${APPLE})
    else()
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lX11 -lXrandr")
    endif(${APPLE})
    
    find_library(OpenGL_LIBRARY OpenGL)
    
elseif(CMAKE_GENERATOR STREQUAL "Xcode")
    set(BIN_DIR_SUFFIX xcode)
    set(XCODE ON)
    set_property(TARGET PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++11)
    
elseif(CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    MESSAGE(STATUS "Configuring for MinGW build")
    set(BIN_DIR_SUFFIX mingw)
    set(MAKEFILES ON)
    set(CMAKE_VERBOSE_MAKEFILE OFF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLEW_STATIC")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLEW_BUILD")
    
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lstdc++ -lgcc   -lmingw32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lglfw3dll  -lglew32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lGLU    -lglu32 -lGL     -lopengl32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lgdi32  -lwinmm -luser32 -lkernel32")
    #-L${PROJECT_SOURCE_DIR}/ext/w32/lib/
    
    # see http://www.cmake.org/Wiki/CMake_Cross_Compiling#Using_executables_in_the_build_created_during_the_build
    file(COPY ${PROJECT_SOURCE_DIR}/ext/lib/glew32.dll ${PROJECT_SOURCE_DIR}/ext/lib/glfw3.dll DESTINATION ${PROJECT_SOURCE_DIR}/bin/${BIN_DIR_SUFFIX})
    
    find_library(OpenGL_LIBRARY OpenGL)
    
    include_directories(${PROJECT_SOURCE_DIR}/ext/w32/include)
    include_directories(${PROJECT_SOURCE_DIR}/ext/w32/include/GLFW)
    include_directories(${PROJECT_SOURCE_DIR}/ext/w32/include/GL)

    #add_subdirectory(${PROJECT_SOURCE_DIR}/ext/w32/lib)
    #target_link_libraries(graphics glew32)
    #target_link_libraries(graphics glfw3)

endif(CMAKE_GENERATOR STREQUAL "Unix Makefiles")

message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: " ${CMAKE_SHARED_LINKER_FLAGS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${BIN_DIR_SUFFIX})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${BIN_DIR_SUFFIX})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${BIN_DIR_SUFFIX})

## include directories
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/apps)
include_directories(${PROJECT_SOURCE_DIR}/src/common)

## homebrew (OSX only)
#include_directories(/homebrew/include)
#link_directories(/homebrew/lib)

## opengl
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} )
MESSAGE( STATUS "OPENGL_INCLUDE_DIRS: " ${OPENGL_INCLUDE_DIRS} )

## glew
#find_package(GLEW REQUIRED)
#include_directories( ${GLEW_INCLUDE_DIRS} )
#MESSAGE( STATUS "GLEW_FOUND: " ${GLEW_FOUND} )
#MESSAGE( STATUS "GLEW_INCLUDE_DIRS: " ${GLEW_INCLUDE_DIRS} )
if(WIN32)
else()
    include_directories(${PROJECT_SOURCE_DIR}/src/common/ext/glew)
endif()

## glfw library without examples, tests
if(WIN32)
else()
    option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
    option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
    option(GLFW_INSTALL "Generate installation target" OFF)
    option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
    option(GLFW_BUILD_DOCUMENTS "Build docs" OFF)
    add_subdirectory(${PROJECT_SOURCE_DIR}/src/common/ext/glfw)
    include_directories(${PROJECT_SOURCE_DIR}/src/common/ext/glfw/include)
endif()

## lodepng
include_directories(${PROJECT_SOURCE_DIR}/src/common/ext/lodepng)


## subdirectories
add_subdirectory(src)


