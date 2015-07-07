cmake_minimum_required(VERSION 3.0.2)

project(TieGunner)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "-std=c11 ${CMAKE_C_FLAGS}")
endif()

file(GLOB_RECURSE source_files src/*.c)

if(APPLE)
  list(REMOVE_ITEM source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/os_win.c ${CMAKE_CURRENT_SOURCE_DIR}/src/os_unix.c)

elseif(WIN32)
  list(REMOVE_ITEM source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/os_mac.c ${CMAKE_CURRENT_SOURCE_DIR}/src/os_unix.c)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
elseif(UNIX)
  list(REMOVE_ITEM source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/os_mac.c ${CMAKE_CURRENT_SOURCE_DIR}/src/os_win.c)
  set(ENVIRONMENT_SPECIFIC_LIBRARIES "m")
else()
  message(FATAL "this system is not supported yet.")
endif()

add_executable(TieGunner ${source_files})

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${OPENAL_INCLUDE_DIR}
  ${PNG_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

target_link_libraries(TieGunner
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARY}
  ${OPENAL_LIBRARY}
  ${PNG_LIBRARY}
  ${ZLIB_LIBRARIES}
  ${ENVIRONMENT_SPECIFIC_LIBRARIES}
)
