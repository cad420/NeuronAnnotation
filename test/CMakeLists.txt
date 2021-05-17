find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
add_executable(test_simple_server test_simple_server.cpp)
add_executable(test_errormessage test_errormessage.cpp)
# add_executable(test_mongodb test_mongodb.cpp AnnotationDS.cpp DataBase.cpp)
# target_include_directories(test_mongodb PRIVATE
#         ${PROJECT_SOURCE_DIR}/third_party)
# target_link_libraries(test_mongodb PRIVATE
#         Poco::Net
#         Poco::Util
#         Poco::MongoDB
#         glm::glm
#         spdlog::spdlog
#         Threads::Threads
#         seria::seria
#         ${OPENGL_LIBRARIES}
#         ${CUDA_CUDA_LIBRARY}
#         ${CMAKE_DL_LIBS}
#         )

target_link_libraries(test_errormessage PRIVATE
        Poco::Net
        Poco::Util
        )

target_link_libraries(test_simple_server PRIVATE
        Poco::Net
        Poco::Util
        Poco::Foundation
        )

add_executable(test_thread_pool test_thread_pool.cpp)
target_include_directories(test_thread_pool
        PRIVATE
        ${PROJECT_SOURCE_DIR}/src/Common)
target_link_libraries(test_thread_pool
        PRIVATE
        glm)
target_compile_features(test_thread_pool
        PRIVATE
        cxx_std_17)

include(${PROJECT_SOURCE_DIR}/third_party/glfw.cmake)
add_executable(test_pick test_pick.cpp
        ${PROJECT_SOURCE_DIR}/third_party/glad/src/glad.c)
target_link_libraries(test_pick
        PRIVATE
        glfw
        glm)
target_include_directories(test_pick
        PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party/glad/include)