# CMakeList.txt : CMake project for MCSC202, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("MCSC202")

find_package(OpenGL REQUIRED)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(SDL3_DIR "${CMAKE_SOURCE_DIR}/external/sdl/cmake")
find_package(SDL3 REQUIRED)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    # Add your backend files here, e.g.:
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
     ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    # ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    # ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Add source to this project's executable.
add_executable (MCSC202 "MCSC202.cpp" "MCSC202.h" ${IMGUI_SOURCES} )

target_include_directories(MCSC202 PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(MCSC202 PRIVATE OpenGL::GL)
target_link_libraries(MCSC202 PRIVATE SDL3::SDL3)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MCSC202 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
