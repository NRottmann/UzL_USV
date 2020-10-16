// ROS packages
#include "ros/ros.h"
#include "serial/serial.h"
#include "std_msgs/String.h"
#include "std_msgs/Empty.h"
#include "std_msgs/Char.h"
#include "sensor_msgs/NavSatFix.h"
#include "sensor_msgs/MagneticField.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/Range.h"
#include "sensor_msgs/Temperature.h"
#include "boat_pkg/Heading.h"
#include "boat_pkg/Safety.h"

// For splitting the input string
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>

// Other?
#include <tuple>
#include <array>
#include <cstring>
#include <stdio.h>

// Define namespace and global variables
using namespace std;
serial::Serial ser;

// Function for splitting
template <class Container>
void split_string(const string& str, Container& cont, char delim = ',')
{
	stringstream ss(str);
	string token;
	while (getline(ss, token, delim)) {
		cont.push_back(token);
	}
}

// Callback functions for getting the velocity
void velocityCB(const geometry_msgs::Twist& msg) {
	float lim = 100;
	float offset = 0;

	float L = 1;

	float w_l = msg.linear.x + L * msg.angular.z;
	float w_r = msg.linear.x - L * msg.angular.z;

	w_r = w_r * (lim / (1 + L));
	w_l = w_l * (lim / (1 + L));

	if (w_r <= -lim) {
		w_r = -lim;
	}
	else if (w_r > lim) {
		w_r = lim;
	}
	else {
		w_r = w_r;
	}
	if (w_l <= -lim) {
		w_l = -lim;
	}
	else if (w_l > lim) {
		w_l = lim;
	}
	else {
		w_l = w_l;
	}

	int w_r_int = (int)(w_r);
	int w_l_int = -(int)(w_l);

	string n = "ROB," + to_string(w_r_int) + "," + to_string(w_l_int) + ",\n"; // ",0,0 \n";
	ser.write(n.c_str());
	ROS_INFO("%s", n.c_str());
}

