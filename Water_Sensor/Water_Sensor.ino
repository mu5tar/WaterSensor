#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

//#include "Config.h"
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Set the LCD address to 0x27 for a 4x20 LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long dataMillis = 0;
int count = 0;

#define Volve_isOpen 1
#define Volve_isClose 2


//Macros of connection and DB configration
// #define ssid "WE"
// #define password ".ppppppp"
// #define ssid "Vodafone_VDSL"
// #define password "2yNG5uhSDCFcPfeS"
// #define firebaseHost "waterflowsensor-f36b6-default-rtdb.firebaseio.com"
// #define firebaseAuth "yLMGECHdv8lbpvaV3PbdQLgJcpi3rK4OevvOu4Rc"
// #define API_KEY "AIzaSyBjXR0v7x2SvG2oHYCOLyS4yfWYAVKRQtw"
// #define DATABASE_SECRET "yLMGECHdv8lbpvaV3PbdQLgJcpi3rK4OevvOu4Rc"
// #define USER_EMAIL "mu55tar266@gmail.com"
// #define USER_PASSWORD "mokh7656261"

#define API_KEY "AIzaSyA-QKSXQR_atzj3CmpBJDMQTAGttfG5aRM"
/* 4. If work with RTDB, define the RTDB URL */
#define DATABASE_URL "https://watersensor-16697-default-rtdb.firebaseio.com/"  //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app


#define DATABASE_SECRET "DATABASE_SECRET"
//Macros of EEPROM Address
#define Total_Liters_EEPROMAddr 0
#define Daily_Consumption_EEPROMAddr 10
#define Balance_EEPROMAddr 20
#define Consumption_Capacity_EEPROMAddr 30
#define SSID_WIFI_EEPROMAddr 100
#define PASWORD_WIFI_EEPROMAddr 150
#define USER_EMAIL_EEPROMAddr 200
#define USER_PASSWORD_EEPROMAddr 250
#define USER_UID_EEPROMAddr 300
#define Flag_Mode_EEPROMAddr 350
#define Choise_Mode_EEPROMAddr 400
#define Valve_State_EEPROMAddr 410

#define Volve_open_Pin 10
#define Volve_close_Pin 9
#define flowSensorPin D3
#define chipSelect D8

File LogFile;

SoftwareSerial bluetoothSerial(3, 2);  // RX, TX pins on ESP8266




FirebaseData fbdo;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;

const int dirPin = 9;
const int stepPin = 10;
const int stepsPerRevolution = 50;
int stepDelay = 5000;

bool flag_mode;
int Volve_Satate=1;
volatile int pulseCount = 0;  // count of pulses from the water flow sensor
float flowRate = 0.0;         // calculated flow rate
float totalLiters = 0.0;      // total water consumption in liters
float Consumption_Capacity = 0.0;
float balance = 100.0;
float dailyConsumption = 0.0;
float Totel_Cubic_Meter = 0.0;
float Remain_Cubic_Meters = 0.0;
unsigned long lastTime = 0;
bool _Notification = false;

int Segmant = 0;
unsigned long leakDuration = 3 * 60;  // Duration to consider
bool _isLeak = false;

unsigned long tempTime = 0;
unsigned long tempTime_2 = 0;
String ssid = "Vodafone_VDSL";
String password = "2yNG5uhSDCFcPfeS";
String USER_EMAIL = "mu55tar266@gmail.com";
String USER_PASSWORD = "mokh7656261";
String USER_UID = "";
unsigned long startTime;
const unsigned long timeoutPeriod = 50000;
RTC_DS1307 rtc;
DateTime now;
int Choise_Mode = 0;

