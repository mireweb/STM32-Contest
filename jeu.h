#ifndef JEU_H
#define JEU_H

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/services/HeartRateService.h"
#include "ble/services/BatteryService.h"
#include "ble/services/DeviceInformationService.h"

DigitalOut led1(LED1);

/******************************
 |      CONFIGURATION BLE     |
 ******************************/

uint16_t customServiceUUID  = 0xA000;
uint16_t readP1UUID         = 0xA001;
uint16_t readP2UUID         = 0xA002;
uint16_t chronoUUID         = 0xA003;

const static char     DEVICE_NAME[]        = "Pong!";
static const uint16_t uuid16_list[]        = {0xFFFF}; //Custom UUID, FFFF is reserved for development


// Set Up custom Characteristics
static uint8_t readP1[10] = {0};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readP1)> player1(readP1UUID, readP1);
static uint8_t readP2[10] = {0};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readP2)> player2(readP2UUID, readP2);
static uint8_t readChrono[10] = {0};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readChrono)> chronoJeu(chronoUUID, readChrono);

// Set up custom service
GattCharacteristic *characteristics[] = {&player1, &player2, &chronoJeu};
GattService        customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

static volatile bool  triggerSensorPolling = false;

void periodicCallback(void){
    led1 = !led1; /* Do blinky on LED1 while we're waiting for BLE events */
    /* Note that the periodicCallback() executes in interrupt context, so it is safer to do
     * heavy-weight sensor polling from the main thread. */
    triggerSensorPolling = true;
}

void initJeuBLE(BLE &ble){
    ble.init();

    /* Setup advertising. */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().setAdvertisingInterval(1000); /* 1000ms */
    ble.addService(customService);
    ble.gap().startAdvertising();
}
void miseAJourBLE(BLE &ble,uint8_t &p1_score,uint8_t &p2_score,uint8_t &chrono){
    const uint8_t scoreP1const = p1_score;
    const uint8_t scoreP2const = p2_score;
    const uint8_t chronoconst = chrono;
    
    ble.gattServer().write(player1.getValueHandle(), &scoreP1const, sizeof(scoreP1const));
    ble.gattServer().write(player2.getValueHandle(), &scoreP2const, sizeof(scoreP2const));
    ble.gattServer().write(chronoJeu.getValueHandle(), &chronoconst, sizeof(chronoconst));
}


/******************************
 |      CONFIGURATION JEU     |
 ******************************/

class InfosJeu {
  public:
    InfosJeu(){
        chrono=0;
        p1_score=0;
        p2_score=0;
        chronoActive=0;
        
        posBallX = 4.0;
        posBallY = 19.0;
        angBall = 25.0;
        state = 0;
        p1_score = 0;
        p2_score = 0;
        wait(1);
        //led=1;
        distance1=0;
        distance2=0;
        for(int i=0;i<30;i++){
            pos1prec[i]=0;
            pos2prec[i]=0;
            pos1[i]=0;
            pos2[i]=0;
        }
    }
    uint8_t chrono;
    bool chronoActive;
    uint8_t p1_score;
    uint8_t p2_score;
    
    int pos1prec[30];
    int pos2prec[30];
    int pos1[30];
    int pos2[30];
    double posBallX;
    double posBallY;
    double angBall;
    int distance1;
    int distance2;
    
    int state;
    
    void addPointP1(){p1_score++;}
    void addPointP2(){p2_score++;}
    void setChronoActive(){chronoActive=true;}
    void incrementeChrono(){if(chronoActive){chrono++;}}
    void stopChrono(){chronoActive=false;}
    void resetAndStart(){chrono=0; p1_score=0; p2_score=0; setChronoActive();}
    
    void reinitPosition(int p1_sc, int p2_sc, int posX, int posY, int angle){
        p1_score = p1_sc;
        p2_score = p2_sc;
        posBallX = posX;
        posBallY = posY;
        angBall = angle;
    }
};




#endif