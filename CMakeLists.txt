cmake_minimum_required(VERSION 3.1.0)

project(DTKWallpaperManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(target DTKWallpaperManager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#if (CMAKE_VERSION VERSION_LESS "3.7.0")
#    set(CMAKE_INCLUDE_CURRENT_DIR ON)
#endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Gui  REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(Dtk COMPONENTS Core REQUIRED)
find_package(Dtk COMPONENTS Gui)

file(GLOB SRC src/*.cpp)
file(GLOB HEAD include/*.h)

include_directories(include)

add_executable(${PROJECT_NAME}
        ${SRC} ${HEAD}
        resources.qrc)

target_link_libraries(DTKWallpaperManager PRIVATE
    Qt5::Widgets
    Qt5::Gui
    Qt5::Network
    ${DtkGui_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkWidget_LIBRARIES}
) # 链接库
