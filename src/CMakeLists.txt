file(GLOB_RECURSE SOURCE_FILES *.cc)

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Vulkan_VERSION = ${Vulkan_VERSION}")
    message("")
endif ()

add_library(${PROJECT_NAME}_headers INTERFACE)
target_include_directories(${PROJECT_NAME}_headers INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_headers PUBLIC Vulkan::Vulkan)
