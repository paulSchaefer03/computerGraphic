FetchContent_Declare(
    tinyexr
    GIT_REPOSITORY https://github.com/syoyo/tinyexr
    GIT_TAG v1.0.7
)
FetchContent_MakeAvailable(tinyexr)
target_include_directories(${PROJECT_NAME} PRIVATE ${tinyexr_SOURCE_DIR})

# Zlib-Integration prüfen
find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    target_compile_definitions(tinyexr PRIVATE TINYEXR_USE_ZLIB=1)
    target_link_libraries(tinyexr ZLIB::ZLIB)
else()
    message(WARNING "zlib not found; TinyEXR will use miniz.")
    target_compile_definitions(tinyexr PRIVATE TINYEXR_USE_MINIZ=1)
endif()
