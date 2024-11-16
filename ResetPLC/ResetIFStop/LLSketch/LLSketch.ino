#include <AlPlc_Opta.h>

/* opta_1.2.0
      - Arduino_UnifiedStorage (1.1.0)
*/

struct PLCSharedVarsInput_t
{
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	int16_t Arduino_Count;
	bool PLC_Init;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(-1963310513);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname


int last_counter = 1000;
unsigned long previousMillis = 0;
const long interval = 2000;  // interval at which to blink 

void setup()
{
/*
	// Configure static IP address
	IPAddress ip(192, 168, 1, 1);
	IPAddress dns(8, 8, 8, 8);
	IPAddress gateway(192, 168, 1, 0);
	IPAddress subnet(255, 255, 255, 0);
	// If cable is not connected this will block the start of PLC with about 60s of timeout!
	Ethernet.begin(ip, dns, gateway, subnet);
*/
   

	AxelPLC.Run();
}

void loop()
{
    unsigned long currentMillis = millis();
    int counter = PLCOut.Arduino_Count;
    
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        
        if (PLCOut.PLC_Init)
        {
            if (counter != last_counter)
            {
                Serial.print("Counter: " );
                Serial.println(counter);
                last_counter = counter;
            }
            else
            {
                Serial.println("plc stop");
                NVIC_SystemReset();
                
            }
        }
        
    }
    


}
