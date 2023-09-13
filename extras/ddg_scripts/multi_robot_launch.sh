#!/usr/bin/env bash
# gnome-termial
cd ../../ws && . install/setup.bash
# Kill previous session
tmux kill-session

map="svd_demo" # other options: "workspace_0", "aws", "NEO_WORKSHOP"
num_robots="2" # 

# Create a tmux session
session_name="ddg_$(date +%s)"
tmux new-session -d -s $session_name 

# Split the window into three panes
tmux selectp -t 0    # select the first (0) pane
tmux splitw -v -p 50 # split it into two halves

tmux selectp -t 0    # go back to the first pane
tmux splitw -h -p 33 # split it into two halves
tmux selectp -t 0    # select the first (0) pane
tmux splitw -h -p 40 # split it into two halves

tmux selectp -t 3    # go back to the first pane
tmux splitw -h -p 50 # split it into two halves

# tmux selectp -t 3    # go back to the first pane
# tmux splitw -h -p 33 # split it into two halves
# tmux selectp -t 3    # select the first (0) pane
# tmux splitw -h -p 40 # split it into two halves
# # tmux splitw -h -p 33 # split it into two halves

# Run the single robot simulation
tmux select-pane -t 0
tmux send-keys "export MAP_NAME=$map" Enter
tmux send-keys "export MY_ROBOT=mp_400" Enter
tmux send-keys "export Number_of_Robots=$num_robots" Enter
tmux send-keys "source /usr/share/gazebo-11/setup.bash" Enter
tmux send-keys "ros2 launch neo_simulation2 multi_robot_simulation.launch.py" Enter

# Run the run single robot navigation
tmux select-pane -t 1
tmux send-keys "export MAP_NAME=$map" Enter
tmux send-keys "export MY_ROBOT=mp_400" Enter
tmux send-keys "export Number_of_Robots=$num_robots" Enter
tmux send-keys "sleep 6 && ros2 launch neo_simulation2 multi_robot_navigation.launch.py" Enter

# Run rviz
tmux select-pane -t 2
tmux send-keys "export MAP_NAME=$map" Enter
tmux send-keys "export MY_ROBOT=mp_400" Enter
tmux send-keys "export Number_of_Robots=$num_robots" Enter
# tmux send-keys "sleep 7 && ros2 launch neo_nav2_bringup rviz_launch.py rviz_config:=install/neo_nav2_bringup/share/neo_nav2_bringup/rviz/svd_demo_final.rviz" Enter
tmux send-keys "sleep 7 && ros2 launch neo_nav2_bringup rviz_launch.py rviz_config:=src/neo_nav2_bringup/rviz/svd_demo_final.rviz" Enter


Run multi-robot commander
tmux select-pane -t 3
tmux send-keys "export MAP_NAME=\"$map\"" Enter
tmux send-keys "export MY_ROBOT=mp_400" Enter
tmux send-keys "export Number_of_Robots=\"$num_robots\"" Enter
tmux send-keys "sleep 20 && ros2 launch ddg_multi_robot_planner multi_robot_planner.launch.py" Enter 
# tmux send-keys "sleep 60 && ros2 run multi_navigator multi_commander" Enter 


Run multi-robot commander
tmux select-pane -t 4
tmux send-keys "export MAP_NAME=\"$map\"" Enter
tmux send-keys "export MY_ROBOT=mp_400" Enter
tmux send-keys "export Number_of_Robots=\"$num_robots\"" Enter
tmux send-keys "sleep 60 && ros2 run multi_navigator multi_commander" Enter 

# Attach to the tmux session
tmux -2 attach-session -t $session_name -c /ws