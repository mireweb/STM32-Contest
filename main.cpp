#include "mbed.h"
#include "HCSR04.h"
#include "jeu.h"
#include "display.h"
#include <math.h>


#define TRIG_1  PA_9
#define TRIG_2  PB_10
#define ECHO_1  PC_7
#define ECHO_2  PA_8
#define PUSH    USER_BUTTON //PC_13
#define LED_1   LED1

/*
#define ECHO_1  PA_4
#define ECHO_2  PB_3
#define TRIG_1  PA_5
#define TRIG_2  PA_10
#define PUSH    PA_9 //PC_13
#define LED_1   PA_8
*/

//------------------------------------
// Hyperterminal configuration
// 115200 bauds, 8-bit data, no parity
//------------------------------------


//DigitalOut led(LED_1);      //Led d'état
DigitalIn bp(PUSH);
Serial pc(USBTX, USBRX);    //UART


HCSR04 sensor1(TRIG_1, ECHO_1);
HCSR04 sensor2(TRIG_2, ECHO_2); 

BLE  ble;

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params){
    ble.gap().startAdvertising(); // restart advertising
}

InfosJeu jeu;
/*  ACTION POSSIBLES : 
 *  --> Accès aux valeurs :
 *      jeu.p1_score
 *      jeu.p2_score
 *      jeu.chrono
 *      jeu.pos1prec[30];
 *      jeu.pos2prec[30];
 *      jeu.pos1[30];
 *      jeu.pos2[30];
 *      jeu.posBallX;
 *      jeu.posBallY;
 *      jeu.angBall;
 *      jeu.distance1;
 *      jeu.distance2;
 *      jeu.state;
 *
 *  --> Méthodes utiles :
 *      jeu.addPointP1();
 *      jeu.addPointP2();
 *      jeu.setChronoActive();
 *      jeu.incrementeChrono();
 *      jeu.stopChrono();
 *      jeu.resetAndStart();
 */   

int main() {
    //Initialisation des fonctions récurrentes (toutes les secondes)
    Ticker ticker;
    ticker.attach(periodicCallback, 1); // blink LED every second
    Ticker tickerChrono;
    tickerChrono.attach(&jeu, &InfosJeu::incrementeChrono, 1);
    
    //Initialisation du BLE
    initJeuBLE(ble);
    ble.gap().onDisconnection(disconnectionCallback);
    
    //Initialisation de l'affichage
    pong_init_ttempro(pc);
    
    //Boucle d'exécution du programme
    while(1) {
        
        /*
        ******* Mise à jour des données BLE chaque seconde *******
        */
        if(triggerSensorPolling && ble.getGapState().connected){
            triggerSensorPolling = false; // flag indiquant qu'une seconde s'est écoulée, il faut mettre à jour les données BLE           
            miseAJourBLE(ble, jeu.p1_score, jeu.p2_score, jeu.chrono);
        }else{
            ble.waitForEvent(); // low power wait for event
        }
        
        /*
        ******* Mécanique de jeu ********
        */
        switch(jeu.state){
        case 0 :    if(bp.read() != 1){ 
                        jeu.state = 4; 
                        pong_init_ttempro(pc);
                    }
                    break;
                    
        case 4 :    for(int i=0; i<=29; i++){
                        jeu.pos1prec[i]= jeu.pos1[i];
                        jeu.pos2prec[i]= jeu.pos2[i];
                        jeu.pos1[i]= 0;
                        jeu.pos2[i]= 0;
                    }
                    jeu.distance1 = sensor1.distance(1);
                    jeu.distance2 = sensor2.distance(1);
                    set_pos_vector(jeu.pos1,jeu.distance1);
                    set_pos_vector(jeu.pos2,jeu.distance2);     
                    print_cursor_ttempro(pc,jeu.pos1,jeu.pos1prec,jeu.pos2,jeu.pos2prec);
                    print_ball(pc,&jeu.distance1,&jeu.distance2,&jeu.posBallX,&jeu.posBallY);
                    if(bp.read() != 1){ 
                        jeu.state = 3; 
                        jeu.setChronoActive();
                    }
                    break;
                    
        case 3 :    for(int i=0; i<=29; i++){
                        jeu.pos1prec[i]= jeu.pos1[i];
                        jeu.pos2prec[i]= jeu.pos2[i];
                        jeu.pos1[i]= 0;
                        jeu.pos2[i]= 0;
                    }
                    jeu.distance1 = sensor1.distance(1);
                    jeu.distance2 = sensor2.distance(1);
                    set_pos_vector(jeu.pos1,jeu.distance1);
                    set_pos_vector(jeu.pos2,jeu.distance2);     
                    print_cursor_ttempro(pc,jeu.pos1,jeu.pos1prec,jeu.pos2,jeu.pos2prec);
                    jeu.state = set_print_ball(pc,jeu.pos1,jeu.pos2,&jeu.posBallX,&jeu.posBallY,&jeu.angBall);
                    break;
                    
        case 1 :    jeu.addPointP2();
                    if( jeu.p2_score >= 3){ 
                        pc.printf("\033[2J");       //Efface la console
                        pc.printf("\033[16;60H"); 
                        pc.printf("P2  WIN ");
                        jeu.stopChrono();
                        jeu.reinitPosition(0, 0, 4.0, 19.0, 25.0); // (p1_score, p2_score, posBallX, posBallY, angBall)
                    }
                    else{
                        pc.printf("\033[2J");       //Efface la console
                        pc.printf("\033[16;55H"); 
                        pc.printf("P1 : %d  // P2 : %d",jeu.p1_score,jeu.p2_score);
                        jeu.reinitPosition(jeu.p1_score, jeu.p2_score, 4.0, 19.0, 25.0); // (p1_score, p2_score, posBallX, posBallY, angBall)
                    }
                    jeu.state = 0;
                    break;
                    
        case 2 :    jeu.addPointP1();
                    if( jeu.p1_score >= 3){ 
                        pc.printf("\033[2J");       //Efface la console
                        pc.printf("\033[16;60H"); 
                        pc.printf("P1  WIN ");
                        jeu.stopChrono();
                        jeu.reinitPosition(0, 0, 125.0, 19.0, 145.0); // (p1_score, p2_score, posBallX, posBallY, angBall)
                    }
                    else{
                        pc.printf("\033[2J");       //Efface la console
                        pc.printf("\033[16;55H"); 
                        pc.printf("P1 : %d  // P2 : %d",jeu.p1_score,jeu.p2_score);
                        jeu.reinitPosition(jeu.p1_score, jeu.p2_score, 125.0, 19.0, 145.0); // (p1_score, p2_score, posBallX, posBallY, angBall)
                    }
                    jeu.state = 0;
                    break;
        }
        wait(0.075);
    }
}

 