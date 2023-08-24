all : rpm_ctrl

rpm_ctrl : 
	colcon build 

run : rpm_ctrl
	ros2 run roscan rpm_ctrl