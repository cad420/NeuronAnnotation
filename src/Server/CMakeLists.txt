
find_package(CUDA REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

if(WIN32)
    find_package(OpenGL REQUIRED)
    list(
            APPEND
            SERVER_SRCS
            main.cpp
            NeuronAnnotaterApplication.cpp
            RequestHandlerFactory.cpp
            MyHTTPRequestHandler.cpp
            UploadRequestHandler.cpp
            DownloadRequestHandler.cpp
            WebSocketRequestHandler.cpp
            VolumeRenderer.cpp
            Image.cpp
            AnnotationDS.cpp
            ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/gl.c
            ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/src/wgl.c
    )
    set(GLAD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/glad_wgl/include)
elseif(UNIX)
    find_package(OpenGL REQUIRED COMPONENTS OpenGL EGL)
    list(
        SERVER_SRCS
        main.cpp
        NeuronAnnotaterApplication.cpp
        RequestHandlerFactory.cpp
        MyHTTPRequestHandler.cpp
        UploadRequestHandler.cpp
        DownloadRequestHandler.cpp
        WebSocketRequestHandler.cpp
        VolumeRenderer.cpp
        AnnotationDS.cpp
        Image.cpp
        ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c
    )
    set(GLAD_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/glad/include)
    set(OPENGL_INCLUDE_DIR ${OPENGL_INCLUDE_DIR};${OPENGL_EGL_INCLUDE_DIRS})
    set(OPENGL_LIBRARIES OpenGL::GL;OpenGL::EGL)
endif()

add_executable(NeuronAnnotateServer ${SERVER_SRCS})
target_include_directories(NeuronAnnotateServer PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party
        ${GLAD_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src/Algorithm)
target_link_libraries(NeuronAnnotateServer PRIVATE
        Poco::Net
        Poco::Util
        Poco::JSON
        Poco::Foundation
        Poco::MongoDB
        spdlog::spdlog
        Threads::Threads
        glm::glm
        seria::seria
        DataBase
        ${OPENGL_LIBRARIES}
        ${CUDA_CUDA_LIBRARY}
        ${CMAKE_DL_LIBS}
        )
