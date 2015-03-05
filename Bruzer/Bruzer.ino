/*
 * Bruzer.ino
 *
 * Created: 3/2/2015 1:15:07 PM
 * Author: TBrink
 */ 

#include "Wire/Wire.h"
#include <stdint.h>
#include <TFT_Touch_Shield_V2-master/TFTv2.h>
#include <SPI/SPI.h>

const int address = 0x4a; //Address of TC74A2 digital temp sensor
const int sp_high = 35;//18.33; //in *C (18.33*C = 65*F)
const int sp = 31;
const int sp_low = 27;
const int heater = 12; //pin 13 is the built in LED, SEEED studio relay pin 4-7 == relay 4-1
const int fridge = 13;

//byte val = 0; //var to hold value read from i2c bus
int tempC;
int tempF;

String cmd;
String name;
String val;

boolean debugging = true;

enum State {
	idle,
	cooling,
	heating,
	cooling_db,
	heating_db
} state;

boolean on = LOW;
boolean off = HIGH;

void setup(){
	Wire.begin();
	Serial.begin(9600);
	pinMode(heater, OUTPUT);
	pinMode(fridge, OUTPUT);
	digitalWrite(fridge, LOW);
	digitalWrite(heater, LOW);
	
	tempC = read_temp();
	if (tempC > sp_high){
		state = cooling;
	}
	if (tempC < sp_low){
		state = heating;
	}
	else{
		state = idle;
	}
	Serial.println("\n\n\nSetup Complete.  ");
	debug_print(tempC);
//	init_display();
	delay(2000);
}

int read_temp(){
	int temp;
	
	//start the transmission
	Wire.beginTransmission(address);

	//Tell the sensor to send data
	Wire.write(0);

	Wire.requestFrom(address, 1);
	for(int i=0; i<10; i++){
		if (Wire.available()){
			temp = Wire.read();
			//tempF = tempC * 1.8 + 32;
			return temp;
		}
	}
	//    Serial.print("Current temp : ");
	Serial.println("ERROR READING TEMP");
	return sp;
}

/*void init_display(){
	
	TFT_BL_ON;      // turn on the background light
	
	//Draw the increase & decrease SP buttons
	Tft.fillRectangle(166, 0, 74, 160, RED);
	Tft.fillRectangle(165, 160, 75, 160, BLUE);
	
	//Rotate display for text rendering
	Tft.setDisplayDirect(DOWN2UP);
	
	//Draw text
	Tft.drawString("Setpoint:", 2, 319, 2, WHITE);
	Tft.drawString("Temp:", 25, 319, 2, WHITE);
}*/

void debug_print(int temp){
	if (debugging)
	{
		String sstring = "";
		String debug = "Temp: ";
		switch (state){
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
		Serial.println(debug);
	}
}


void loop()
{
	
	switch (state){
		tempC = read_temp();
		case idle:
		if (sp_low < tempC && tempC < sp_high){
			debug_print(tempC);
			digitalWrite(heater, off);
			digitalWrite(fridge, off);
			delay(1000);
			tempC = read_temp();
		}
		else if (tempC >= sp_high){
			state = cooling;
		}
		else if (tempC <= sp_low){
			state = heating;
		}
		
		break;
		
		
		case cooling:
		if (tempC >= sp_low){
			debug_print(tempC);
			digitalWrite(heater, off);
			digitalWrite(fridge, on);
			delay(1000);
			tempC = read_temp();
		}
		else{
			state = cooling_db;
		}
		break;
		
		
		case heating:
		if (tempC <= sp_high){
			debug_print(tempC);
			digitalWrite(heater, on);
			digitalWrite(fridge, off);
			delay(1000);
			tempC = read_temp();
		}
		else{
			state = heating_db;
		}
		break;
		
		
		case cooling_db:
		if (tempC < sp && tempC > sp_low - 2){
			debug_print(tempC);
			digitalWrite(heater, off);
			digitalWrite(fridge, off);
			delay(1000);
			tempC = read_temp();
		}
		else{
			state = idle;
		}
		break;
		
		
		case heating_db:
		if (tempC > sp && tempC < sp_high + 2){
			debug_print(tempC);
			digitalWrite(heater, off);
			digitalWrite(fridge, off);
			delay(1000);
			tempC = read_temp();
		}
		else{
			state = idle;
		}
		break;
	}
	
}