//************************************************************************************************************************************************
void Init_SD() {
  SPI.begin();
  //Setup for the SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}
//************************************************************************************************************************************************
void setup() {
  Serial.begin(115200);
  bluetoothSerial.begin(9600);
  EEPROM.begin(1024);
  // Init_RTC();
  flag_mode = EEPROM.read(Flag_Mode_EEPROMAddr);
  String UID = readStringFromEEPROM(USER_UID_EEPROMAddr);
  Volve_Satate = EEPROM.read(Valve_State_EEPROMAddr);
  // Initialize the LCD
  lcd.init();
  lcd.begin(16, 2);  // Initialize the LCD
  lcd.backlight();   // Turn on the backlight
  lcd.clear();

  //pinMode(Button_Pin, INPUT);

  if (flag_mode == 0 || UID == "") {
    Configration_Mode();
  } else {
    App_Mode();
  }
}
//************************************************************************************************************************************************
void App_Mode() {

  pinMode(flowSensorPin, INPUT_PULLUP);
  // Declare pins as Outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  Open_Valve();

  Consumption_Capacity = readFloatFromEEPROM(Consumption_Capacity_EEPROMAddr);
  totalLiters = readFloatFromEEPROM(Total_Liters_EEPROMAddr);
  balance = readFloatFromEEPROM(Balance_EEPROMAddr);
  dailyConsumption = readFloatFromEEPROM(Daily_Consumption_EEPROMAddr);

  String Valid_USER_UID = readStringFromEEPROM(USER_UID_EEPROMAddr);
  Serial.println("\n1-Online\n2-Offline");
  bluetoothSerial.println("\n1-Online\n2-Offline");
  lcd.setCursor(0, 0);
  lcd.print("Online Enter 1");
  lcd.setCursor(0, 1);
  lcd.print("Offline Enter 2");
  delay(1000);
  Serial.print("Waiting for Choice");


  startTime = millis();
  while (Choise_Mode == 0) {
    if (millis() - startTime < timeoutPeriod) {
      Serial.print('.');
      delay(500);

      if (bluetoothSerial.available() > 0) {
        delay(10);
        Choise_Mode = bluetoothSerial.parseInt();
        delay(100);
        EEPROM.write(Choise_Mode_EEPROMAddr, Choise_Mode);
        EEPROM.commit();
      }
    } else {
      Choise_Mode = EEPROM.read(Choise_Mode_EEPROMAddr);
      // break;
    }
    if (Choise_Mode == 1) {
      delay(100);
      bluetoothSerial.println("Online Mode");
      Serial.println("Online Mode");

      connectToWiFi();

      if (signIn()) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Authentication done");
        bluetoothSerial.println("Authentication done");
        Serial.println("Authentication done");
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Authentication Failed");
        bluetoothSerial.println("Authentication Failed");
        Serial.println("Authentication Failed");
        delay(5000);
        ESP.restart();
      }
    } else if (Choise_Mode == 2) {
      bluetoothSerial.println("Offline Mode");
      Serial.println("Offline Mode");
    }
  }

  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
}
//************************************************************************************************************************************************

