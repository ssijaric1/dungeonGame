# dungeonGame.cmake - Project specific configuration

set(dungeonGame_NAME dungeonGame)				# Project name

# File globbing - match the pattern exactly
file(GLOB dungeonGame_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB dungeonGame_INCS  ${CMAKE_CURRENT_LIST_DIR}/*.h)

# Application icon (even if we don't have one, follow the pattern)
set(dungeonGame_PLIST  ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/AppIcon.plist)
if(WIN32)
	set(dungeonGame_WINAPP_ICON ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/winAppIcon.rc)
else()
	set(dungeonGame_WINAPP_ICON ${CMAKE_CURRENT_LIST_DIR}/res/appIcon/winAppIcon.cpp)
endif()

# add executable - match pattern exactly
add_executable(${dungeonGame_NAME} ${dungeonGame_INCS} ${dungeonGame_SOURCES} ${dungeonGame_WINAPP_ICON})

source_group("inc"            FILES ${dungeonGame_INCS})
source_group("src"            FILES ${dungeonGame_SOURCES})

target_link_libraries(${dungeonGame_NAME} 
    debug ${MU_LIB_DEBUG} 
    debug ${NATGUI_LIB_DEBUG} 
    optimized ${MU_LIB_RELEASE} 
    optimized ${NATGUI_LIB_RELEASE}
)

setTargetPropertiesForGUIApp(${dungeonGame_NAME} ${dungeonGame_PLIST})

setAppIcon(${dungeonGame_NAME} ${CMAKE_CURRENT_LIST_DIR})

setIDEPropertiesForGUIExecutable(${dungeonGame_NAME} ${CMAKE_CURRENT_LIST_DIR})

setPlatformDLLPath(${dungeonGame_NAME})