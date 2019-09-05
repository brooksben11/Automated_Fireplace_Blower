#include "Particle.h"
#include <math.h>

#define Power_Pin A1
#define Temp_Pin A0

int Temp_Reading;
float TempC;
float TempF;
int Temp;
String Temp_String;
double Battery_V;
int Battery_Percent;
String Battery_String;
String Status = "Startup";
int USB_Status;
int CHG_Status;
bool Startup = true;

SYSTEM_THREAD(ENABLED);

void setup() {
    
    pinMode(Power_Pin, OUTPUT);
    pinMode(Temp_Pin, INPUT);
    pinMode(PWR, INPUT);
	pinMode(CHG, INPUT);
	
	Particle.variable("Status", Status);
	Particle.variable("Battery_Percent", Battery_Percent);
	Particle.variable("Temp", Temp);
	Particle.connect(); //added because of System_Thread(ENABLED) to keep from blocking if not connected yet
	
	if (Startup == true) {
	    waitUntil(Particle.connected); 
	    Startup = false;
	}
	//Since the loop doesn't run long enough to connect to the cloud, force a connection at the beginning
	//Had issues with the sleep timer without this added. Seems it calibrates it's internal clock during the cloud connection process?
	//Also seems that the setup runs after every sleep cycle
	Time.zone(-5);
}

void loop() {
    
    digitalWrite(Power_Pin, HIGH);
    delay(250); //Delay to give the temp sensor time to 'warm-up'. Occassionally worked with 100, worked with 500 and 300
    Temp_Reading = analogRead(Temp_Pin);
    digitalWrite(Power_Pin, LOW);
    TempC=((100*((float)Temp_Reading/4095)*3.3)-50);
    TempF=((TempC*9/5)+32);
    Temp=TempF; //Select TempF or TempC
    Temp_String = String(Temp);
    
    Battery_V = analogRead(BATT) * 0.0011224;
    USB_Status = digitalRead(PWR); // PWR: 0=no USB power, 1=USB powered
    CHG_Status = digitalRead(CHG); // CHG: 0=charging, 1=not charging
    
    if (USB_Status == 0) {
        Status = "Running on Battery";
        Battery_Percent = 157.58*Battery_V - 525.82;
        Battery_String = String(Battery_Percent);
        
        waitUntil(Mesh.ready);
        Mesh.publish("Fireplace_Temp", Temp_String);
        Mesh.publish("Fireplace_Battery", Battery_String);
        System.sleep(PWR,CHANGE,60); //Sleeps for 60 seconds. {} can be used if no wakeup pins are wanted
    }
    else if (USB_Status == 1 and CHG_Status == 0) {
        Status = "Battery Charging";
        Battery_Percent = 127.56*pow(Battery_V,2) - 846.99*Battery_V + 1405.8;
        Battery_String = String(Battery_Percent);
    }
    else if (USB_Status == 1 and CHG_Status == 1) {
        Status = "Battery Charged/Running on USB Power";
        Battery_Percent = 100;
        Battery_String = String(Battery_Percent);
    }
}