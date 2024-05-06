# This is the program that will run on an ESP32 to automatically water my plans when I'm not around
This server can do the following:
 - Record the values of the humidity sensores in a Log for a limited number of entries
 - Send an email with this entries every time it's full
 - Automatically water the plans as scheduled
 - It can be accessed by a browser on a local network on the port 7531. 
    - The page that returned allows to congigure the following options
	- Manually turning the pumps ON/OFF
	- Set the pumps to automatically water the plants at a specified time
	- Set all pumps ON/OFF, or Auto/Msnual mode in one go
	- Send an email with the current values in the Log
	- TODO: sets which sensores to display in the graph
    - This page also allows to see:
	- The graphic with the sensores data in the Log
	- The current value for each sensor
	- Current time, time when the server started
	- WiFi Strengh
