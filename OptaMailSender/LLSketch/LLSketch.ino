#include <AlPlc_Opta.h>

/* opta_1.2.0
      - EMailSender (3.0.14)
      - SD (1.3.0)
*/

struct PLCSharedVarsInput_t
{
	bool MailOk;
	bool MailKo;
};
PLCSharedVarsInput_t& PLCIn = (PLCSharedVarsInput_t&)m_PLCSharedVarsInputBuf;

struct PLCSharedVarsOutput_t
{
	bool MandaMail;
	char Messaggio[32];
};
PLCSharedVarsOutput_t& PLCOut = (PLCSharedVarsOutput_t&)m_PLCSharedVarsOutputBuf;


AlPlc AxelPLC(-1154620693);

// shared variables can be accessed with PLCIn.varname and PLCOut.varname
#include <EMailSender.h>

EMailSender emailSend("testopta@libero.it","Xw3jT-R58Z9$nnY","testopta@libero.it","smtp.libero.it", 465);

unsigned int fsm = 0;

void setup()
{

	// Configure static IP address
	/*IPAddress ip(192, 168, 251, 223);
	IPAddress dns(8, 8, 8, 8);
	IPAddress gateway(192, 168, 251, 76);
	IPAddress subnet(255, 255, 255, 0);
	// If cable is not connected this will block the start of PLC with about 60s of timeout!
	Ethernet.begin(ip, dns, gateway, subnet);*/
	Serial.begin(9600);
	Serial.println("Partito");

	AxelPLC.Run();
}

void loop()
{
    send_mail();

}



void send_mail()
{
    EMailSender::EMailMessage message;
    EMailSender::Response     resp;
    const char* arrayOfEmail[] = {"dvdmarchetto@gmail.com", "davide.marchetto@sacchi.it", "testopta@libero.it"};
    switch (fsm)
    {
        case 0:   // attendo richiesta
            if (PLCOut.MandaMail)
                fsm = 1;
            break;

        case 1:   // invio mail
        
            // scope locale - le variabili vivono solo qui
            message.subject = "allarme macchina";
            message.message = PLCOut.Messaggio;
            Serial.println("Sending status: ");
            Serial.println(resp.status);
            Serial.println(resp.code);
            Serial.println(resp.desc);

                
            resp = emailSend.send(arrayOfEmail, 3, message);

            if (resp.status)
                PLCIn.MailOk = true;
            else
                PLCIn.MailKo = true;

            fsm = 2;          // passa allo stato di attesa reset
        
            break;

        case 2:   // reset dopo sincronizzazione con PLC task
            if (!PLCOut.MandaMail)
            {
                PLCIn.MailOk = false;
                PLCIn.MailKo = false;
                fsm = 0;
            }
            break;
    }
}
