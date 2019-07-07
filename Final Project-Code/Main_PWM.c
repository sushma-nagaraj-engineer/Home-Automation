/*
 * Main_PWM.c
 * 
 * Copyright 2019 <Sushma sxs173433>
 Used WiringPi Library for this project
 Referred to the link http://wiringpi.com/download-and-install/
 Referred to the sample source code at http://www.uugear.com/portfolio/read-dht1122-temperature-humidity-sensor-from-raspberry-pi/
 Referred to the Community https://raspberrypi.stackexchange.com/questions/78346/dht11-doesnt-work-on-raspberry-pi-3-stretch
 Better ways to implement: Use of ISR
 *  
 * 
 */

#include <wiringPi.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <stdint.h>  
#include <stdbool.h>

// Initialize Global Variables and GPIO pin configurations
#define MAXIMUM_TIME 85
#define HEATSENSOR_PIN 20
#define PWM_PIN 18
#define MOT_SENSOR_PIN 23
#define MOT_DET_PIN 24
//Defining size of sensor output
int value_sensor[5] = {0,0,0,0,0};
bool isDataValid = false;
//function to read value from DHT11 Sensor
void read_sensor_temp()  
{  
	// Initialization
	uint8_t state=HIGH;
	uint8_t counter=0;
	uint8_t j=0,i;
	
	// Reset the contents of value_sensor to zero
	for(i=0;i<5;i++)
		value_sensor[i]=0;
		
	// Set HEATSENSOR_PIN mode as Output so that it can be used to invoke the sensor in order to start sending data.
	pinMode(HEATSENSOR_PIN,OUTPUT);
	
	// Sending the active signal by setting it to LOW for 18 millisecond then sending a high pulse for 40 microsecond in order to wake up the slave (Sensor)
	digitalWrite(HEATSENSOR_PIN,LOW);
	delay(18);
	digitalWrite(HEATSENSOR_PIN,HIGH);
	delayMicroseconds(40);
	
	// Changing the mode to receive the response signal and serial data from DHT11
	pinMode(HEATSENSOR_PIN,INPUT);
	
	// Wait for MAXIMUM_TIME to capture serial data from Sensor
	for(i=0;i<MAXIMUM_TIME;i++)
	{
		counter=0;
		// Waiting till DHT11 sensor starts data transmission
		while(digitalRead(HEATSENSOR_PIN)==state){
			counter++;
			delayMicroseconds(1);
			if(counter==255)
				break;
		}
		state=digitalRead(HEATSENSOR_PIN);
		if(counter==255)
			break;

		// The first three transistions are ignored as they may contain invalid data thus increasing efficiency
		if((i>=4)&&(i%2==0)){
			// Reading and storing data from DHT11 sensor
			value_sensor[j/8]<<=1;
			if(counter>16)
				value_sensor[j/8]|=1;
			j++;
		}
	}
	
	// Verifying if the received data matches the checksum 
	if((j>=40)&&(value_sensor[4]==((value_sensor[0]+value_sensor[1]+value_sensor[2]+value_sensor[3])& 0xFF)))
		isDataValid = true;
	else 
		isDataValid = false;
}  

int main(void)
{
	// Initializing Raspberry Pi's GPIO Pins
	if(wiringPiSetupGpio()==-1)  
		exit(1);  
	// Set PWM_PIN mode as PWM Output
	pinMode(PWM_PIN, PWM_OUTPUT);
	// Set MOT_SENSOR_PIN mode as Input so that it can be used to receive the signal from the PIR Motion sensor
	pinMode(MOT_SENSOR_PIN,INPUT);
	// Set MOT_DET_PIN mode as Output to signal if motion has been detected
	pinMode(MOT_DET_PIN,OUTPUT);
	// Initialize Variables
	float temperature = 0.0;
	
	// This loop repeats until interrupted
	while(1)  
	{
		// Processing PIR Sensor Data
		printf("Interfacing PIR Motion Sensor With Raspberry Pi\n");
		//active low operation
		if(digitalRead(MOT_SENSOR_PIN)==LOW)
		{
			printf("Motion Detected...\n");
			digitalWrite(MOT_DET_PIN,HIGH);
		}
		else
		{
			printf("No Motion Detected...\n");
			digitalWrite(MOT_DET_PIN,LOW);
		}

		// Processing Heat Sensor Data
		// Read Data from DHT11 Sensor
		printf("Interfacing Temperature and Humidity Sensor (DHT11) With Raspberry Pi\n");
		//function call
		read_sensor_temp();  
		// delay added to wait until data has been read completely
		delay(3000);
		
		// if valid data was read
		if(isDataValid)
		{
			printf("Humidity = %d.%d %% Temperature = %d.%d *C\n",value_sensor[0],value_sensor[1],value_sensor[2],value_sensor[3]);
			//used only temperature information
			float decimalPart = value_sensor[3];
			temperature = value_sensor[2] + (decimalPart/10);
		}
		else 
		{
			printf("\tInvalid Data!!!\n");
		}
		printf("Temperature = %.1f\n",temperature);
		
		// Setting the PWM duty cycle based on temperature
		if(temperature > 60)
		{
			printf("Setting Dutycycle to :%.1f\n",97.6539);
			pwmWrite(PWM_PIN, 999);	//97.6539 % Duty Cycle
		}
		else if(temperature > 50)
		{
			printf("Setting Dutycycle to :%.1f\n",80.0586);
			pwmWrite(PWM_PIN, 819);	//80.0586% 
		}
		else if(temperature > 40)
		{
			printf("Setting Dutycycle to :%.1f\n",60.0195);
			pwmWrite(PWM_PIN, 614);	//60.0195%
		}
		else if(temperature > 30)
		{
			printf("Setting Dutycycle to :%.1f\n",40.0782);
			pwmWrite(PWM_PIN, 410);	//40.0782 % 
		}
		else
		{
			printf("Setting Dutycycle to :%.1f\n",20.0391);
			pwmWrite(PWM_PIN, 205);	//20.0391 % 
		}
	}
	return 0;
}
