cmake_minimum_required(VERSION 3.18)

set(SOLUTION_NAME dungeonv3-2)	# Change to your project name

#BEGIN: Standard structure for all solutions
project(${SOLUTION_NAME})
set(SOURCE_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(HOME_ROOT $ENV{HOME})
if (WIN32)
	string(REPLACE "\\" "/" HOME_ROOT "${HOME_ROOT}")
endif()
set(WORK_ROOT ${HOME_ROOT}/Work)
include(${WORK_ROOT}/DevEnv/Common.cmake) 
#END: Standard structure for all solutions

include(${WORK_ROOT}/DevEnv/natGUI.cmake)

include(dungeonv3-2.cmake)
