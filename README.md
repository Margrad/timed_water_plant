# This is the program that will run on an ESP32 to automatically water my plants when I'm not around
I have used a lot of information and code from https://randomnerdtutorials.com/

This server can do the following:
 - Record the values of the humidity sensors in a Log for a limited number of entries;
 - Send an email with the entries int the log every time it's full;
 - Automatically water the plans as scheduled;
 - It can be accessed by a browser on a local network on the port 7531. 
    - The page returned allows to configure the following options:
	 - Manually turning the pumps ON/OFF
	 - Set the pumps to automatically water the plants at a specified time
	 - Set all pumps ON/OFF, or Auto/Manual mode in one go
	 - Send an email with the current values in the Log
         - Label the pumps (so I can more easily remember which pump goes where)
	- TODO: sets which sensors to display in the graph
	- TODO: Label the sensores (so I can more easily remember which sensor goes where)
    - This page also allows to see:
	 - The graphic with the sensors data in the Log
	 - The current value for each sensor
	 - Current time, time when the server started
	 - WiFi Strength