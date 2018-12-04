cmake_minimum_required(VERSION 3.5)
project(ros_environment NONE)
find_package(ament_cmake_core REQUIRED)

set(ROS_VERSION "2")
set(ROS_PYTHON_VERSION "3")

# allow overriding the distro name
if(DEFINED ENV{ROS_DISTRO_OVERRIDE})
  set(ROS_DISTRO $ENV{ROS_DISTRO_OVERRIDE})
else()
  set(ROS_DISTRO "bouncy")
endif()

set(
  hooks
  "1.ros_distro"
  "1.ros_version"
  "1.ros_python_version"
)
if(CMAKE_HOST_UNIX)
  set(shell "sh")
else()
  set(shell "bat")
endif()
foreach(hook ${hooks})
  ament_environment_hooks(
    "${CMAKE_CURRENT_SOURCE_DIR}/env-hooks/${hook}.${shell}.in")
endforeach()

ament_package()
