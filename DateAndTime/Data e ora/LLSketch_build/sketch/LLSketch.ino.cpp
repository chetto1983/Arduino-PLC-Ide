#include <Arduino.h>
#line 1 "C:\\Users\\Davide\\Desktop\\Data e ora\\Data e ora\\LLSketch\\LLSketch.ino"
#include <AlPlc_Opta.h>

/* opta_1.2.0
      - NTPClient (3.2.1)
*/

struct PLCSharedVarsInput_t
{
	uint32_t TimeStamp;
	bool TimeSetOk;
	bool NtpSetOk;
	bool NtpSetKo;
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	uint32_t TimeToSet;
	bool Set_Time;
	bool NTP_Set;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(-1516634858);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname
#include <NTPClient.h>
#include <EthernetUdp.h>

EthernetUDP ntpUDP;
// Set NTP  IP adress
IPAddress ip_npt (192, 168, 1, 225);
// Set NTP  on UTC
NTPClient timeClient(ntpUDP, ip_npt, -6*0, 0);
unsigned long last_timestamp = 1;
unsigned long  timestamp = 0;




#line 42 "C:\\Users\\Davide\\Desktop\\Data e ora\\Data e ora\\LLSketch\\LLSketch.ino"
void setup();
#line 59 "C:\\Users\\Davide\\Desktop\\Data e ora\\Data e ora\\LLSketch\\LLSketch.ino"
void loop();
#line 99 "C:\\Users\\Davide\\Desktop\\Data e ora\\Data e ora\\LLSketch\\LLSketch.ino"
void updateTime();
#line 42 "C:\\Users\\Davide\\Desktop\\Data e ora\\Data e ora\\LLSketch\\LLSketch.ino"
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

