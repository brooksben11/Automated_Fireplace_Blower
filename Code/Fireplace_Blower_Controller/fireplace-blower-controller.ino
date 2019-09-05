// This #include statement was automatically added by the Particle IDE.
#include <OneWire.h>

#include "DS18.h"
DS18 sensor(D0);

#define Extra_Low D1
#define Low D2
#define Medium D3
#define High D5
#define Power D4

bool Heating_Season = true;
String Mode = "Heating";
String Status = "Off";
bool Status_Change_Flag = false;
bool On = false;
int Setting = 0;
int Old_Setting = 0;
int Extra_Low_Current;
int Low_Current;
int Medium_Current;
int High_Current;
int Power_Current;
int Extra_Low_Previous;
int Low_Previous;
int Medium_Previous;
int High_Previous;
int Power_Previous;
int Fireplace_Temp;
bool Fireplace_Temp_Flag = false;
//float TempF;
//float TempC;
double Fireplace_Battery;
String FP_Temp_TimeStamp;
String FP_Battery_TimeStamp;

int Fan_TempF;

//Timer to read digital temp sensor every 3 seconds
Timer timer(3000, Read_Temp);
bool Read_Temp_Flag = false;

SYSTEM_THREAD(ENABLED);

void setup() {
    
    timer.start(); 

    pinMode(Extra_Low, INPUT);
    pinMode(Low, INPUT);
    pinMode(Medium, INPUT);
    pinMode(High, INPUT);
    pinMode(Power, INPUT);
    
    Particle.subscribe("Fan_Command", Fan_Command, MY_DEVICES);
    Particle.subscribe("Fireplace_Temp", Fireplace_Temp_Action, MY_DEVICES);
    Particle.subscribe("Fireplace_Battery", Fireplace_Battery_Action, MY_DEVICES);
    
    Particle.variable("Status", Status);
    Particle.variable("Mode", Mode);
    Particle.variable("Fireplace_Temp", Fireplace_Temp);
    Particle.variable("Fireplace_Battery", Fireplace_Battery);
    Particle.variable("FP_Temp_TimeStamp", FP_Temp_TimeStamp);
    Particle.variable("FP_Battery_TimeStamp", FP_Battery_TimeStamp);
    Particle.variable("Fan_Temp", Fan_TempF);
    
    Particle.connect(); //added because of System_Thread(ENABLED) to keep from blocking if not connected yet
    Time.zone(-5);

}

