#include <Arduino.h>
#line 1 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
#include <AlPlc_Opta.h>

/* opta_1.2.0
      - NTPClient (3.2.1)
      - Arduino_UnifiedStorage (1.1.0)
      - Arduino_POSIXStorage (1.2.1)
      - Arduino_USBHostMbed5 (0.3.1)
*/

struct PLCSharedVarsInput_t
{
	uint32_t TimeStamp;
	bool TimeSetOk;
	bool NtpSetOk;
	bool NtpSetKo;
	bool UsbAvaiable;
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	uint32_t TimeToSet;
	bool Set_Time;
	bool NTP_Set;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(-1173833909);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname
#include <NTPClient.h>
#include <EthernetUdp.h>
#include <Arduino_UnifiedStorage.h>

EthernetUDP ntpUDP;
// Set NTP  IP adress
IPAddress ip_npt (192, 168, 1, 225);
// Set NTP  on UTC
NTPClient timeClient(ntpUDP, ip_npt, -6*0, 0);
unsigned long last_timestamp = 1;
unsigned long  timestamp = 0;


// Timing control variables
unsigned long previousMillis = 0;
const long interval = 1000;


// USB storage and folder instances
USBStorage usbStorage;
Folder backupFolder = Folder();
bool usbIntialized = false;
volatile bool usbAvailable = false;


#line 57 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void connectionCallback();
#line 70 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void disconnectionCallback();
#line 77 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void writeToUSB();
#line 104 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void performUpdate();
#line 124 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void setup();
#line 146 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void loop();
#line 188 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void updateTime();
#line 57 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\DataLogger\\DataLogger\\LLSketch\\LLSketch.ino"
void connectionCallback() {
    usbAvailable = true;
    Arduino_UnifiedStorage::debugPrint("- USB device connected!");
    usbStorage.removeOnConnectCallback();
}

/**
  Function to handle USB disconnection callback.
  Resets the USB available flag and reinstalls the connection callback.

  @param none
  @return none
*/
void disconnectionCallback() {
    usbAvailable = false;
    Arduino_UnifiedStorage::debugPrint("- USB device disconnected!");
    usbStorage.onConnect(connectionCallback);
}


void writeToUSB() {
  if (usbAvailable && !usbIntialized) {
    usbStorage.begin();
    Folder usbRoot = usbStorage.getRootFolder();
    String folderName = "backup_data";
    backupFolder = usbRoot.createSubfolder(folderName);
    Arduino_UnifiedStorage::debugPrint("- Backup folder created: " + backupFolder.getPathAsString());
    usbStorage.unmount();
    usbIntialized = true;
  } else if (usbAvailable && usbIntialized) {
    if (!usbStorage.isMounted()) {
      if (usbStorage.begin()) {
        performUpdate();
      }
    } else if (usbStorage.isMounted()) {
      performUpdate();
    }
  }
}

/**
  Function to perform data update on USB storage.
  Writes analog sensor data in a formatted string to a text file.

  @param none
  @return none
*/
void performUpdate() {
  UFile backupFile = backupFolder.createFile("analog_inputs_data.txt", FileMode::APPEND);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;

    String buffer = "PIPPO";
    buffer += "\n";
    backupFile.write(buffer);
    Arduino_UnifiedStorage::debugPrint("- Data written to file: " + buffer);
  }
  
  backupFile.close();
  usbStorage.unmount();
  Arduino_UnifiedStorage::debugPrint("- File closed and USB storage unmounted!");
}



void setup()
{
    
	// Configure static IP address
	IPAddress ip(192, 168, 1, 12);
	IPAddress dns(8, 8, 8, 8);
	IPAddress gateway(192, 168, 1, 1);
	IPAddress subnet(255, 255, 255, 0);
	// If cable is not connected this will block the start of PLC with about 60s of timeout!
	Ethernet.begin(ip, dns, gateway, subnet);
	timeClient.begin();
      
      
    Arduino_UnifiedStorage::debuggingModeEnabled = true;
    usbStorage = USBStorage();
    usbStorage.onConnect(connectionCallback);
    usbStorage.onDisconnect(disconnectionCallback);
   

	AxelPLC.Run();
}

void loop()
{
  // set time manualy
  if(PLCOut.Set_Time)
  {
      set_time(PLCOut.TimeToSet);
      PLCIn.TimeSetOk = true;
      
  }
  else
  {
     PLCIn.TimeSetOk = false; 
  }
  
  //  read rtc
   time_t t = time(NULL);
   timestamp = (unsigned long) t;
  
  // sent time to plc
  if (timestamp != last_timestamp)
  {
      PLCIn.TimeStamp = t;
      last_timestamp = timestamp;
  }
  
  // set time by ntp
  if (PLCOut.NTP_Set)
  {
      updateTime();
  }
  else
  {
      PLCIn.NtpSetOk = false;
      PLCIn.NtpSetKo = false;
  }
  
  PLCIn.UsbAvaiable = usbAvailable;
  
}



void updateTime() 
{
  timeClient.update();
   if(timeClient.isTimeSet())
      {
          const unsigned long epoch = timeClient.getEpochTime();
          set_time(epoch);
          PLCIn.NtpSetOk = true;
      }
      else
      {
          PLCIn.NtpSetKo = true;
      }
}

