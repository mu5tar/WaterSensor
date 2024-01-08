
#ifndef CONFIGRATION_H                                       
#define CONFIGRATION_H                                       

//Macros index for GetTime Array
#define currentYear     0
#define currentMonth    1
#define currentDay      2
#define currentHour     3
#define currentMinute   4
#define currentSecond   5

//Macros of connection and DB configration
#define ssid  "Vodafone_VDSL"
#define password  "2yNG5uhSDCFcPfeS"
#define firebaseHost  "waterflowsensor-f36b6-default-rtdb.firebaseio.com"
#define firebaseAuth  "yLMGECHdv8lbpvaV3PbdQLgJcpi3rK4OevvOu4Rc"

//Macros of EEPROM Address
#define Total_Liters_EEPROMAddr                 0
#define Daily_Consumption_EEPROMAddr            10
#define Balance_EEPROMAddr                      20

#define Volve_Pin                       D1
#define Button_Pin                      D0



SoftwareSerial bluetoothSerial(3, 2);  // RX, TX pins on ESP8266

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "africa.pool.ntp.org", 7200, 60000);


FirebaseData fbdo;

const int flowSensorPin = D2;  // GPIO pin where the water flow sensor is connected
volatile int pulseCount = 0;   // count of pulses from the water flow sensor
float flowRate = 0.0;          // calculated flow rate
float totalLiters = 0.0;       // total water consumption in liters
float balance = 100.0;
float dailyConsumption = 0.0;
float totalCubicMeters = 0.0;
float remainingCubicMeters = 0.0;
unsigned long lastTime = 0;
bool _Notification = false;
int gettime[6];
int Segmant = 0;
unsigned long leakDuration = 3 * 60 ;  // Duration to consider 
bool _isLeak = false;

unsigned long tempTime=0;

#endif              