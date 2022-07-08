#include <gpiod.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <string.h>
#include <sstream>

using namespace std;

bool debug = true;

int tempInt(string);
void log(string);

int main(){

	int ret;

	const char* chipname = "gpiochip0";
	struct gpiod_chip* chip;

	struct gpiod_line* solarPump; //BCM Nr: 18

	chip = gpiod_chip_open_by_name(chipname);
	if(!chip){
		perror("Opne chip failed\n");
		return 1;
	}

	solarPump = gpiod_chip_get_line(chip, 18);
	if(!solarPump){
		perror("Get line for the solarpump failed!\n");
		return 1;
	}

	ret = gpiod_line_request_output(solarPump, "SolarPump", 1);
	if(ret<0){
		perror("Requesst line as output failed!\n");
		return 1;
	}

	ifstream sensor;
	string temperature;
	int solarPanelTemp = 0, solarBoilerTemp = 0, boilerTopTemp = 0, diff = 8000, workIndicator = 0;

	for(;;){

	//Solarpanel
	sensor.open("/sys/bus/w1/devices/28-00000b5795ed/w1_slave");
	if(!sensor.fail()){
		getline(sensor, temperature);
		//if(debug) cout << temperature << "\n";
		getline(sensor, temperature);
		//if(debug) cout << temperature << "\n";
		
		solarPanelTemp = tempInt(temperature);
		if(debug) cout << "SolarPanelTemp: " << solarPanelTemp  << "\n";
		sensor.close();
		
		log("Solarpanel: " + to_string(solarPanelTemp));
	}
	else{
		cout << "Can not open Solarpanel!\n";
	};

	//Solarboiler
	sensor.open("/sys/bus/w1/devices/28-000005a43d45/w1_slave");
	if(!sensor.fail()){
		getline(sensor, temperature);
		//if(debug) cout << temperature << "\n";
		getline(sensor, temperature);
		//if(debug) cout << temperature << "\n";
		
		solarBoilerTemp = tempInt(temperature);
		if(debug) cout << "SolarBoilerTemp: " << solarBoilerTemp  << "\n";
		sensor.close();
		
		log("Solarboiler: " + to_string(solarBoilerTemp));
	}
	else{
		if(debug) cout << "Can not open Solarboiler!\n";
	};

	//Boilertop
	sensor.open("/sys/bus/w1/devices/28-0315046004ff/w1_slave");
		if(!sensor.fail()){
		getline(sensor, temperature);
		//if(debug) cout << temperature << "\n";
		getline(sensor, temperature);
		//if(debug) cout << temperature << "\n";
		
		boilerTopTemp = tempInt(temperature);
		if(debug) cout << "BoilerTopTemp: " << boilerTopTemp  << "\n";
		sensor.close();
		
		log("BoilerTopTemp: " + to_string(boilerTopTemp));
	}
	else{
		if(debug) cout << "Can not open Boilertop!\n";
	};
	

	if (solarPanelTemp > solarBoilerTemp + diff && !workIndicator) {

		gpiod_line_set_value(solarPump, 0); //Turn the pump ON
		workIndicator = 1;
		log("Solar pump turned ON");
		if(debug) cout << "Solar pump turned ON\n";
	}
	else if (workIndicator){
		gpiod_line_set_value(solarPump, 1); // Trun the pump OFF
		log("Solar pump turned OFF");
		if(debug) cout << "Solar pump turned OFF\n";
		workIndicator = 0;

	};

	this_thread::sleep_for(5000ms); //waiting 2sec
	};

	gpiod_line_release(solarPump);
	gpiod_chip_close(chip);

	return 0;
}

int tempInt (string row){

	int temp = 0;
	string rawtemp;

	rawtemp = row.substr(row.find("=")+1, row.length()-1);
	//if(debug) cout << "rawtemp" << rawtemp << "\n";

	stringstream x;

	x << rawtemp;
	x >> temp;
	
	return temp;
}

void log(string text){
	
	ofstream f;
	
	f.open("/home/pi/solarheating/log.txt", ios_base::app);

	std::time_t result = std::time(nullptr);
	string ltime = std::asctime(std::localtime(&result));
	ltime = ltime.substr(0, ltime.length()-1);

	f << ltime << " - " << text << "\n";

	f.close();

}
