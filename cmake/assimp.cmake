set(ASSIMP_BUILD_TESTS                    OFF)
set(ASSIMP_INSTALL                        OFF)
set(ASSIMP_BUILD_ASSIMP_VIEW              OFF)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER             ON)
# enable other importers here via ASSIMP_BUILD_XXX_IMPORTER

FetchContent_Declare(assimp
    GIT_REPOSITORY "https://github.com/assimp/assimp.git"
    GIT_TAG "v5.4.3"
    GIT_SHALLOW ON)
FetchContent_Declare(sponza
    GIT_REPOSITORY "https://github.com/jimmiebergmann/Sponza.git"
    GIT_TAG "master"
    GIT_SHALLOW ON
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/models/sponza")
FetchContent_MakeAvailable(assimp sponza)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp)