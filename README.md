# UzL_USV

This is the ROS package for the unmanned surface vehicle of the university of Luebeck. 



## Table of Contents  

[Sensors and Actuators](#sensorsAndActuators) <br/>

[A-DRZ Registration and Communication](#adrzConnection) <br/>



## Sensors and Actuators <a name="sensorsAndActuators"></a>

The USV has two motors and multiple sensors. In the serialInterface node, addressing and reading out of the actuators and the sensors respectively is defined.

#### Serial Connection

The individual entries are separated by a commas, e.g.

````html
ROB,0.00,0.00,0.00,0.00,0.00,0.00,0,0,0,0,0.00,0.00,0.00,0,0.00,0.00,0.00,0
````

for the received sensor values or

````html
ROB,0,0
````

for the sent actuator messages.

###### Received from the boat: 

| Description                                        |  Type   |     Range     |  Unit  | Index |
| :------------------------------------------------- | :-----: | :-----------: | :----: | :---: |
| Message ID to identify the beginning of the String | String  |       -       |   -    |   0   |
| Positional Dilution of Precision                   |  Float  |       -       |   -    |   1   |
| Latitude                                           |  Float  |  -90 ... 90   |   °    |   2   |
| Longitude                                          |  Float  | -180 ... 180  |   °    |   3   |
| Height above ground level                          |  Float  |       -       |   m    |   4   |
| Ground speed                                       |  Float  |       -       |  m/s   |   5   |
| Motion heading                                     |  Float  | -180 ... 180  |   °    |   6   |
| Magnetometer x-direction                           |  Float  |       -       | uTesla |   7   |
| Magnetometer y-direction                           |  Float  |       -       | uTesla |   8   |
| Magnetometer z-direction                           |  Float  |       -       | uTesla |   9   |
| Magnetometer heading                               |  Float  | 0 ... 359.999 |   °    |  10   |
| US distance (Depth)                                |  Float  |   0 ... 10    |   m    |  11   |
| US distance (Front)                                |  Float  |   0 ... 4.5   |   m    |  12   |
| Temperature                                        |  Float  |       -       |   °C   |  13   |
| Batttery (Safety)                                  | Integer |   0 ... 100   |   -    |  14   |
| Voltage (Safety)                                   |  Float  |       -       |   -    |  15   |
| Current (Safety)                                   |  Float  |       -       |   -    |  16   |
| Charge (Safety)                                    |  Float  |       -       |   -    |  17   |
| Water in Boat (Safety)                             | Integer |    0 ... 1    |   -    |  18   |

###### Sent to the boat:

| Description                                        |  Type   |    Range     | Unit | Index |
| -------------------------------------------------- | :-----: | :----------: | :--: | :---: |
| Message ID to identify the beginning of the String | String  |      -       |  -   |   0   |
| Motor power (right)                                | Integer | -100 ... 100 |  -   |   1   |
| Motor power (left)                                 | Integer | -100 ... 100 |  -   |   2   |



## A-DRZ Registration and Communication <a name="adrzConnection"></a>

The A-DRZ registration and communication requires two steps: the registration via the A-DRZ registration service and the initialization of the Nimbro network connection. *For both we require to be in the same network as the A-DRZ base station.*

#### Registration Service

Follow the instructions from [here](https://redmine.rettungsrobotik.de/projects/a-drz/wiki/RegistrationServiceDocumentation) for the *Without ROS* procedure (Unfortunately, the ROS procedure does not work on the Raspbian Pi). We only have to adjust the config.json in order to define our messages we like to send to the base station, for example to

````
{
	"server_url" : "http://10.42.1.16:8080/registration/register_robot",
	"user_name": "development",
	"pass_word": "LookMomNoVPN!",
	"robot_name" : "UzL_USV",
	"robot_type" : "USV",
	"operator"   : "",
	"mode"       : ["teleoperated", "autonomous", "semi-autonomous"],
	"capabilities": [
		{"<adrz:PositionCapability>" : [{"/UzL_USV/gps" : "sensor_msgs/NavSatFix"}]},
		{"<adrz:MagneticField>" : [{"/UzL_USV/mag" : "sensor_msgs/MagneticField"}]},
		{"<adrz:Depth>" : [{"/UzL_USV/range_depth" : "sensor_msgs/Range"}]},
		{"<adrz:Temperature>" : [{"/UzL_USV/temperature" : "sensor_msgs/Temperature"}]},
                {"<adrz:VideoStreamingCapability>" : [{"rtsp_url" : "rtsp://10.42.1.181:8554/"}]}
	]
}
````

Afterwards we can build everything and try to connect to the base station.

#### Nimbro Network

Follow the instructions from [here](https://redmine.rettungsrobotik.de/projects/a-drz/wiki/NimbroNetwork) to install the nimbro repository onto the Raspbian Pi. You might require to edit some parts of the code in order to be able to compile the package. Also, you have to add

````
add_definitions(-DGF256_TARGET_MOBILE)
````

to the CMakeLists.cpp of the nimbro_topic_transport package since it disables some libraries which are only available for Intel CPUs. After successfully installing the nimbro repository, you can now launch everything.

