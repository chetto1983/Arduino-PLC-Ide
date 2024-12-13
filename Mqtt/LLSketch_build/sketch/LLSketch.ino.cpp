#include <Arduino.h>
#line 1 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\Mqtt\\LLSketch\\LLSketch.ino"
#include <AlPlc_Opta.h>

/* opta_1.2.0
      - ArduinoJson (7.2.0)
      - ArduinoMqttClient (0.1.8)
*/

struct PLCSharedVarsInput_t
{
	bool Mqtt_Ok;
	bool Send_Ok;
	bool Send_Ko;
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	bool Send_Data;
	float Temperatura;
	float Umidita;
	bool SensTemp_Ok;
	bool SensHum_Ok;
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(936861442);

#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>



IPAddress ip_Mqtt (192, 168, 1, 110);
#define topic "Stazione1"
EthernetClient ethClient;
MqttClient client(ethClient);

JsonDocument doc;
long lastReconnectAttempt = 0;
unsigned int fsm = 0;
unsigned int total_retry = 0;
bool conncected = false;

#line 45 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\Mqtt\\LLSketch\\LLSketch.ino"
boolean reconnect();
#line 70 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\Mqtt\\LLSketch\\LLSketch.ino"
void setup();
#line 88 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\Mqtt\\LLSketch\\LLSketch.ino"
void loop();
#line 128 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\Mqtt\\LLSketch\\LLSketch.ino"
void send_mqtt();
#line 45 "C:\\Users\\Davide\\Documents\\GitHub\\Arduino-PLC-Ide\\Mqtt\\LLSketch\\LLSketch.ino"
boolean reconnect()
{
  // Ensure clean disconnect

  Serial.println("Attempting MQTT connection...");

  // Attempt to connect to the MQTT broker
  
  // boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
  client.connect(ip_Mqtt, 1883);
  if (client.connect(ip_Mqtt, 1883))
  {
    Serial.println("connected");
    return true;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(String(client.connectError()));
    Serial.println(" try again in 5 seconds");
    return false;
  }
}


void setup()
{

	// Configure static IP address
	IPAddress ip(192, 168, 1, 20);
	IPAddress dns(8, 8, 8, 8);
	IPAddress gateway(192, 168, 1, 1);
	IPAddress subnet(255, 255, 255, 0);
	Ethernet.begin(ip, dns, gateway, subnet);
	// If cable is not connected this will block the start of PLC with about 60s of timeout!


	


	AxelPLC.Run();
}

void loop()
{
    if (!client.connected()) 
    {
        PLCIn.Mqtt_Ok = false;
        PLCIn.Send_Ok = false;
        PLCIn.Send_Ko = false;
        conncected = false;
        fsm = 0;
        
        long now = millis();
        if (now - lastReconnectAttempt > 5000) 
        {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnect())
            {
                lastReconnectAttempt = 0;
                PLCIn.Mqtt_Ok = true;
                conncected= true;
            }
            total_retry++;
            
            if (total_retry> 5)
            {
                NVIC_SystemReset();
            }

        }
        
    }
    
    send_mqtt();
    
    client.poll();
    

}


void send_mqtt()
{
    switch (fsm)
    {
        case 0:
        // passo iniziale
        if (PLCOut.Send_Data && conncected)
        {
            fsm = 1;
        }
        break;
        
        case 1:
        // pubblico dati
        char buffer[256];
        doc["Temperatura"] = PLCOut.Temperatura;
        doc["SensTempStat"] = PLCOut.SensTemp_Ok;
        doc["Umidita"] = PLCOut.Umidita;
        doc["SensHumStat"] = PLCOut.SensHum_Ok;
        serializeJson(doc, buffer);
        
        client.beginMessage(topic);
        client.print(buffer);
        client.endMessage();
        PLCIn.Send_Ok = true;
        fsm = 2;
        break;
        
        case 2:
        // attendo sincronizzazione con plc task
        if (! PLCOut.Send_Data)
        {
            PLCIn.Send_Ok = false;
            PLCIn.Send_Ko = false;
            fsm = 0; 
        }
        break;
    }
}

