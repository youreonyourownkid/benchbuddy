#!/usr/bin/env python3

#include <TFT_eSPI.h>
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// network variables
const char* ssid = "hotspot9486";   // network SSID 
const char* password = "zn2wg8vp";  // network password 

String host = "3.133.141.196";   // IP address for AWS instance
const int port = 5000;                // standard port for web traffic
String deviceID = "R";

// angle sensor
Adafruit_MMA8451 mma = Adafruit_MMA8451();
// display
TFT_eSPI tft = TFT_eSPI(TFT_WIDTH, TFT_HEIGHT);

// Circle variables
const int CIRCLE_RADIUS = 20;
const int MIN_ANGLE = -60;
const int MAX_ANGLE = 60;
const int TARGET_ANGLE = 15;
const int ERROR_ALLOWED = 4;
const int CIRCLE_Y = TFT_HEIGHT / 2;
const int DELAY = 100;

void makeCircle(int x, uint16_t color)
{
	tft.fillCircle(x, CIRCLE_Y, CIRCLE_RADIUS, color); 
}

float getAngle() {
	sensors_event_t event;
	mma.read();
	mma.getEvent(&event);
	
	float x = event.acceleration.x;
	float y = event.acceleration.y;
	float z = event.acceleration.z;
	
	// float pitch = atan2(x, sqrt(y * y + z * z));
	// float roll = atan2(y, sqrt(x * x + z * z));
	float deviation = atan2(sqrt(x * x + y * y), z);
	
	// float pitch_degrees = pitch * 180.0 / PI;
	// float roll_degrees = roll * 180.0 / PI;
	float deviation_degrees = deviation * 180.0 / PI;
	
	deviation_degrees -= 90;
	return deviation_degrees;
}

void setup() {
	Serial.begin(9600);
	
	// display setup, and make display green while other setup is happening
	tft.init(); 
	tft.fillScreen(TFT_GREEN); 
	
	// network setup
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.println("Connecting to WiFi...");
	}
	Serial.println("Connected to WiFi");
	
	
	// angle sensor setup
	while (!Serial);
	if (!mma.begin()) {
		Serial.println("Could not find a valid MMA8451 sensor, check wiring!");
		while (1);
	}
	mma.setRange(MMA8451_RANGE_2_G);
	tft.fillScreen(TFT_BLACK); 
}

void loop() {
	// vertical = 0 degres
	int degree = int(getAngle());
	degree -= TARGET_ANGLE;  // this is a correction
	// Serial.print("<d:" + String(degree) + "> ");
	
	// figure out the color of the ball
	uint16_t color = TFT_WHITE;
	if (degree > ERROR_ALLOWED) {
		color = TFT_RED;
	} else if (degree < -ERROR_ALLOWED) {
		color = TFT_BLUE;
	}
	
	// find screenX value by adding the angle to half of the width of the display to display
	int screenX = degree + (TFT_WIDTH / 2);
	if (screenX < 0) {
		screenX = 0;
	}
	if (screenX >= TFT_WIDTH) {
		screenX = TFT_WIDTH-1;
	}
	String varvalue = deviceID + "_" + String(degree);
	// Serial.print(varvalue + " ");
	
	// show circle on TTGO display
	tft.fillScreen(TFT_BLACK); 
	makeCircle(screenX, color); 
	
	// Construct the string that will get added to the url after ?var=
	String url = "http://" + host + ":" + String(port) + "/?var=" + varvalue;
	
	// Send the HTTP GET request
	HTTPClient http;
	http.begin(url);
	int httpResponseCode = http.GET();
	if (httpResponseCode != 0) {
		// Serial.print("HTTP Response code: ");
		// Serial.println(httpResponseCode);
		String payload = http.getString();
		// Serial.println("Response from AWS Server: " + payload);
	}
	else {
		Serial.print("Error code: ");
		Serial.println(httpResponseCode);
	}
	http.end();
	
	delay(DELAY); 
}
