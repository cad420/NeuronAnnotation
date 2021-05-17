

find_package(CUDA REQUIRED)
find_package(Threads REQUIRED)


if(WIN32)
    find_package(OpenGL REQUIRED)
    list(
            APPEND
            BlockVolumeRenderer_SRCS
            BlockVolumeRenderer.cpp
            Image.cpp
            BlockData/BlockVolumeManager.cpp
            ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/gl.c
            ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/wgl.c
    )
    list(
        APPEND
        LinesRenderer_SRCS
        LinesRenderer.cpp
        Image.cpp
        ${PROJECT_SOURCE_DIR}/src/Server/AnnotationDS.cpp
        ${PROJECT_SOURCE_DIR}/src/Server/DataBase.cpp
        ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/gl.c
        ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/wgl.c
    )
    set(GLAD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/include)
elseif(UNIX)
    find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
    list(
            APPEND
            BlockVolumeRenderer_SRCS
            BlockVolumeRenderer.cpp
            Image.cpp
            BlockData/BlockVolumeManager.cpp
            ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
    )
    list(
        APPEND
        LinesRenderer_SRCS
        LinesRenderer.cpp
        Image.cpp
        ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
    )
    set(GLAD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/glad/include)
    set(OPENGL_INCLUDE_DIR ${OPENGL_INCLUDE_DIR};${OPENGL_EGL_INCLUDE_DIRS})
    set(OPENGL_LIBRARIES OpenGL::GL;OpenGL::EGL)
endif()

add_library(BlockVolumeRenderer SHARED ${BlockVolumeRenderer_SRCS})
add_library(LinesRenderer SHARED ${LinesRenderer_SRCS})

target_include_directories(
        BlockVolumeRenderer
        PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${CUDA_INCLUDE_DIRS}
        ${GLAD_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party
)

target_include_directories(
        LinesRenderer
        PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${CUDA_INCLUDE_DIRS}
        ${GLAD_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/third_party
)

target_link_libraries(
        LinesRenderer
        PRIVATE
        seria::seria
        Threads::Threads
        spdlog::spdlog
        glm::glm
        voxel_uncompress
        Poco::Net
        Poco::Util
        Poco::JSON
        Poco::MongoDB
        ${CUDA_CUDA_LIBRARY}

        PUBLIC
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS}
)

target_link_libraries(
        BlockVolumeRenderer
        PRIVATE
        seria::seria
        Threads::Threads
        spdlog::spdlog
        glm::glm
        voxel_uncompress
        Poco::Net
        Poco::Util
        Poco::JSON
        ${CUDA_CUDA_LIBRARY}

        PUBLIC
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS}
)

target_compile_features(BlockVolumeRenderer PRIVATE cxx_std_14
        )
target_compile_features(LinesRenderer PRIVATE cxx_std_14
)

include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)
if (UNIX)
    target_compile_options(BlockVolumeRenderer PRIVATE -Wall -Wextra)
    target_compile_options(LineRenderer PRIVATE -Wall -Wextra)
endif()
#embed_shaders(
#        CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Shaders.hpp.in
#        TARGET ${CMAKE_CURRENT_SOURCE_DIR}/Shaders.hpp
#        DIR ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
#)