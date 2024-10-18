include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME prosper_opengl)
pr_add_library(${PROJ_NAME} SHARED)

pr_add_dependency(${PROJ_NAME} prosper TARGET PUBLIC)
add_subdirectory("third_party_libs/glad")
pr_add_dependency(${PROJ_NAME} glad TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} iclient TARGET PUBLIC)

pr_add_headers(${PROJ_NAME} "include/")
pr_add_sources(${PROJ_NAME} "src/")

pr_add_compile_definitions(${PROJ_NAME} -DSHPROSPER_OPENGL_DLL PUBLIC)

pr_finalize(${PROJ_NAME})
