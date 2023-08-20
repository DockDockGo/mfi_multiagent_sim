### Scripts to launch the multirobot simulation

The scripts in this repo are used to launch single robot and multirobot simulations in gazebo.
Assets for world and map files can be found [here](https://github.com/DockDockGo/robot-setup-tool/tree/ros2/world_files)

1. copy the `.pgm` and `.yaml` files to `neo_simulation2/maps/`
2. copy the `.world` file to `neo_simulation2/worlds/`
3. copy the `.pgm` and `.yaml` to `neo_simulation2/maps/`
4. copy the `.rviz` config to `/neo_nav2_bringup/rviz/`

Follow the 'Run the Simulation' steps as listed in [robot-setup-utils](https://github.com/DockDockGo/robot-setup-tool/tree/ros2) to setup everything for the first time.

After everything is setup as expected, you can use the scripts in this folder to run the simulations.

To launch a single robot simulation use this:

```
bash single_robot_launch.sh
```

To launch multi robot simulation use this:

```
bash multi_robot_launch.sh
```

**Note**: You can change the number of robots spawned and the spawn environment by changing the [`map`](https://github.com/DockDockGo/ddg_scripts/blob/d4efecb0f0b07bbca49d0fcee8c18fd4ef24b5d2/multi_robot_launch.sh#L8) and [`num_robots`](https://github.com/DockDockGo/ddg_scripts/blob/d4efecb0f0b07bbca49d0fcee8c18fd4ef24b5d2/multi_robot_launch.sh#LL9C1-L9C1) field inside `multi_robot_launch.sh`
