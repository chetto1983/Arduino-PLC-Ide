#include <AlPlc_Opta.h>

/* opta_1.2.0
      - ArduinoJson (7.2.0)
      - PubSubClient (2.8.0)
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
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(1274156522);

#include <PubSubClient.h>
#include <ArduinoJson.h>




IPAddress ip_Mqtt (192, 168, 1, 110);
#define topic "Stazione1"
String clientName = "OptaStazione1";
EthernetClient ethClient;
PubSubClient client(ethClient);
JsonDocument doc;
long lastReconnectAttempt = 0;
unsigned int fsm = 0;
unsigned int total_retry = 0;

boolean reconnect()
{
  // Ensure clean disconnect
  client.disconnect();
  delay(100);

  Serial.println("Attempting MQTT connection...");

  // Attempt to connect to the MQTT broker
  clientName += String(random(0xffff), HEX);
  client.connect(clientName.c_str());
  if (client.connect(clientName.c_str()))
  {
    Serial.println("connected");
    return true;
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.println(String(client.state()));
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
	client.setServer(ip_Mqtt, 1883);


	


	AxelPLC.Run();
}

void loop()
{
    if (!client.connected()) 
    {
        PLCIn.Mqtt_Ok = false;
        PLCIn.Send_Ok = false;
        PLCIn.Mqtt_Ok = false;
        long now = millis();
        if (now - lastReconnectAttempt > 5000) 
        {
            lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnect())
            {
                lastReconnectAttempt = 0;
                PLCIn.Mqtt_Ok = true;
            }
            total_retry++;
            
            if (total_retry> 5)
            {
                NVIC_SystemReset();
            }

        }
        
    }
    
    send_mqtt();
    
    client.loop();
    

}


void send_mqtt()
{
    switch (fsm)
    {
        case 0:
        // passo iniziale
        if (PLCOut.Send_Data && PLCIn.Mqtt_Ok)
        {
            fsm = 1;
        }
        break;
        
        case 1:
        // pubblico dati
        char buffer[256];
        doc["Temperatura"] = PLCOut.Temperatura;
        doc["Umidita"] = PLCOut.Umidita;
        
        serializeJson(doc, buffer);

        if (client.publish(topic, buffer))
        {
            PLCIn.Send_Ok = true;
            fsm = 2;
        }
        else
        {
            PLCIn.Send_Ko = true;
            fsm = 2;
        }
        
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