void loop() {
    
    //Read the fan temp sensor
    if (Read_Temp_Flag==true and sensor.read()) {
        Fan_TempF = sensor.fahrenheit();
        Read_Temp_Flag=false;
    }
    
    //Read all of the fan button pins to see if a button was pressed
    Extra_Low_Current = digitalRead(Extra_Low);
    Low_Current = digitalRead(Low);
    Medium_Current = digitalRead(Medium);
    High_Current = digitalRead(High);
    Power_Current = digitalRead(Power);
    
    //Series of if-statements to update fan settings and status based on button presses and current status
    if (Extra_Low_Current == 0 and Extra_Low_Current != Extra_Low_Previous and On == true) {
        Setting = 1;
        Status = "Low-Low";
    }
    else if (Low_Current == 0 and Low_Current != Low_Previous and On == true) {
        Setting = 2;
        Status = "Low";
    }
    else if (Medium_Current == 0 and Medium_Current != Medium_Previous and On == true) {
        Setting = 3;
        Status = "Medium";
    }
    else if (High_Current == 0 and High_Current != High_Previous and On == true) {
        Setting = 4;
        Status = "High";
    }
    else if (Power_Current == 0 and Power_Current != Power_Previous and On == true) {
        Setting = 0;
        Status = "Off";
        On = false;
    }
    else if (Power_Current == 0 and Power_Current != Power_Previous and On == false) {
        Setting = 5;
        Status = "High";
        On = true;
    }
    
    //Update 'previous' button values to current values
    Extra_Low_Previous = Extra_Low_Current;
    Low_Previous = Low_Current;
    Medium_Previous = Medium_Current;
    High_Previous = High_Current;
    Power_Previous = Power_Current;
    
    //Heating Mode fan speed evaluation
    if (Fireplace_Temp_Flag == true and Heating_Season == true) {
        Fireplace_Temp_Flag = false;
        if (Fireplace_Temp < 100) {
            Setting = 0;
        }
        else if (Fireplace_Temp >= 100 and Fireplace_Temp < 120) {
            Setting = 1;
        }
        else if (Fireplace_Temp >= 120 and Fireplace_Temp < 140) {
            Setting = 2;
        }
        else if (Fireplace_Temp >= 140) {
            Setting = 3;
        }
    }
    
    //Cooling Mode fan speed evaluation
    if (Fireplace_Temp_Flag == true and Heating_Season == false) {
        Fireplace_Temp_Flag = false;
        if ((Fireplace_Temp-Fan_TempF) < 5) {
            Setting = 0;
        }
        else if ((Fireplace_Temp-Fan_TempF) >= 5 and (Fireplace_Temp-Fan_TempF) < 10) {
            Setting = 1;
        }
        else if ((Fireplace_Temp-Fan_TempF) >= 10 and (Fireplace_Temp-Fan_TempF) < 15) {
            Setting = 2;
        }
        else if ((Fireplace_Temp-Fan_TempF) >= 15 and (Fireplace_Temp-Fan_TempF) < 20) {
            Setting = 3;
        }
        else if ((Fireplace_Temp-Fan_TempF) >= 20) {
            Setting = 4;
        }
    }
    
    //If-statement to check whether the setting changed, requiring a fan speed change
    if (Setting != Old_Setting) {
        Status_Change_Flag = true;
    }
    
    //If-statements to evaluate if a fan-speed change is needed
    if (Status_Change_Flag == true) {
        if (Setting == 0 and On == true) {
            pinMode(Power, OUTPUT);
            digitalWrite(Power, LOW);
            delay(75);
            pinMode(Power, INPUT);
            On = false;
            Status = "Off";
        }
        else if (Setting == 5 and On == false) {
            pinMode(Power, OUTPUT);
            digitalWrite(Power, LOW);
            delay(75);
            pinMode(Power, INPUT);
            On = true;
            Status = "High";
        }
        else if (Setting == 1) {
            if (On == false) {
                pinMode(Power, OUTPUT);
                digitalWrite(Power, LOW);
                delay(75);
                pinMode(Power, INPUT);
                On = true;
                Status = "High";
                delay(75);
                pinMode(Extra_Low, OUTPUT);
                digitalWrite(Extra_Low, LOW);
                delay(75);
                pinMode(Extra_Low, INPUT);
                Status = "Low-Low";
            }
            else {
                pinMode(Extra_Low, OUTPUT);
                digitalWrite(Extra_Low, LOW);
                delay(75);
                pinMode(Extra_Low, INPUT);
                Status = "Low-Low";
            }
        }
        else if (Setting == 2) {
            if (On == false) {
                pinMode(Power, OUTPUT);
                digitalWrite(Power, LOW);
                delay(75);
                pinMode(Power, INPUT);
                On = true;
                Status = "High";
                delay(75);
                pinMode(Low, OUTPUT);
                digitalWrite(Low, LOW);
                delay(75);
                pinMode(Low, INPUT);
                Status = "Low";
            }
            else {
                pinMode(Low, OUTPUT);
                digitalWrite(Low, LOW);
                delay(75);
                pinMode(Low, INPUT);
                Status = "Low";
            }
        }
        else if (Setting == 3) {
            if (On == false) {
                pinMode(Power, OUTPUT);
                digitalWrite(Power, LOW);
                delay(75);
                pinMode(Power, INPUT);
                On = true;
                Status = "High";
                delay(75);
                pinMode(Medium, OUTPUT);
                digitalWrite(Medium, LOW);
                delay(75);
                pinMode(Medium, INPUT);
                Status = "Medium";
            }
            else {
                pinMode(Medium, OUTPUT);
                digitalWrite(Medium, LOW);
                delay(75);
                pinMode(Medium, INPUT);
                Status = "Medium";
            }
        }
        else if (Setting == 4) {
            if (On == false) {
                pinMode(Power, OUTPUT);
                digitalWrite(Power, LOW);
                delay(75);
                pinMode(Power, INPUT);
                On = true;
                Status = "High";
            }
            else {
                pinMode(High, OUTPUT);
                digitalWrite(High, LOW);
                delay(75);
                pinMode(High, INPUT);
                Status = "High";
            }
        }
        Status_Change_Flag = false;
    }
    
    Old_Setting = Setting;
}

//Update variables based on commands published
void Fan_Command(const char *event, const char *data)
{
    Status_Change_Flag = true;
    if (strcmp(data,"Off")==0) {
        Setting = 0;
    }
    else if (strcmp(data,"On")==0) {
        Setting = 5;
    }
    else if (strcmp(data,"1")==0) {
        Setting = 1;
    }
    else if (strcmp(data,"2")==0) {
        Setting = 2;
    }
    else if (strcmp(data,"3")==0) {
        Setting = 3;
    }
    else if (strcmp(data,"4")==0) {
        Setting = 4;
    }
    else if (strcmp(data,"Heating")==0) {
        Heating_Season = true;
        Mode = "Heating";
    }
    else if (strcmp(data,"Cooling")==0) {
        Heating_Season = false;
        Mode = "Cooling";
    }
}

//Update variables and flags when fireplace temp sensor data published
void Fireplace_Temp_Action(const char *event, const char *data)
{
    String Temp_String = data;
    Fireplace_Temp = Temp_String.toInt();
    FP_Temp_TimeStamp = Time.timeStr();
    Fireplace_Temp_Flag = true;
}

//Update variables when fireplace battery level data published
void Fireplace_Battery_Action(const char *event, const char *data)
{
    String Battery_String = data;
    Fireplace_Battery = Battery_String.toInt();
    FP_Battery_TimeStamp = Time.timeStr();
}

//Timer to set a flag true for reading the digital fan temp sensor
void Read_Temp()
{
    Read_Temp_Flag = true;
}