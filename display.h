
#ifndef DISPLAY_H
#define DISPLAY_H

#include "mbed.h"

/* Initialisation de l'affichage de l'espace de jeu */
void pong_init_ttempro(Serial &pc){
	pc.baud(115200);
	
    pc.printf("\033[2J");       //Efface la console
    pc.printf("\033[?25l");     //Cache le curseur
    
    for(int i=0; i <= 128 ; i++){
        pc.printf("\033[0;%dH",i);     //Place le curseur à 0:0
        pc.printf("X");     //Place le curseur à 0:0
        pc.printf("\033[32;%dH",i);     //Place le curseur à 0:0
        pc.printf("X");     //Place le curseur à 0:0
    }
    
    for(int i=0; i <= 32 ; i++){
        pc.printf("\033[%d;0H",i);     //Place le curseur à 0:0
        pc.printf("X");     //Place le curseur à 0:0
        pc.printf("\033[%d;128H",i);     //Place le curseur à 0:0
        pc.printf("X");     //Place le curseur à 0:0
    }
    
}

/* Mise à jour de l'affichage des raquettes en tenant compte de leur position courante et précédente */
void print_cursor_ttempro(Serial &pc, int *pos1, int *pos1prec, int *pos2, int *pos2prec){
    for(int i=0;i<=29;i++){
        if( pos1[i] != pos1prec[i] ){
            if(pos1[i] == 1){
                pc.printf("\033[%d;3H",i+2);     
                pc.printf("X");     
            }
            else{
                pc.printf("\033[%d;3H",i+2);     
                pc.printf(" ");     
            }
        }
                
        if( pos2[i] != pos2prec[i] ){
            if(pos2[i] == 1){
                pc.printf("\033[%d;126H",i+2);
                pc.printf("X");
            }
            else{
                pc.printf("\033[%d;126H",i+2);
                pc.printf(" "); 
            }
        }   
    } 
}

/* Association entre la distance mesurée par les capteurs et la position des raquettes sur leur axe de déplacement */
void set_pos_vector(int *pos, int distance){
            
    if(distance <= 9){
        for(int i=0;i<=5;i++){ pos[i] = 1; }
    }else if(distance >= 33){
        for(int i=24;i<=29;i++){ pos[i] = 1; }
    }else{
        for(int i=(distance-9);i<=(distance-4);i++){ pos[i] = 1; }
    }   
}

/* Affichage de la position de la balle en fonction de sa position précédente, son angle, et la position des 2 raquettes */
int set_print_ball(Serial &pc, int *pos1,int *pos2,double *posBallX, double *posBallY, double *angBall){
    pc.printf("\033[%d;%dH",(int)*posBallY, (int)*posBallX);
    pc.printf(" ");
    *posBallX = *posBallX + 2.0*cos((double)*angBall*3.1415/180.0);
    *posBallY = *posBallY - 2.0*sin((double)*angBall*3.1415/180.0);
    
    if(*posBallX <= 4){
        if(pos1[(int)*posBallY] == 1){
            *posBallX = 8 - *posBallX;
            *angBall = 180 - *angBall;
        }else{ return 4; }
    }else if(*posBallX >= 125){
        if(pos2[(int)*posBallY] == 1){
            *posBallX = 250 - *posBallX;
            *angBall = 180 - *angBall;
        }else{ return 3; }
    }
        
        
    if(*posBallY <= 2){
        *posBallY = -*posBallY+4;
        *angBall = -*angBall;
    }else if(*posBallY >= 31){
        *posBallY = 62 - *posBallY;
        *angBall = -*angBall;
    }
    pc.printf("\033[%d;%dH",(int)*posBallY, (int)*posBallX);
    pc.printf("o");
    return 2;
}

/* Affiche et place la balle à coté de la raquette avant une remise en jeu */
int print_ball(Serial &pc, int *distance1, int *distance2, double *posBallX, double *posBallY){
    pc.printf("\033[%d;%dH",(int)*posBallY, (int)*posBallX);
    pc.printf(" ");
    if(*posBallX <= 50){
        if(*distance1 <= 10){
            *posBallY = 4;
        }else if(*distance1 >= 33){
            *posBallY = 28;
        }else{
            *posBallY = *distance1-5;
        }
    }else{
        if(*distance2 <= 10){
            *posBallY = 4;
        }else if(*distance2 >= 34){
            *posBallY = 28;
        }else{
            *posBallY = *distance2-5;
        }
    }
    pc.printf("\033[%d;%dH",(int)*posBallY, (int)*posBallX);
    pc.printf("o");
    return 2;
}



#endif