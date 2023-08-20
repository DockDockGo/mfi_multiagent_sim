#!/bin/bash

# exit if any command below fails
set -e

# Check if ROS is sourced

if [ "$ROS_DISTRO" == "" ];
then
	echo "Installation cannot continue. No ROS sourced, please check if ROS is installed and sourced. Please try again after that!"
	exit 0
fi

sudo apt update

# Install build tool
sudo apt install python3-colcon-common-extensions

sudo apt-get install ros-humble-xacro

# Install Gazebo packages

sudo apt-get install -y ros-$ROS_DISTRO-gazebo-ros ros-$ROS_DISTRO-gazebo-plugins ros-$ROS_DISTRO-gazebo-ros-pkgs

#Install xterm

sudo apt install -y xterm

#Install tmux
sudo apt install -y tmux

# Install navigation packages

# Nav2
sudo apt install -y ros-$ROS_DISTRO-navigation2 ros-$ROS_DISTRO-nav2-*

sudo apt install -y ros-$ROS_DISTRO-slam-toolbox

#Teleop-joy
sudo apt-get install -y ros-$ROS_DISTRO-teleop-twist-joy

#Teleop-key
sudo apt-get install -y ros-$ROS_DISTRO-teleop-twist-keyboard

# rviz
sudo apt install ros-$ROS_DISTRO-rviz2

# cd ~

# mkdir -p mp_400_ws/src
# cd mp_400_ws/src

# clone git repos here...
# git clone --branch $ROS_DISTRO     https://github.com/neobotix/neo_simulation2.git
# git clone --branch $ROS_DISTRO     https://github.com/neobotix/neo_nav2_bringup.git
# git clone --branch $ROS_DISTRO     https://github.com/neobotix/neo_local_planner2.git
# git clone --branch $ROS_DISTRO     https://github.com/neobotix/neo_localization2.git
# git clone --branch master          https://github.com/neobotix/neo_common2
# git clone --branch master          https://github.com/neobotix/neo_msgs2
# git clone --branch master          https://github.com/neobotix/neo_srvs2
# git clone --branch multi-robot-sim-with-xacro     https://github.com/neobotix/neo_simulation2.git
# git clone --branch main  		   https://github.com/neobotix/neo_fleet_rviz2_plugin.git

# copy workspace 0 files
# cp ~/robot-setup-tool/world_files/workspace_0/workspace_0* ~/mp_400_ws/src/neo_simulation2/maps/
# cp ~/robot-setup-tool/world_files/workspace_0/worlds/* ~/mp_400_ws/src/neo_simulation2/worlds/

# #copy svd_demo
# cp ~/robot-setup-tool/world_files/svd_demo/map/* ~/mp_400_ws/src/neo_simulation2/maps/
# cp ~/robot-setup-tool/world_files/svd_demo/world/* ~/mp_400_ws/src/neo_simulation2/worlds/
# cp -r ~/robot-setup-tool/world_files/svd_demo/models/* ~/mp_400_ws/src/neo_simulation2/models/

# # build workspace
# cd ..
# colcon build --symlink-install 

echo "export LC_NUMERIC="en_US.UTF-8" " >> ~/.bashrc

echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

echo "Setup Successful, Continue Installation after creating container!!!"

# echo "Do not forget to set the environment variables"

exit 0