void Configration_Mode() {


  for (byte i = 0; i < 20; i++) {
    lcd.setCursor(0, 0);
    lcd.print("Config Mode");
    lcd.setCursor(15 - i, 1);
    lcd.print("Must Connect with Blu");
    delay(800);
  }
  Serial.println("Config_mode_loop");
}
//************************************************************************************************************************************************
void Config_mode_loop() {

  //while (millis() - startTime > timeoutPeriod) {
  bluetoothSerial.println("\n1-WIFI SSID    2-WIFI Password\n3-USER Email    4-USER Password\n5-Connect to Database\n6-Restart\n*******************************");
  delay(1000);
  if (bluetoothSerial.available() > 0) {

    int choise = bluetoothSerial.parseInt();
    switch (choise) {
      case 1:
        bluetoothSerial.println("Please Enter WIFI SSID : ");


        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String SSID = bluetoothSerial.readString();

          writeStringToEEPROM(SSID_WIFI_EEPROMAddr, removeSpaces(SSID));
          // Print the received data to the Serial Monitor
          Serial.println("SSID: ");
          Serial.println(removeSpaces(SSID).length());
        }
        break;
      case 2:
        bluetoothSerial.println("Please Enter WIFI PASSWORD : ");


        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String PASSWORD = bluetoothSerial.readString();

          writeStringToEEPROM(PASWORD_WIFI_EEPROMAddr, removeSpaces(PASSWORD));
          // Print the received data to the Serial Monitor
          Serial.println("PASSWORD:");
          Serial.println(removeSpaces(PASSWORD).length());
        }
        break;
      case 3:
        bluetoothSerial.println("Please Enter User Email : ");


        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String User_Email = bluetoothSerial.readString();

          writeStringToEEPROM(USER_EMAIL_EEPROMAddr, removeSpaces(User_Email));
        }

        break;
      case 4:
        bluetoothSerial.println("Please Enter User Password : ");


        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String User_Password = bluetoothSerial.readString();

          writeStringToEEPROM(USER_PASSWORD_EEPROMAddr, removeSpaces(User_Password));
        }

        break;
      case 5:
        connectToWiFi();
        if (signIn()) {
          writeStringToEEPROM(USER_UID_EEPROMAddr, removeSpaces(USER_UID));
          bluetoothSerial.println("Authentication done");
          Serial.println("Authentication done");
          EEPROM.write(Flag_Mode_EEPROMAddr, 1);
          EEPROM.commit();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Authentication done");
          lcd.setCursor(0, 1);
          lcd.print("Restart to go to Main app");
          delay(2000);
          writeFloatToEEPROM(Consumption_Capacity_EEPROMAddr, 0.0);
          writeFloatToEEPROM(Total_Liters_EEPROMAddr, 0.0);
          writeFloatToEEPROM(Daily_Consumption_EEPROMAddr, 0.0);
          writeFloatToEEPROM(Balance_EEPROMAddr, 0.0);
        } else {
          bluetoothSerial.println("authentication failed....Check Email & passw");
          Serial.println("authentication failed....Check Email & passw");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("authentication failed");
          lcd.setCursor(0, 1);
          lcd.print("Check Email & passw");
          delay(2000);
        }

        break;
      case 6:
        bluetoothSerial.println("Restart");
        Serial.println("Restart");
        ESP.restart();
        break;
      default:
        choise = 0;
    }
    //startTime = timeoutPeriod;
  } else {
    //startTime = millis();
  }
  //}
  //ESP.deepSleep(10e6);
}
//************************************************************************************************************************************************
void Online_App_mode_loop() {

  //now = rtc.now();
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTime;
  if (elapsedTime > 1000) {  // Calculate flow rate every second
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));

    flowRate = (pulseCount / 5.5);
    totalLiters += flowRate / 60.0;

    Totel_Cubic_Meter = totalLiters / 1000.0;
    Consumption_Capacity += Totel_Cubic_Meter;
    lastTime = currentTime;
    pulseCount = 0;
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  }
  //Leak_Detiction();

  calculateBalance();
  calculateRemainingCubicMeters();
  printResults();
  StorInEEPROM();
  //checks();

  Read_Balance_From_Blu();

  sendDataToFirebase();

  LCD_Print();
}
//************************************************************************************************************************************************
void Offline_App_mode_loop() {
  //now = rtc.now();
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - lastTime;
  if (elapsedTime > 1000) {  // Calculate flow rate every second
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));

    flowRate = (pulseCount / 5.5);
    totalLiters += flowRate / 60.0;

    Totel_Cubic_Meter = totalLiters / 1000.0;
    Consumption_Capacity += Totel_Cubic_Meter;
    lastTime = currentTime;
    pulseCount = 0;
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  }
  //Leak_Detiction();
  calculateBalance();
  calculateRemainingCubicMeters();
  printResults();
  StorInEEPROM();
  //checks();
  Read_Balance_From_Blu();
  LCD_Print();
}
//************************************************************************************************************************************************
void loop() {
  if (flag_mode == 0) {
    Config_mode_loop();
  } else {
    if (Choise_Mode == 1) {
      Online_App_mode_loop();
    } else if (Choise_Mode == 2) {
      Offline_App_mode_loop();
    }
  }
}
//************************************************************************************************************************************************
ICACHE_RAM_ATTR void pulseCounter() {
  pulseCount++;
}
//************************************************************************************************************************************************
void Leak_Detiction() {
  // ckeck leak Detiction
  if (flowRate > 0.0) {
    if (flowRate <= 0.25) {
      tempTime += (60 - now.second());
      if (tempTime >= 3600) {
        _isLeak = true;
        bluetoothSerial.println("Warning: There is a leak. Please check the water network..!!!!");
      }

      else if (tempTime >= 7200) {
        _isLeak = true;
        bluetoothSerial.println("Warning: There is a leak. Please check the water network..!!!!");
        Close_Valve();  // setAlarm and close water
      }
    }

    else if (flowRate > 0.25) {
      tempTime_2 += (60 - now.second());
      if (tempTime_2 >= 1800) {
        _isLeak = true;
        bluetoothSerial.println("Warning: There is a leak. Please check the water network..!!!!");
        Close_Valve();  // setAlarm and close water
      }
    }
  } else {
  }
}
//************************************************************************************************************************************************
void checks() {
  //chekc end day to stor daily Consumption and reset value to stor a new values
  if (now.hour() == 23 && now.minute() == 59 && now.second() >= 55) {
    String Date = now.timestamp(DateTime::TIMESTAMP_DATE);
    if (Choise_Mode == 1) {
      json.set("/Daily Consumption/" + Date, (dailyConsumption / 1000.0));
      LogFile = SD.open("dailyConsumption.txt", FILE_WRITE);
      LogFile.print(Date);
      LogFile.print("->");
      LogFile.println((dailyConsumption / 1000.0));
      LogFile.close();
    } else if (Choise_Mode == 2) {
      LogFile = SD.open("dailyConsumption.txt", FILE_WRITE);
      LogFile.print(Date);
      LogFile.print("->");
      LogFile.println((dailyConsumption / 1000.0));
      LogFile.close();
    }

    dailyConsumption = 0.0;

  } else {

    dailyConsumption += flowRate / 60.0;
    //Firebase.setFloat(fbdo, "/Daily Consumption/" + Day + "-" + Month, dailyConsumption);
  }
  //chekc end Month to go back the first segment
  if (now.day() == 1 && now.hour() == 0 && now.minute() == 0 && now.second() <= 3) {

    totalLiters = 0.0;
  }

  //chekc Remaining Consumption
  if ((Remain_Cubic_Meters) <= ((Totel_Cubic_Meter / now.day()) * 2)) {
    _Notification = true;


  } else {
    _Notification = false;
  }
}
//************************************************************************************************************************************************
void calculateBalance() {

  if (Totel_Cubic_Meter <= 10.0) {
    balance -= (flowRate / 60.0 / 1000.0) * 0.65;
    Segmant = 1;
  } else if (Totel_Cubic_Meter <= 20.0) {
    balance -= (flowRate / 60.0 / 1000.0) * 1.60;
    Segmant = 2;
  } else if (Totel_Cubic_Meter <= 30.0) {
    balance -= (flowRate / 60.0 / 1000.0) * 2.25;
    Segmant = 3;
  } else if (Totel_Cubic_Meter <= 40.0) {
    balance -= (flowRate / 60.0 / 1000.0) * 2.75;
    Segmant = 4;
  }
}
//************************************************************************************************************************************************
void calculateRemainingCubicMeters() {

  if (Totel_Cubic_Meter <= 10.0) {
    Remain_Cubic_Meters = balance / 0.65;
  } else if (Totel_Cubic_Meter <= 20.0) {
    Remain_Cubic_Meters = balance / 1.60;
  } else if (Totel_Cubic_Meter <= 30.0) {
    Remain_Cubic_Meters = balance / 2.25;
  } else if (Totel_Cubic_Meter <= 40.0) {
    Remain_Cubic_Meters = balance / 2.75;
  }
}
//************************************************************************************************************************************************
void sendDataToFirebase() {
  String path = "/UsersData/";
  path += auth.token.uid.c_str();

  if (millis() - dataMillis > 5000 && Firebase.ready()) {
    dataMillis = millis();

    json.set("/Balance", balance);
    json.set("/Totel Cubic Meter", Totel_Cubic_Meter);
    json.set("/Remain Cubic Meters", Remain_Cubic_Meters);
    json.set("/Charge Balance Detection", _Notification);
    json.set("/Leak Detection", _isLeak);
    json.set("/Volve State", Volve_Satate);
    //<- user uid of current user that sign in with Emal/Password
    Serial.printf("Set int... %s\n", Firebase.setJSON(fbdo, path, json) ? "ok" : fbdo.errorReason().c_str());
      // Serial.printf("Get bool ref... %s\n", Firebase.getBool(fbdo, (path + "/Volve State"), &Volve_Satate) ? Volve_Satate ? "true" : "false" : fbdo.errorReason().c_str());

  }
}
//************************************************************************************************************************************************
void StorInEEPROM() {
  writeFloatToEEPROM(Consumption_Capacity_EEPROMAddr, Consumption_Capacity);
  writeFloatToEEPROM(Total_Liters_EEPROMAddr, totalLiters);
  writeFloatToEEPROM(Daily_Consumption_EEPROMAddr, dailyConsumption);
  writeFloatToEEPROM(Balance_EEPROMAddr, balance);
}
//************************************************************************************************************************************************
void printResults() {
  Serial.print("Flow Rate: ");
  Serial.print(flowRate);
  Serial.print(" L/min\t");
  Serial.print("Total Cubic Meters: ");
  Serial.print(Totel_Cubic_Meter);
  Serial.print(" m^3\t");

  Serial.print("Remain Cubic Meters: ");
  Serial.print(Remain_Cubic_Meters);
  Serial.print(" m^3 \t");
  Serial.print("Balance: ");
  Serial.print(balance);
  Serial.print(" EGP \t");
  Serial.print("Segmant:");
  Serial.println(Segmant);
}
//************************************************************************************************************************************************
// void GetTime(int arr[]) {
//   unsigned long epochTime = timeClient.getEpochTime();
//   tmElements_t timeInfo;
//   if (epochTime > 0) {
//     breakTime(epochTime, timeInfo);
//     arr[currentYear] = timeInfo.Year + 1970;  // Years since 1970
//     arr[currentMonth] = timeInfo.Month;
//     arr[currentDay] = timeInfo.Day;
//     arr[currentHour] = timeInfo.Hour;
//     arr[currentMinute] = timeInfo.Minute;
//     arr[currentSecond] = timeInfo.Second;
//   }
// }
//************************************************************************************************************************************************
void connectToWiFi() {
  ssid = readStringFromEEPROM(SSID_WIFI_EEPROMAddr);
  password = readStringFromEEPROM(PASWORD_WIFI_EEPROMAddr);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");
  startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime < timeoutPeriod) {
      delay(1000);
      Serial.print(".");
      lcd.print(".");
      startTime = millis();
    } else {
      Serial.print("SSID or Passw not valid");
      lcd.setCursor(0, 0);
      lcd.print("SSID or Passw not valid");
      delay(2000);
      ESP.restart();
      //break;
    }
  }
  Serial.println("\nWiFi Connected Done...");
  lcd.setCursor(0, 1);
  lcd.print("WiFi Connected");
  delay(2000);
}
//************************************************************************************************************************************************
void writeStringToEEPROM(int address, String data) {
  int length = data.length();
  EEPROM.write(address, length);  // Write the length of the string

  for (int i = 0; i < length; i++) {
    EEPROM.write(address + 1 + i, data[i]);  // Write each character of the string
  }
  EEPROM.commit();
}
//************************************************************************************************************************************************
String readStringFromEEPROM(int address) {
  int length = EEPROM.read(address);  // Read the length of the string

  String data = "";
  for (int i = 1; i <= length; i++) {
    char character = EEPROM.read(address + i);  // Read each character of the string
    data += character;
  }

  return data;
}
//************************************************************************************************************************************************
void writeFloatToEEPROM(int address, float value) {
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    EEPROM.write(address + i, *p++);
  }
  EEPROM.commit();
}
//************************************************************************************************************************************************
float readFloatFromEEPROM(int address) {
  float value;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    *p++ = EEPROM.read(address + i);
  }
  return value;
}
//************************************************************************************************************************************************
String removeSpaces(const String& inputString) {
  String resultString = "";

  for (int i = 0; i < inputString.length(); i++) {
    char currentChar = inputString.charAt(i);
    // Serial.println(int(currentChar));

    // Add non-space characters to the result string
    if (currentChar != '\0') {
      resultString += currentChar;
      //Serial.println(resultString);
    }
  }

  return resultString;
}
//************************************************************************************************************************************************
void Read_Balance_From_Blu() {
  bluetoothSerial.println("1 - To Set WIFI SSID\n2 - To Set WIFI Password\n3 - To Recharge the balance\n4 - Open Valve\n5 - Close Valve");
  bluetoothSerial.println("6 - To Set USER Email\n7 - To Set USER Password\n8 - To Go Configration Mode\n9 - To Show Resultes\n10 - To Restart");
  bluetoothSerial.println("**********************");
  bluetoothSerial.print("Valve State = ");
  bluetoothSerial.println(Volve_Satate);
  bluetoothSerial.println("**********************");
  if (bluetoothSerial.available() > 0) {

    int choise = bluetoothSerial.parseInt();
    switch (choise) {
      case 1:
        bluetoothSerial.println("Please Enter WIFI SSID : ");
        delay(100);

        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String SSID = bluetoothSerial.readString();

          writeStringToEEPROM(SSID_WIFI_EEPROMAddr, removeSpaces(SSID));
          // Print the received data to the Serial Monitor
          Serial.println("SSID: ");
          Serial.println(removeSpaces(SSID).length());
        }
        break;
      case 2:
        bluetoothSerial.println("Please Enter WIFI PASSWORD : ");
        delay(100);

        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String PASSWORD = bluetoothSerial.readString();

          writeStringToEEPROM(PASWORD_WIFI_EEPROMAddr, removeSpaces(PASSWORD));
          // Print the received data to the Serial Monitor
          Serial.println("PASSWORD:");
          Serial.println(removeSpaces(PASSWORD).length());

          //delay(5000);
        }
        break;
      case 3:
        bluetoothSerial.println("Please Enter the Balance : ");
        delay(100);
        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          float receivedData = bluetoothSerial.parseFloat();
          balance += receivedData;
          writeFloatToEEPROM(Balance_EEPROMAddr, balance);
          // Print the received data to the Serial Monitor
          Serial.print("Received Data: ");
          Serial.println(receivedData);
        }
        break;
      case 4:
        Open_Valve();
        break;
      case 5:
        Close_Valve();
        break;
      case 6:
        bluetoothSerial.println("Please Enter User Email : ");
        delay(100);

        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String User_Email = bluetoothSerial.readString();

          writeStringToEEPROM(USER_EMAIL_EEPROMAddr, removeSpaces(User_Email));
          // Print the received data to the Serial Monitor
          Serial.print("User Email: ");
          Serial.println(removeSpaces(User_Email));
        }

        break;
      case 7:
        bluetoothSerial.println("Please Enter User Password : ");
        delay(100);

        if (bluetoothSerial.available() > 0) {
          // Read the incoming decimal data
          String User_Password = bluetoothSerial.readString();

          writeStringToEEPROM(USER_PASSWORD_EEPROMAddr, removeSpaces(User_Password));
          // Print the received data to the Serial Monitor
          Serial.print("User Password: ");
          Serial.println(removeSpaces(User_Password));
        }

        break;
      case 8:
        EEPROM.write(Flag_Mode_EEPROMAddr, 0);
        EEPROM.commit();
        ESP.restart();
        break;
      case 9:
        bluetoothSerial.print("Flow Rate: ");
        bluetoothSerial.print(flowRate);
        bluetoothSerial.print(" L/min\n");
        bluetoothSerial.print("Total Cubic Meters: ");
        bluetoothSerial.print(Totel_Cubic_Meter);
        bluetoothSerial.print(" m^3\n");
        bluetoothSerial.print("Remain Cubic Meters: ");
        bluetoothSerial.print(Remain_Cubic_Meters);
        bluetoothSerial.print(" m^3 \n");
        bluetoothSerial.print("Balance: ");
        bluetoothSerial.print(balance);
        bluetoothSerial.print(" EGP \n");
        bluetoothSerial.print("Segmant:");
        bluetoothSerial.println(Segmant);

        break;
      case 10:
        // EEPROM.write(Flag_Mode_EEPROMAddr, 0);
        // EEPROM.commit();
        ESP.restart();
        break;
      default:
        choise = 0;
    }
  }
}
//************************************************************************************************************************************************
void Close_Valve() {
  if (Volve_Satate == 1) {
    //clockwise
    digitalWrite(dirPin, HIGH);

    // Spin motor
    for (int x = 0; x < stepsPerRevolution; x++) {
      
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepDelay);
    }
    Volve_Satate = 2;
    delay(1000);
    
    EEPROM.write(Valve_State_EEPROMAddr, Volve_Satate);
    EEPROM.commit();
  }
}
//************************************************************************************************************************************************
void Open_Valve() {
  delay(10);
    if (Volve_Satate == 2) {
    //clockwise
    digitalWrite(dirPin, LOW);

    // Spin motor
    for (int x = 0; x < stepsPerRevolution; x++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepDelay);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepDelay);
    }
    Volve_Satate = 1;
    delay(1000);
    
    EEPROM.write(Valve_State_EEPROMAddr, Volve_Satate);
    EEPROM.commit();
  }


  // if (Volve_Satate == Volve_isClose) {
  //   digitalWrite(dirPin, LOW);

  //   // Spin motor
  //   for (int x = 0; x < stepsPerRevolution; x++) {
  //     digitalWrite(stepPin, HIGH);
  //     delayMicroseconds(stepDelay);
  //     digitalWrite(stepPin, LOW);
  //     delayMicroseconds(stepDelay);
  //   }
  //   delay(1000);
  //   Volve_Satate = Volve_isOpen;
  //   EEPROM.write(Valve_State_EEPROMAddr, Volve_Satate);
  //   EEPROM.commit();
  // }
}
//************************************************************************************************************************************************
void LCD_Print() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Liters:");
  lcd.setCursor(7, 0);
  lcd.print(totalLiters);
  // lcd.setCursor(15, 0);
  // lcd.print("L");
  lcd.setCursor(0, 1);
  lcd.print("Flow Rate:");
  lcd.setCursor(10, 1);
  lcd.print(flowRate);

  // delay(1500);


  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("Segmant:");
  // lcd.setCursor(9, 0);
  // lcd.print(Segmant);
  // lcd.setCursor(0, 1);
  // lcd.print("Volve: ");
  // if (Volve_Satate == Volve_isOpen) {
  //   lcd.setCursor(8, 1);
  //   lcd.print("Open");
  // } else {
  //   lcd.setCursor(8, 1);
  //   lcd.print("close");
  // }

  // delay(1500);
}
//************************************************************************************************************************************************
void Init_RTC() {
  // Init RTC
  Wire.begin(D2, D1);  // SDA, SCL pins for RTC module
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC module");
    while (1)
      ;
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

//************************************************************************************************************************************************
bool signIn() {
  USER_EMAIL = readStringFromEEPROM(USER_EMAIL_EEPROMAddr);
  USER_PASSWORD = readStringFromEEPROM(USER_PASSWORD_EEPROMAddr);

  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL */
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  String base_path = "/UsersData/";

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);

  String var = "$userId";
  String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
  Firebase.setReadWriteRules(fbdo, base_path, var, val, val, DATABASE_SECRET);


  Serial.println("Getting User UID");
  startTime = millis();
  while ((auth.token.uid) == "") {
    if (millis() - startTime < timeoutPeriod) {
      Serial.print('.');
      delay(1000);
    } else {
      Serial.print("Cannot get User UID ... Ckeck Email and Password ");
      break;
    }
  }
  // Print user UID
  USER_UID = auth.token.uid.c_str();

  return Firebase.ready();
}
