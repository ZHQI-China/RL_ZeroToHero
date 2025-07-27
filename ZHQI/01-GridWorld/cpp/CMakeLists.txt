cmake_minimum_required (VERSION 3.21)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(TARGET5 05_rl)
project (${TARGET5})

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_BINARY_DIR}/../ZHQI/_out/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_BINARY_DIR}/../ZHQI/_out/bin)

# message(STATUS "SDL3_LIBRARIES： ${SDL3_LIBRARIES}")
# message(STATUS "CMAKE_SOURCE_DIR：${CMAKE_SOURCE_DIR}")
# message(STATUS "引擎目录：${ENGINE_SOURCES}")
# message(STATUS "1-ghost_escape 目录：${GHOST_ESCAPE_SOURCES}")

# 引擎代码
file(GLOB_RECURSE ENGINE_SOURCES ${CMAKE_SOURCE_DIR}/src/engine/*.cpp)
file(GLOB_RECURSE ENGINE_HEADERS ${CMAKE_SOURCE_DIR}/src/engine/*.h)


# 设置第三方库路径
set(SELF_INSTALL_BASE_PATH "D:/code/_3rdparties/_cmake_install")
set(CMAKE_PREFIX_PATH
    ${SELF_INSTALL_BASE_PATH}/SDL3
    ${SELF_INSTALL_BASE_PATH}/glm
    ${SELF_INSTALL_BASE_PATH}/spdlog
    ${CMAKE_PREFIX_PATH}
)

# 配置 SDL3,glm
# 查找并载入Cmake预设
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)

############################################### TARGET5 ###############################################
file(GLOB_RECURSE TARGET5_SOURCES "${CMAKE_SOURCE_DIR}/src/${TARGET5}/*.cpp")

add_executable (
    ${TARGET5} 
        ${TARGET5_SOURCES}
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
)
target_include_directories(
    ${TARGET5} PRIVATE 
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/engine
        ${CMAKE_SOURCE_DIR}/src/05_rl 
        ${SPDLOG_DIR}/include
)

target_link_libraries(
    ${TARGET5} PRIVATE 
        ${SDL3_LIBRARIES}
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
        glm::glm
        spdlog::spdlog
)
# 设置第三方 DLL 的基础路径
set(SDL3_DLL_BASE_PATH ${SELF_INSTALL_BASE_PATH}/SDL3/lib/x64)

# 定义所有需要复制的 DLL 文件路径
set(SDL3_DLLS
    ${SDL3_DLL_BASE_PATH}/SDL3.dll
    ${SDL3_DLL_BASE_PATH}/SDL3_image.dll
    ${SDL3_DLL_BASE_PATH}/SDL3_mixer.dll
    ${SDL3_DLL_BASE_PATH}/SDL3_ttf.dll
)

# 遍历所有 DLL 并添加复制命令
foreach(DLL_FILE ${SDL3_DLLS})
   add_custom_command(
       TARGET  ${TARGET5} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different
       ${DLL_FILE}
       $<TARGET_FILE_DIR:${TARGET5}>
   )
endforeach()