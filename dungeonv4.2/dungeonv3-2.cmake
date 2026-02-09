# dungeonv2.cmake
set(PROJECT_NAME dungeonv3)

# Explicitly list only the source files we want
file(GLOB PROJECT_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB PROJECT_INCS  ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
# Application icon
set(PROJECT_PLIST  ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/AppIcon.plist)
if(WIN32)
    set(PROJECT_WINAPP_ICON ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/winAppIcon.rc)
else()
    set(PROJECT_WINAPP_ICON ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/winAppIcon.cpp)
endif()

# add executable
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_INCS} ${PROJECT_WINAPP_ICON})

# Source groups for IDE organization (optional, can be removed if causing issues)
if(MSVC)
    source_group("inc"            FILES ${PROJECT_INCS})
	source_group("src"        FILES ${PROJECT_SOURCES})
endif()

target_link_libraries(${PROJECT_NAME} 
    debug ${MU_LIB_DEBUG} 
    debug ${NATGUI_LIB_DEBUG} 
    optimized ${MU_LIB_RELEASE} 
    optimized ${NATGUI_LIB_RELEASE})

setTargetPropertiesForGUIApp(${PROJECT_NAME} ${PROJECT_PLIST})

setAppIcon(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR})

setIDEPropertiesForGUIExecutable(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR})

setPlatformDLLPath(${PROJECT_NAME})