// Main function
int main(int argc, char** argv) {
	// Inits ros
	ros::init(argc, argv, "serialInterface");
	ros::NodeHandle nh;
	ros::NodeHandle nhp("~");

	// Publishers
	ros::Publisher pubNavSatFix = nh.advertise<sensor_msgs::NavSatFix>("gps", 10);
	ros::Publisher pubMagneticField = nh.advertise<sensor_msgs::MagneticField>("mag", 10);
	ros::Publisher pubHeading = nh.advertise<boat_pkg::Heading>("heading", 10);
	ros::Publisher pubVelocity = nh.advertise<geometry_msgs::Twist>("velocity", 10);
	ros::Publisher pubRangeDepth = nh.advertise<sensor_msgs::Range>("range_depth", 10);
	ros::Publisher pubRangeFront = nh.advertise<sensor_msgs::Range>("range_front", 10);
	ros::Publisher pubTemperature = nh.advertise<sensor_msgs::Temperature>("temperature", 10);
	ros::Publisher pubSafety = nh.advertise<boat_pkg::Safety>("safety", 10);

	// Subscribers
	ros::Subscriber cmd_vel = nh.subscribe("/cmd_vel", 1, velocityCB);

	// Get parameters
	string serialConnection = "/dev/ttyACM0";
	if (!nhp.getParam("serialConnection", serialConnection)) ROS_ERROR("serialInterface: Could not find serialConnection parameter!");
	ROS_INFO("serialInterface: Loaded Parameter\n serialConnection: %s", serialConnection);

	// Error catching
	try {
		ser.setPort(serialConnection);
		ser.setBaudrate(9600);
		serial::Timeout to = serial::Timeout::simpleTimeout(1000);
		ser.setTimeout(to);
		ser.open();
	}
	catch (serial::IOException& e)
	{
		ROS_ERROR("Unable to open serial port!");
		return -1;
	}
	if (ser.isOpen()) {
		ROS_INFO("Serial port initialized!");
	}
	else {
		ROS_ERROR("Serial Port not open!");
		return -1;
	}

	// Define maximum frequency
	ros::Rate loop_rate(20);

	// Holder
	string result;
	result.clear();

	// Operating loop
	while (ros::ok()) {
		if (ser.available()) {
			string tmpString = ser.read(ser.available());
			for (int i = 0; i < tmpString.length(); i++) {
				if (tmpString.at(i) == 'R') {
					//Split the string
					vector<string> words;
					split_string(result.c_str(), words);

					// Debug
					/* for(int ll=0; ll<words.size(); ll++) {
						ROS_INFO("%s", words[ll].c_str());
					} */


					if(words.size() == 19) {
					// GPS
					float lat = stof(words[2].c_str());		// Latitude
					float lon = stof(words[3].c_str());		// Longitude
					float alt = stof(words[4].c_str());		// Altitude

					sensor_msgs::NavSatFix gps;
					gps.status.STATUS_FIX;
					gps.latitude = lat;
					gps.longitude = lon;
					gps.altitude = alt;
					gps.position_covariance_type = 0;

					// Magnetometer
					float xMag = stof(words[7].c_str());
					float yMag = stof(words[8].c_str());
					float zMag = stof(words[9].c_str());

					sensor_msgs::MagneticField mag;
					mag.magnetic_field.x = xMag;
					mag.magnetic_field.y = yMag;
					mag.magnetic_field.z = zMag;

					// Heading
					float gpsHeading = stof(words[6].c_str());
					float magHeading = stof(words[10].c_str());

					boat_pkg::Heading heading;
					heading.gps_heading = gpsHeading;
					heading.mag_heading = magHeading;

					// Velocity
					float groundSpeed = stof(words[5].c_str());
					geometry_msgs::Twist vel;
					vel.linear.x = groundSpeed;

					// US
					float usDepth = stof(words[11].c_str());
					float usFront = stof(words[12].c_str());

					sensor_msgs::Range us_depth;
					us_depth.radiation_type = 0;
					us_depth.field_of_view = 0.7854;
					us_depth.min_range = 0.0;
					us_depth.max_range = 10.0;
					us_depth.range = usDepth;

					sensor_msgs::Range us_front;
					us_front.radiation_type = 0;
					us_front.field_of_view = 0.7854;
					us_front.min_range = 0.0;
					us_front.max_range = 4.5;
					us_front.range = usFront;

					// Temperature
					float temp = stof(words[13].c_str());

					sensor_msgs::Temperature temperature;
					temperature.temperature = temp;
					temperature.variance = 0.0;

					// Safety
					int battery = stoi(words[14].c_str());
          float voltage = stof(words[15].c_str());
					float current = stof(words[16].c_str());
					float charge = stof(words[17].c_str());
					int water = stoi(words[18].c_str());

					boat_pkg::Safety safety;
					safety.battery = battery;
					safety.voltage = voltage;
					safety.current = current;
					safety.charge = charge;
					safety.water = water;
					if (battery < 21) {
						ROS_WARN("Battery Low! Only %i %% left!", battery);
					}
					if (water != 0) {
						ROS_ERROR("Water in the boat!");
					}

					// Data publishing
					pubNavSatFix.publish(gps);
					pubMagneticField.publish(mag);
					pubHeading.publish(heading);
					pubVelocity.publish(vel);
					pubRangeDepth.publish(us_depth);
					pubRangeFront.publish(us_front);
					pubTemperature.publish(temperature);
					pubSafety.publish(safety);
					}

					// Debug
					// ROS_INFO("%s", result.c_str());

					result.clear();
					result.push_back(tmpString.at(i));
				}
				else {
					result.push_back(tmpString.at(i));
				}
			}

		}
		ros::spinOnce();
		loop_rate.sleep();
	}
}
