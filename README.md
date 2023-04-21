# rbe594_asars_ws

An Autonomous Search and Rescue System (ASARS) created for the Spring 2023 RBE 594 Capstone project.

## Installation

Clone repository:
```bash
cd ~/
git clone https://github.com/poolec4/rbe594_asars_ws.git
cd ~/rbe594_asars_ws/
git submodule init
git submodule update
```

Build catkin environment:
```bash
catkin_make 
source ~/rbe594_asars_ws/devel/setup.bash
```

## World Test

Open the test environment with LIDAR unit:
```bash
roslaunch rbe594_asars my_world.launch
```

## Hector Test

This will run Hector UAV in a test world and start the teleop menu. It will also open the sensor visualization in RViz. This has the downward facing LIDAR for ground mapping:
```bash
roslaunch rbe594_asars hector_world.launch
```

## Husky Test

This will run Husky AGV in a test world and start the teleop menu. It will also open the sensor visualization in RViz. This has the front facing HUSKY_UST10 2D-LIDAR for local planner:
```bash
roslaunch rbe594_asars husky_world.launch
```


## Full Simulation

This will run the full ASARS simulation. It will begin by generating the world and spawning the victims. Then, it will begin the UAV mapping and scanning operations. Once this is complete, the occupancy grid is generated and the AGV planning commences.
```bash
roslaunch rbe594_asars main_asars_world.launch
```
