
# Automated Hydroponics

A prototype system combining the VEX EXP robotics platform with Arduino to automate the planting and watering process of plants.


## Group Members (Group 20)

- Thomas Gordge - [@Nox-iv](https://github.com/Nox-iv)
    - Construction of horizontal gantry (from scratch)
    - Fabrication of multiple 3D printed elements (stands, gears etc)
    - Programming of gantry
    - Communication between Arduinos
- Louis Putland - [@moz-ljp](https://github.com/moz-ljp)
    - Construction of pump system
    - Construction of vex arm and seed dispersal component
    - Programming of pump system, vex arm and disperal component
    - Communication between VEX and Arduino


## Features

- Horizontal gantry allows for multiple plants
- Automated digging of a hole for seeds
- Automated disperal of seeds into hole
- Monitoring of each plant site for moisture control
- Automated watering of plants when signaled by moisture sensors


## Roadmap

- Move seed disperal system onto gantry for longer gantry length
- Add light sensors and humidity sensors to further monitor/control plant health
- Add a light we can enable/disable to aid plant growth
- Add container around plants to allow for humidity control

## How to Run

- Upload control_box arduino code to arduino inside pump controller box
- Upload move_control arduino code to arduino connected to gantry
- Upload plant_master_vexcode file to Vex EXP brain
- Start all three devices, gantry should find home position
- Once started, the serial monitor for control_box arduino should be waiting for commands
- You can then enter 1,2 or 3 to make it plant in said positions.
- Watering is automatic and will occur when a sensor reading is low.

## Demo
- See videos in submission
