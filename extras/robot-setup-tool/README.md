# robot-setup-tool

Tools for setting up your Neobotix's simulation workspace

# Setup Process

## Setup Docker Container from Scratch

1. Clone this repo

2. ```bash
   cd ./robot_setup-tool
   ```

3. Run the following command
   ```bash
   docker build -f Dockerfile.humble22 . -t humblesim
   ```

4. Try running docker:
   ```docker run -it humblesim bash```

   But Better to do:
   ```chmod +x run_sim_docker.sh```
   ```./run_sim_docker.sh```

5. Once you enter the docker environment navigate to the Neobotix Workspace
   ```bash
   cd /root/neobotix_workspace
   ```

6. ```bash
   colcon build --symlink-install

   echo "export LC_NUMERIC="en_US.UTF-8" " >> ~/.bashrc

   echo "source neobotix_workspace/install/setup.bash" >> ~/.bashrc
   ```

## Use Pre-Built Docker Container

1. Clone this repo

2. ```bash
   cd ./robot_setup-tool
   ```

3. Download existing docker image:
   ```bash
   docker pull sushanthj/humble_sim_mapping_built
   ```

4. Modify the run_sim_docker.sh file to be:
   ```bash
   # Map host's display socket to docker
   DOCKER_ARGS+=("-v /tmp/.X11-unix:/tmp/.X11-unix")
   DOCKER_ARGS+=("-v $HOME/.Xauthority:/home/admin/.Xauthority:rw")
   DOCKER_ARGS+=("-e DISPLAY")
   DOCKER_ARGS+=("-e NVIDIA_VISIBLE_DEVICES=all")
   DOCKER_ARGS+=("-e NVIDIA_DRIVER_CAPABILITIES=all")
   DOCKER_ARGS+=("-e FASTRTPS_DEFAULT_PROFILES_FILE=/usr/local/share/middleware_profiles/rtps_udp_profile.xml")

   # Run container from image
   # print_info "Running humblesim"
   docker run -it --rm \
      --privileged \
      --network host \
      ${DOCKER_ARGS[@]} \
      -v <ADD_PATH_TO_ROBOT_SETUP_TOOL>/world_files:/home/admin/worlds \
      -v /dev/*:/dev/* \
      --name "humble_sim_docker" \
      --runtime nvidia \
      $@ \
      sushanthj/humble_sim_mapping_built:latest \
      /bin/bash
   ```


# Run the Simulation

1. Set Environment Variables (note you need to set your own world file inside the neobotix's world folder)
   ```bash
   export MY_ROBOT=mp_400
   export MAP_NAME=neo_workshop
   ```

2. Install tmux and rviz2
   ```bash
   sudo apt install tmux
   sudo apt install ros-humble-rviz2
   ```

3. Create multiple tmux panes, in each you must set environment variables of step 7

4. Run Simulation in first tmux pane
   ```bash
   ros2 launch neo_simulation2 simulation.launch.py
   ```

5. Run Navigation in Simulation in second tmux pane
   ```bash
   ros2 launch neo_simulation2 navigation.launch.py map:=/root/neobotix_workspace/src/neo_simulation2/maps/neo_workshop.yaml
   ```

6. Run rviz2 in third tmux pane
   ```bash
   ros2 launch neo_nav2_bringup rviz_launch.py
   ```



# Using Custom World Files for Simulation

1. Use ```world_files``` folder to copy the ```trial_world.world``` into the ```~/neobotix_workspace/src/neo_simulation2/worlds``` folder
2. Use ```world_files``` folder to copy the entire folder ```mfi_floor_trial1``` into ```~/neobotix_workspace/src/neo_simulation2/models``` folder
3. Change ```export MAP_NAME=trial_world```
4. Run the simulation as in step 10 in previous section


## Detailed Reference
The below reference will also show how to use neobotix's neo_mp-400-2/mapping package which is a better alternative to the manual mapping package above
[Team H Website](https://mrsd-project.herokuapp.com/docs/Simulation/build_floorplan.html){: .btn .fs-3 .mb-4 .mb-md-0 }



# Appendix

1. If you want to stop any active running continers
   ```bash
   docker rm $(docker ps -a -q)
   ```

2. If you want to retry docker image builds after changing something in this repo, it's suggested to delete existing images which are not required:
   ```bash
   docker images
   docker rmi <image_tag_or_ID>
   ```
3. If you face issues with display driver not working, then before running the run_sim_docker.sh do:
   ```bash
   xhost +local:docker
   ```
