list(
        APPEND
        DataBase_SRCS
        DataBase.cpp
)

add_library(DataBase STATIC ${DataBase_SRCS})

target_link_libraries(
        DataBase
        PRIVATE
        seria::seria
        Poco::Net
        Poco::Util
        Poco::JSON
        Poco::MongoDB
)
target_include_directories(DataBase PRIVATE
        ${PROJECT_SOURCE_DIR}/third_party)

target_compile_features(DataBase PRIVATE cxx_std_14
        )
include(${PROJECT_SOURCE_DIR}/cmake/utils.cmake)
if (UNIX)
    target_compile_options(DataBase PRIVATE -Wall -Wextra)
endif()