set(OPENGLLIBS ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARY} ${GLFW_LIBRARIES})

set(common_srcs
                                        # punchout
    common.h                            # punchout
    debug.h                             # punchout
    gls.h                               # punchout
    image.cpp image.h                   # punchout
                                        # punchout
    json.cpp json.h                     # punchout
                                        # punchout
    picojson.h                          # punchout
    scene.cpp scene.h                   # punchout
                                        # punchout
                                        # punchout
    tesselation.cpp tesselation.h       # punchout
    vmath.h                             # punchout
)

set(ext_lodepng_srcs
    ext/lodepng/lodepng.cpp ext/lodepng/lodepng.h
)

set(ext_glew_srcs
    ext/glew/glew.c ext/glew/glew.h ext/glew/glxew.h ext/glew/wglew.h
)


include_directories(ext/glew)

add_library(common ${common_srcs} ${ext_lodepng_srcs} ${ext_glew_srcs})
target_link_libraries(common ${OPENGLLIBS})

SOURCE_GROUP("common" FILES ${common_srcs})
SOURCE_GROUP("ext\\lodepng" FILES ${ext_lodepng_srcs})





#add_executable(png2lrle gfx/png2lrle.cpp)
#target_link_libraries(png2lrle common glfw ${OPENGLLIBS})
#SOURCE_GROUP("" FILES gfx/png2lrle.cpp)



if(CMAKE_GENERATOR STREQUAL "Xcode")
    set_property(TARGET common PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD c++11)
    set_property(TARGET common PROPERTY XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY libc++)
endif(CMAKE_GENERATOR STREQUAL "Xcode")

