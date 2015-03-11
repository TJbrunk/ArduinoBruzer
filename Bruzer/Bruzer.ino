/*
 * Bruzer.ino
 *
 * Created: 3/2/2015 1:15:07 PM
 * Author: TBrink
 Yeast range 15-22*C
 */ 

#include <stdint.h>
#include "I2C.h"//I2C library downloaded from http://www.dsscircuits.com/articles/arduino-i2c-master-library

/***************** I2C addresses for temp sensors  ************************/
const int ambient = 0x4a;//TC 74a digital temperature sensors from Micro
const int beer = 0x48;

/***************** Sketch variables ************************************/
//Temperature setpoints in *C
const int sp_high = 21;//ambient air temp 'max'
const int sp = 19;  //ambient air temp set point
const int sp_low = 16; //minimum ambient air temp

//Temperature Variables
int temp_ambient;
int temp_beer;

//State machine states
enum State {
	idle,
	cooling,
	heating,
	cooling_db,
	heating_db
} state;

//Fridge and heater are connected to Normally Open relay circuit, so pin values are reversed
boolean on = LOW;
boolean off = HIGH;

//Enable (true)/Disable (false) verbose serial output
boolean debugging = true;
/********************** Arduino Pins ****************************/
const int heater = 9; 
const int fridge = 8;


void setup(){
	//enable serial if debugging
	if (debugging){Serial.begin(9600);};
	//Set pins & states
	pinMode(heater, OUTPUT);
	pinMode(fridge, OUTPUT);
	digitalWrite(fridge, LOW);
	digitalWrite(heater, LOW);
	//Start I2C library
	I2c.begin();
	//Set a 3 second timeout for I2C data
	I2c.timeOut(3000);
	//Set both Temp sensors to read from register 0
	I2c.write(beer, 0);
	I2c.write(ambient, 0);
	//Get current temperatures
	read_temp();
	//Set the beginning state based on the temperature
	if (temp_ambient > sp_high){
		state = cooling;
	}
	if (temp_ambient < sp_low){
		state = heating;
	}
	else{
		state = idle;
	}
	if(debugging){
	  Serial.println("\n\n\nSetup Complete.  ");
	}
	debug_print(temp_ambient);
}

int read_temp(){
	
	
	I2c.read(beer, 1);
	for(int i=0; i<10; i++){//Try 10 times for data to become available
		if (I2c.available()){
			temp_beer = I2c.receive();
		}
		delay(5);
	}
	
	I2c.read(ambient, 1);//Get the temperature
	for(int i=0; i<10; i++){//Try 10 times for data to become available
		if (I2c.available()){
			temp_ambient = I2c.receive();
			return 0;//break out of FN once complete
		}
		delay(5);
	}
	//If data wasn't available after 10 attempts, Print an error, and set the state to idle
	Serial.println("ERROR READING TEMP");
	state = idle;
	temp_ambient = sp;
	return 0;
}

void debug_print(int temp){
	if (debugging)
	{
		String sstring = "";// local state variable of type string
		String debug = "Air: ";
		switch (state){//This just gets the string representation of the state enum and could be improved
			case idle:
			sstring = "idle";
			break;
			case heating:
			sstring = "heating";
			break;
			case cooling:
			sstring = "cooling";
			break;
			case heating_db:
			sstring = "heating deadband";
			break;
			case cooling_db:
			sstring = "cooling deadband";
			break;
		}
	
		debug += temp;
		debug += " --- ";
		debug += "State: ";
		debug += sstring;
		debug += " --- Beer: ";
		debug += temp_beer;
		Serial.println(debug);
	}
}


void loop()
{
	switch (state){
		read_temp();
		case idle:
		if (sp_low < temp_ambient && temp_ambient < sp_high){
			debug_print(temp_ambient);
			digitalWrite(heater, off);
			digitalWrite(fridge, off);
			delay(1000);
			read_temp();
		}
		else if (temp_ambient >= sp_high){
			state = cooling;
		}
		else if (temp_ambient <= sp_low){
			state = heating;
		}
		break;
		
		case cooling:
		if (temp_ambient >= sp_low){
			debug_print(temp_ambient);
			digitalWrite(heater, off);
			digitalWrite(fridge, on);
			delay(1000);
			read_temp();
		}
		else{
			state = cooling_db;
		}
		break;
		
		
		case heating:
		if (temp_ambient <= sp_high){
			debug_print(temp_ambient);
			digitalWrite(heater, on);
			digitalWrite(fridge, off);
			delay(1000);
			read_temp();
		}
		else{
			state = heating_db;
		}
		break;
		
		
		case cooling_db:
		if (temp_ambient < sp && temp_ambient > sp_low - 3){
			debug_print(temp_ambient);
			digitalWrite(heater, off);
			digitalWrite(fridge, off);
			delay(1000);
			read_temp();
		}
		else{
			state = idle;
		}
		break;
		
		
		case heating_db:
		if (temp_ambient > sp && temp_ambient < sp_high + 3){
			debug_print(temp_ambient);
			digitalWrite(heater, off);
			digitalWrite(fridge, off);
			delay(1000);
			read_temp();
		}
		else{
			state = idle;
		}
		break;
	}
	
}
