cmake_minimum_required(VERSION 3.1)
project(boat_pkg)

find_package(catkin REQUIRED COMPONENTS
	roscpp
	serial
	std_msgs
	sensor_msgs
	message_generation
)


add_message_files(
  FILES
  Heading.msg
  Safety.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

catkin_package(
  CATKIN_DEPENDS 
    roscpp 
    serial 
    std_msgs
    sensor_msgs 
    message_runtime)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${src}
)

add_executable(serialInterface src/serialInterface.cpp)
target_link_libraries(serialInterface ${catkin_LIBRARIES})
add_dependencies(serialInterface ${catkin_EXPORTED_TARGETS} boat_pkg_generate_messages_cpp)
