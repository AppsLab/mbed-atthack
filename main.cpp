#include "mbed.h"
#include "M2XStreamClient.h"
#include "include_me.h"
#include "MMA8451Q.h"

using namespace mts;

const char key[] = "XXX";  // enter your m2x user account master key
const char feed[] = "XXX"; // enter your blueprint feed id

// set to 1 for cellular shield board
// set to 0 for wifi shield board
#define CELL_SHIELD 1

#define LED_ON  0
#define LED_OFF 1

/////////////////////////////////////////////////////////////////////
// Include support for MMA8451Q Acceleromoter
#define MMA8451_I2C_ADDRESS (0x1d<<1)
MMA8451Q acc(PTE25, PTE24, MMA8451_I2C_ADDRESS);

float down;

// ssid and phrase for wifi
std::string ssid = "belkin54g";
std::string phrase = "hackathon";

DigitalIn holster(A0);
DigitalIn gun(A1);
DigitalIn taser(A2);
DigitalOut greenLED(LED_GREEN);
DigitalOut redLED(LED_RED);


int main()
{
    greenLED = LED_OFF;
    redLED = LED_OFF;
#if CELL_SHIELD
    for (int i = 30; i >= 0; i = i - 2) {
        wait(2);
        printf("Waiting %d seconds...\n\r", i);
    }

    MTSSerialFlowControl* serial = new MTSSerialFlowControl(PTD3, PTD2, PTA12, PTC8);
    serial->baud(115200);
    Transport::setTransport(Transport::CELLULAR);
    Cellular* cell = Cellular::getInstance();
    cell->init(serial, PTA4, PTC9); //DCD and DTR pins for KL46Z

    int max_tries = 5;
    int i;
    std::string apn = "wap.cingular";

    i = 0;
    while (i++ < max_tries) {
        if (cell->getRegistration() == Cellular::REGISTERED) {
            printf("registered with tower\n\r");
            break;
        } else if (i >= max_tries) {
            printf("failed to register with tower\n\r");
        } else {
            wait(3);
        }
    }

    printf("signal strength: %d\n\r", cell->getSignalStrength());

    i = 0;
    printf("setting APN to %s\n\r", apn.c_str());
    while (i++ < max_tries) {
        if (cell->setApn(apn) == SUCCESS) {
            printf("successfully set APN\n\r");
            break;
        } else if (i >= max_tries) {
            printf("failed to set APN\n\r");
        } else {
            wait(1);
        }
    }

    i = 0;
    printf("bringing up PPP link\n\r");
    while (i++ < max_tries) {
        if (cell->connect()) {
            printf("PPP link is up\n\r");
            break;
        } else if (i >= max_tries) {
            printf("failed to bring PPP link up\n\r");
        } else {
            wait(1);
        }
    }
#else
    for (int i = 6; i >= 0; i = i - 2) {
        wait(2);
        printf("Waiting %d seconds...\n\r", i);
    }
    MTSSerial* serial = new MTSSerial(PTD3, PTD2, 256, 256);
    serial->baud(9600);
    Transport::setTransport(Transport::WIFI);
    Wifi* wifi = Wifi::getInstance();
    printf("Init: %s\n\r", wifi->init(serial) ? "SUCCESS" : "FAILURE");
    printf("Set Network: %s\n\r", getCodeNames(wifi->setNetwork(ssid, Wifi::WPA, phrase)).c_str());
    printf("Set DHCP: %s\n\r", getCodeNames(wifi->setDeviceIP("DHCP")).c_str());
    printf("Signal Strnegth (dBm): %d\n\r", wifi->getSignalStrength());
    printf("Is Connected: %s\n\r", wifi->isConnected() ? "True" : "False");
    printf("Connect: %s\n\r", wifi->connect() ? "Success" : "Failure");
    printf("Is Connected: %s\n\r", wifi->isConnected() ? "True" : "False");
#endif

    /* send some data */
    Client client;
    M2XStreamClient m2xClient(&client, key);
    int holster_ret;
    int gun_ret;
    int taser_ret;
    int down_ret;
    
    
    holster.mode(PullUp);
    gun.mode(PullUp);
    taser.mode(PullUp);
    
    
    while (true) {
        redLED = LED_OFF;
        int num = rand();
        printf("ramdom number %d\r\n", num);
        
        if (!holster){
            holster_ret = m2xClient.send(feed, "holster", 1);
            printf("Holster %d\r\n", holster_ret);
            redLED = LED_ON;
        }
        if (!gun){
            gun_ret = m2xClient.send(feed, "gun", 1);
            printf("Gun %d\r\n", gun_ret);
            redLED = LED_ON;
        }
        if (!taser){
            taser_ret = m2xClient.send(feed, "taser", 1);
            printf("Taser %d\r\n", taser_ret);
            redLED = LED_ON;
        }
        
        
        down = acc.getAccZ();
        
        if(down > 0.8 || down < -0.8){
            down_ret = m2xClient.send(feed, "down", 1);
            printf("Down %f\r\n", down_ret);
            redLED = LED_ON;
        }
        wait(0.2);
    }
}
