set(OPENGLLIBS ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARY})

if(WIN32)
message(STATUS "Win32: adding glfw32dll and glew32")
set(OPENGLLIBS glfw3dll glew32 ${OPENGLLIBS})
else()
set(OPENGLLIBS glfw ${OPENGLLIBS})
endif()




set(03_srcs  03_animate.cpp)                                # 03_animate
add_executable(03_animate ${03_srcs})                       # 03_animate
target_link_libraries(03_animate common ${OPENGLLIBS})      # 03_animate
SOURCE_GROUP("" FILES ${03_srcs})                           # 03_animate





if(CMAKE_GENERATOR STREQUAL "Xcode")
    set_property(TARGET    03_animate   PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++11)
    set_property(TARGET    03_animate   PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++)
endif(CMAKE_GENERATOR STREQUAL "Xcode")


