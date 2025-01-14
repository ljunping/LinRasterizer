cmake_minimum_required(VERSION 3.29)
project(LinRasterizer)

set(CMAKE_CXX_STANDARD 20)
find_package(X11 REQUIRED)
find_package(assimp REQUIRED)
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)




# 目标文件夹路径
set(TARGET_DIR ${CMAKE_BINARY_DIR})

# 添加编译器选项，针对 Debug 定义 DEBUG 宏
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
endif()

# 添加编译器选项，针对 Debug 定义 DEBUG 宏
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

#
#file(GLOB_RECURSE LUA_SOURCES_C "lua-5.4.7/src/*.c")
#file(GLOB_RECURSE LUA_SOURCES_HEADER "lua-5.4.7/src/*.h")

add_executable(LinRasterizer src/main.cpp
#        ${LUA_SOURCES_C}
#        ${LUA_SOURCES_HEADER}
        include/stb_image.h
        include/stb_image_write.h
        include/L_math.h
        src/WindowHandle.cpp
        src/WindowHandle.h
        src/Geometry.cpp
        src/Geometry.h
        src/TrianglePrimitive.cpp
        src/TrianglePrimitive.h
        src/object/Camera.cpp
        src/object/Camera.h
        src/object/Mesh.cpp
        src/object/Mesh.h
        src/LinkList.h
        src/object/FragShader.cpp
        src/object/FragShader.h
        src/object/Component.cpp
        src/object/Component.h
        src/Color.cpp
        src/Color.h
        src/debug.h
        src/PODPool.cpp
        src/PODPool.h
        src/JobSystem.cpp
        src/JobSystem.h
        src/RasterizerJob.h
        src/RasterizerJob.cpp
        src/BresenhamLine.h
        src/VectorRemoveEasy.h
        src/object/Texture.cpp
        src/object/Texture.h
        src/object/Material.cpp
        src/object/Material.h
        src/Context.cpp
        src/Context.h
        src/object/Object.cpp
        src/object/Object.h
        src/object/MeshRender.cpp
        src/object/MeshRender.h
        src/object/Transform.cpp
        src/object/Transform.h
        src/CommonMacro.h
        src/object/Resource.cpp
        src/object/Resource.h
        src/EventSystem.cpp
        src/EventSystem.h
        src/object/Light.cpp
        src/object/Light.h
        src/object/VertShader.cpp
        src/object/VertShader.h
        src/DrawCallContext.cpp
        src/DrawCallContext.h
        src/DrawCallSetting.h
        src/GPU.cpp
        src/GPU.h
        src/object/DrawCallNodeComponent.cpp
        src/object/DrawCallNodeComponent.h
        src/MeshGenerator.h
        src/MeshGenerator.cpp
        src/BVHTree.cpp
        src/BVHTree.h
        src/ImageUtil.cpp
        src/ImageUtil.h

)
target_include_directories(LinRasterizer PRIVATE ${X11_INCLUDE_DIR})
target_include_directories(LinRasterizer PRIVATE ./include)
target_include_directories(LinRasterizer PRIVATE ./src)
target_include_directories(LinRasterizer PRIVATE ./src/object)
#target_include_directories(LinRasterizer PRIVATE ./lua-5.4.7/src)


target_link_libraries(LinRasterizer ${X11_LIBRARIES})
target_link_libraries(LinRasterizer ${ASSIMP_LIBRARIES})

target_compile_definitions(LinRasterizer PRIVATE WINDOW_X11)
target_compile_definitions(LinRasterizer PRIVATE LUA_LIBRARY)

target_compile_definitions(LinRasterizer PRIVATE PROFILE)


add_custom_command(TARGET LinRasterizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCE_DIR}  # 资源文件夹路径
        ${TARGET_DIR}    # 目标输出目录（可执行文件目录）
)
