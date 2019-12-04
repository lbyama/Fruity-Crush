#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "nokia.h"
#include "bmps.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "fruity_crush.h"


#define ESC_REG(x)                  (*((volatile uint32_t *)(x)))

#define GPIO_O_LOCK                 0x520       //usadas para desbloquear as portas PC3,2,1 & 0
#define GPIO_O_CR                   0x524
#define GPIO_LOCK_KEY               0x4C4F434B


int tabuleiro[4][4] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int PONTUACAO = 0;
int QTD_FRUTAS = 5;
int NIVEL_ATUAL = 1;
int NIVEL_JOGANDO = 1;
int JOGADAS = 20;

uint32_t portal_c[4]={GPIO_PORTF_BASE,GPIO_PORTF_BASE,GPIO_PORTF_BASE,GPIO_PORTF_BASE};
uint32_t pinos_c[4]={GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_2,GPIO_PIN_3};
uint32_t portal_l[4]={GPIO_PORTF_BASE,GPIO_PORTB_BASE,GPIO_PORTB_BASE,GPIO_PORTB_BASE};
uint32_t pinos_l[4]={GPIO_PIN_4,GPIO_PIN_0,GPIO_PIN_1,GPIO_PIN_5};

int ult_i = -10;
int ult_j = -10;

int PONTUACAO_NIVEL[17] = {0,600,700,800,900,600,700,800,900,700,800,900,1000,700,800,700,800};


void destrava_pino(uint32_t portal, uint8_t pino){
    ESC_REG( portal + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    ESC_REG( portal + GPIO_O_CR) |= pino;
}

void habilita_matriz_botoes2(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    destrava_pino(GPIO_PORTF_BASE,0x01);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);  //colocar como input
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_0 |GPIO_PIN_1 |GPIO_PIN_5);  //colocar como input
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4 , GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);   //colocar como pull-u
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_0 |GPIO_PIN_1 |GPIO_PIN_5 , GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);   //colocar como pull-u
}


void renovar_tabuleiro(void){
    int i,j,k;
    for(k=0;k<4;k++){
        for(j=0;j<4;j++){
            for(i=3;i>0;i--){
                if(tabuleiro[i][j]==0){
                    tabuleiro[i][j] = tabuleiro[i-1][j];
                    tabuleiro[i-1][j] = 0;

                }
            }
        }
        for(i=0;i<4;i++){
            if( tabuleiro[0][i] == 0){
                tabuleiro[0][i] = rand()%QTD_FRUTAS+1;
                PONTUACAO+=5;
            }
        }
    }

}

int ha_troca(void){
    int i,j,houve_troca = 0;
    int l1=0,c1=0,l2=0,c2=0;
    int qtd_ng=0,erro = 0,aux=0;
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if(tabuleiro[i][j] < 0){
                if(qtd_ng == 0){
                    l1 = i;
                    c1 = j;
                    qtd_ng++;
                }else if(qtd_ng == 1){
                    l2 = i;
                    c2 = j;
                    qtd_ng++;
                }
            }
        }
    }
    if (qtd_ng == 2){
        if((l1==l2 &&(c1==c2+1 || c1==c2-1))||(c1==c2 &&(l1==l2+1 || l1==l2-1))){
            tabuleiro[l1][c1] = abs(tabuleiro[l1][c1]);
            tabuleiro[l2][c2] = abs(tabuleiro[l2][c2]);
            aux = tabuleiro[l1][c1];
            tabuleiro[l1][c1] = tabuleiro[l2][c2];
            tabuleiro[l2][c2] = aux;
            ult_i = -1;
            ult_j = -1;
            houve_troca++;
            JOGADAS--;
        }
        erro++;
    }else if (qtd_ng > 2){
        erro++;
    }
    if(erro){
        for(i=0;i<4;i++){
            for(j=0;j<4;j++){
                 tabuleiro[i][j] = abs(tabuleiro[i][j]);
            }
        }
    }
    if(houve_troca && !(ha_combinacao())){
        JOGADAS++;
        imprimir_tabuleiro();
        SysCtlDelay(5000000);
        tabuleiro[l1][c1] = abs(tabuleiro[l1][c1]);
        tabuleiro[l2][c2] = abs(tabuleiro[l2][c2]);
        aux = tabuleiro[l1][c1];
        tabuleiro[l1][c1] = tabuleiro[l2][c2];
        tabuleiro[l2][c2] = aux;
        imprimir_tabuleiro();
        SysCtlDelay(500000);
        houve_troca = 0;
    }
    return houve_troca;
}

int ha_combinacao(void){
    int i,j,k,l,qual_fruta = 0, ret = 0;
    int soma = 0,soma_2 = 0;
    int eliminar[4][4] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

    for(i=0;i<4;i++){ //varrer linhas
        for(j=0;j<3;j++){
            if((abs(tabuleiro[i][j]) == abs(tabuleiro[i][j+1]))&&soma==0){
                soma++;
                qual_fruta = abs(tabuleiro[i][j]);
            }else if((abs(tabuleiro[i][j]) == abs(tabuleiro[i][j+1]))&&soma !=0){
                if(abs(tabuleiro[i][j]) == qual_fruta){
                    soma++;
                }
            }
        }
        if(soma==2){
            for(k=0;k<4;k++){
                if(tabuleiro[i][k] == qual_fruta){
                    if(eliminar[i][k] == 0){
                        eliminar[i][k] = 1;
                    }
                }
            }
        }
        if(soma>2 && qual_fruta != 0){
            for(k=0;k<4;k++){
                if(eliminar[i][k] == 0){
                    eliminar[i][k] = 1;
                }
            }
            PONTUACAO+=50;
            eliminar[i][rand()%4] = 2;
        }
        soma = 0;
        qual_fruta = 0;
    }

    for(j=0;j<4;j++){ //varrer colunas
        for(i=0;i<3;i++){
            if((abs(tabuleiro[i][j]) == abs(tabuleiro[i+1][j]))&&soma==0){
                soma++;
                qual_fruta = abs(tabuleiro[i][j]);
            }else if((abs(tabuleiro[i][j]) == abs(tabuleiro[i+1][j]))&&soma != 0){
                if(abs(tabuleiro[i][j])==qual_fruta){
                    soma++;
                }

            }
        }
        if(soma>=2){
             for(k=0;k<4;k++){
                 if(abs(tabuleiro[k][j]) == qual_fruta){
                     if(eliminar[k][j] == 0){
                         eliminar[k][j] = 1;
                     }
                 }
             }
         }
         if(soma>2 && qual_fruta != 0){
             for(k=0;k<4;k++){
                 if(eliminar[k][j] == 0){
                     eliminar[k][j] = 1;
                 }
             }
             PONTUACAO+=50;
             eliminar[rand()%4][j] = 3;
         }
         soma = 0;
         qual_fruta = 0;
    }

    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if(eliminar[i][j] >= 1 && tabuleiro[i][j] != 0){
                qual_fruta = abs(tabuleiro[i][j]);
                for(k=0;k<4;k++){
                    if(abs(tabuleiro[k][j]) == qual_fruta && eliminar[k][j] != 0){
                        soma++;
                    }
                    if(abs(tabuleiro[i][k]) == qual_fruta && eliminar[i][k] != 0){
                        soma_2++;
                    }
                }
                if(soma >= 3 && soma_2 >= 3){
                    if(eliminar[i][j] >1){
                        if(eliminar[i][j] == 2){
                            if(j>0){
                                eliminar[i][j-1] = 2;
                            }else{
                                eliminar[i][j+1] = 2;
                            }
                        }else if (eliminar[i][j] == 3){
                            if(i>0){
                                eliminar[i-1][j] = 3;
                            }else{
                                eliminar[i+1][j] = 3;
                            }
                        }
                    }
                    eliminar[i][j] = 4;
                    PONTUACAO+=50;
                }
                soma = 0;
                soma_2 = 0;
                qual_fruta = 0;
            }
        }
    }

    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if(eliminar[i][j] == 1 || tabuleiro[i][j] == 0){
                ret = 1;
            }
        }
    }


    if(ret == 0){
        for(i=0;i<4;i++){
            for(j=0;j<4;j++){
                if(tabuleiro[i][j] == 9){
                    for(k=0;k<4;k++){
                        eliminar[k][j] = 1;
                    }
                    ret = 1;
                } else if(tabuleiro[i][j] == 10){
                    for(k=0;k<4;k++){
                        eliminar[i][k] = 1;
                    }
                    ret = 1;
                }else if(tabuleiro[i][j] == 11){
                    for(k=0;k<4;k++){
                        for(l=0;l<4;l++){
                            if((k == i-1 || k == i || k == i+1)&&(l == j-1 || l == j || l == j+1)){
                                if(k >= 0 && k < 4 && l >= 0 && l < 4){
                                    eliminar[k][l] = 1;
                                }
                            }
                        }
                    }
                    ret = 1;
                }
            }
        }
    }

    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if(eliminar[i][j] == 1){
                tabuleiro[i][j] = 0;
            }else if(eliminar[i][j] != 0){
                tabuleiro[i][j] = 7 + eliminar[i][j];
            }
        }
    }
    return ret;
}

void imprimir_tabuleiro(void){
    int i,j;
    Nokia5110_ClearBuffer();
    Nokia5110_PrintBMP2(0, 0, tabuleiro_bitmap, 84, 48);
    for(i=0; i<4;i++){
        for(j=0;j<4;j++){
            if(tabuleiro[i][j]<0){
                Nokia5110_PrintBMP2(3 + 12*j, 2 + 11*i, frutas_i[abs(tabuleiro[i][j])], 11, 11);
            }else if (tabuleiro[i][j]<9){
                Nokia5110_PrintBMP2(3 + 12*j, 2 + 11*i, frutas[tabuleiro[i][j]], 11, 11);
            }else{
                Nokia5110_PrintBMP2(3 + 12*j, 2 + 11*i, especiais[tabuleiro[i][j]-9], 11, 11);
            }
        }
    }
    Nokia5110_PrintBMP2(84-20,8, numeros[(JOGADAS - JOGADAS%10)/10], 5, 6);
    Nokia5110_PrintBMP2(84-20+6,8, numeros[JOGADAS%10],  5, 6);
    if(PONTUACAO>999){
        PONTUACAO = PONTUACAO - 1000;
        Nokia5110_PrintBMP2(84-26,48-22, numeros[1],  5, 6);
        Nokia5110_PrintBMP2(84-20,48-22, numeros[(PONTUACAO - PONTUACAO%100)/100], 5, 6);
        Nokia5110_PrintBMP2(84-20+6,48-22, numeros[(PONTUACAO%100 - PONTUACAO%10)/10],  5, 6);
        Nokia5110_PrintBMP2(84-20+12,48-22, numeros[PONTUACAO%10], 5, 6);
        PONTUACAO = PONTUACAO + 1000;
    } else if(PONTUACAO >99){
        Nokia5110_PrintBMP2(84-20,48-22, numeros[(PONTUACAO - PONTUACAO%100)/100], 5, 6);
        Nokia5110_PrintBMP2(84-20+6,48-22, numeros[(PONTUACAO%100 - PONTUACAO%10)/10], 5, 6);
        Nokia5110_PrintBMP2(84-20+12,48-22, numeros[PONTUACAO%10], 5, 6);
    }else if(PONTUACAO >9){
        Nokia5110_PrintBMP2(84-20+6,48-22, numeros[(PONTUACAO - PONTUACAO%10)/10], 5, 6);
        Nokia5110_PrintBMP2(84-20+12,48-22, numeros[PONTUACAO%10], 5, 6);
    }else{
        Nokia5110_PrintBMP2(84-20+12,48-22, numeros[PONTUACAO%10], 5, 6);
    }

    Nokia5110_PrintBMP2(84-20,48-10, numeros[(NIVEL_ATUAL - NIVEL_ATUAL%10)/10], 5, 6);
    Nokia5110_PrintBMP2(84-20+6,48-10, numeros[NIVEL_ATUAL%10], 5, 6);
    //52 ate 56
    i = (PONTUACAO*1.0/PONTUACAO_NIVEL[NIVEL_JOGANDO])*42.0;
    if(i<42){
        for(j=1;j<=i;j++){
            Nokia5110_SetPxl(45-j,54);
            Nokia5110_SetPxl(45-j,55);
        }
    } else {
        for(j=1; j<=42;j++){
            Nokia5110_SetPxl(45-j,54);
            Nokia5110_SetPxl(45-j,55);
        }
    }
    Nokia5110_DisplayBuffer();
    SysCtlDelay(3000);
}

void qual_botao2(void){
    int i,j;
    for(j=0;j<4;j++){
        GPIOPinWrite(portal_c[j], pinos_c[0] | pinos_c[1] | pinos_c[2] | pinos_c[3], ~(pinos_c[j]));
        for(i=0;i<4;i++){
            if(GPIOPinRead(portal_l[i],pinos_l[i]) == 0x00){
                if(i != ult_i || j!=ult_j){
                      tabuleiro[i][j] = (-1)*tabuleiro[i][j];
                      ult_i = i;
                      ult_j = j;
                }
            }
        }
    }
    SysCtlDelay(1000000);
}

int checagem(int tabu[4][4]){
    int i,j,k,qual_fruta = 0, ret = 0;
        int soma = 0;

        for(i=0;i<4;i++){ //varrer linhas
            for(j=0;j<3;j++){
                if((abs(tabu[i][j]) == abs(tabu[i][j+1]))&&soma==0){
                    soma++;
                    qual_fruta = abs(tabu[i][j]);
                }else if((abs(tabu[i][j]) == abs(tabu[i][j+1]))&&soma !=0){
                    if(abs(tabu[i][j]) == qual_fruta){
                        soma++;
                    }
                }
            }
            if(soma>=2){
                for(k=0;k<4;k++){
                    if(tabu[i][k] == qual_fruta){
                        ret = 1;
                    }
                }
            }
            soma = 0;
            qual_fruta = 0;
        }

        for(j=0;j<4;j++){ //varrer colunas
            for(i=0;i<3;i++){
                if((abs(tabu[i][j]) == abs(tabu[i+1][j]))&&soma==0){
                    soma++;
                    qual_fruta = abs(tabu[i][j]);
                }else if((abs(tabu[i][j]) == abs(tabu[i+1][j]))&&soma != 0){
                    if(abs(tabu[i][j])==qual_fruta){
                        soma++;
                    }

                }
            }
            if(soma>=2){
                 for(k=0;k<4;k++){
                     if(abs(tabu[k][j]) == qual_fruta){
                         ret = 1;
                     }
                 }
             }
             soma = 0;
             qual_fruta = 0;
        }
        return ret;
}

int ha_comb_possiveis(){
    int i,j,sim = 0,aux = 0;
    int tab[4][4];
    for(i=0;i<4;i++){
            for(j=0;j<4;j++){
               tab[i][j] = tabuleiro[i][j];
               if(tabuleiro[i][j]>=9){
                   sim++;
               }
            }
        }

    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if(i+1<4){
                tab[i][j] = abs(tab[i][j]);
                tab[i+1][j] = abs(tab[i+1][j]);
                aux = tab[i][j];
                tab[i][j] = tab[i+1][j];
                tab[i+1][j] = aux;

                sim += checagem(tab);

                tab[i][j] = abs(tab[i][j]);
                tab[i+1][j] = abs(tab[i+1][j]);
                aux = tab[i][j];
                tab[i][j] = tab[i+1][j];
                tab[i+1][j] = aux;
            }
            if(j+1<4){
                tab[i][j] = abs(tab[i][j]);
                tab[i][j+1] = abs(tab[i][j+1]);
                aux = tab[i][j];
                tab[i][j] = tab[i][j+1];
                tab[i][j+1] = aux;
                sim += checagem(tab);
                tab[i][j] = abs(tab[i][j]);
                tab[i][j+1] = abs(tab[i][j+1]);
                aux = tab[i][j];
                tab[i][j] = tab[i][j+1];
                tab[i][j+1] = aux;
            }
        }
    }
    if(sim == 0){
        for(i=0;i<4;i++){
            for(j=0;j<4;j++){
                tabuleiro[i][j] = rand()%QTD_FRUTAS + 1;
            }
        }
    }
    return sim;
}

void setup_nivel(void){
    int i,j,mudanca = 0,pos_x[4]={9,27,44,62},int_aux = 0,inst = 10;
    uint8_t aux = 0x11;
    aux = 0x11;
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    Nokia5110_ClearBuffer();
    Nokia5110_PrintBMP2(0, 0, primeira_tela, 84, 48);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(1000000);
    while(GPIOPinRead(GPIO_PORTF_BASE,GPIO_INT_PIN_0|GPIO_INT_PIN_1|GPIO_INT_PIN_2|GPIO_INT_PIN_3 | GPIO_PIN_4) == 0x11){}
    while(aux != 0x10){
        Nokia5110_ClearBuffer();
        Nokia5110_PrintBMP2(0, 0, opcoes, 84, 48);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(3000000);

        aux = GPIOPinRead(GPIO_PORTF_BASE,GPIO_INT_PIN_0|GPIO_PIN_4);
        while(aux == 0x11){
            aux = GPIOPinRead(GPIO_PORTF_BASE,GPIO_INT_PIN_0|GPIO_PIN_4);
        }
        if(aux == 0x01){
            SysCtlDelay(3000000);
            while(inst>-1){
                Nokia5110_ClearBuffer();
                Nokia5110_PrintBMP2(0,0,INSTRUCOES[10-inst],84,48);
                Nokia5110_DisplayBuffer();
                if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_INT_PIN_0|GPIO_PIN_4) == 0x01){
                    inst--;
                    SysCtlDelay(5000000);
                }
                if(GPIOPinRead(GPIO_PORTF_BASE,GPIO_INT_PIN_0|GPIO_PIN_4) == 0x10){
                    inst = -1;
                }
            }
        }
    }
    int_aux = 0;
    Nokia5110_ClearBuffer();
    //void Nokia5110_SetCursor(9 + 18 + 17 + 18, 1 + 12 + 12 + 12);
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            if( (i+(4*j+1)) <= NIVEL_ATUAL){
                Nokia5110_PrintBMP2(pos_x[i], 1 + 12*j,nivel_num[i+(4*j)], 13, 10);
            }else{
                Nokia5110_PrintBMP2(pos_x[i], 1 + 12*j, nivel_bloq, 13, 10);
            }
        }
    }
    //Nokia5110_PrintBMP2(9, 1, nivel_bloq, 13, 10);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(10000000);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

    while(int_aux <= 0 || int_aux > NIVEL_ATUAL){
        qual_botao2();
        int_aux = ult_j+(ult_i*4+1);
    }
    NIVEL_JOGANDO = int_aux;

    if(NIVEL_JOGANDO <=4){
        QTD_FRUTAS = 4;
        JOGADAS = 20;
    }else if(NIVEL_JOGANDO <=8){
        QTD_FRUTAS = 5;
        JOGADAS = 25;
    }else if(NIVEL_JOGANDO <=12){
        QTD_FRUTAS = 6;
        JOGADAS = 30;
    }else if(NIVEL_JOGANDO <=16){
        QTD_FRUTAS = 7;
        JOGADAS = 40;
    }
    srand(NIVEL_JOGANDO);
    for(i=0;i<4;i++){
        for(j=0;j<4;j++){
            tabuleiro[i][j] = rand()%QTD_FRUTAS + 1;
        }
    }

    mudanca = ha_combinacao();
    SysCtlDelay(300000);

    while(mudanca != 0){
        renovar_tabuleiro();
        mudanca = ha_combinacao();
     }

    PONTUACAO = 0;

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);

    imprimir_tabuleiro();
    Nokia5110_PrintBMP2(10, 11,atin_pont, 63, 25);
    if(PONTUACAO_NIVEL[NIVEL_JOGANDO]<=999){
        Nokia5110_PrintBMP2(10+35, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO] - PONTUACAO_NIVEL[NIVEL_JOGANDO]%100)/100], 5, 6);
        Nokia5110_PrintBMP2(10+35+6, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO]%100 - PONTUACAO_NIVEL[NIVEL_JOGANDO]%10)/10], 5, 6);
        Nokia5110_PrintBMP2(10+35+12, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO]%10)], 5, 6);

    }else{
        Nokia5110_PrintBMP2(10+35, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO] - PONTUACAO_NIVEL[NIVEL_JOGANDO]%1000)/1000], 5, 6);
        Nokia5110_PrintBMP2(10+35+6, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO]%1000 - PONTUACAO_NIVEL[NIVEL_JOGANDO]%100)/100], 5, 6);
        Nokia5110_PrintBMP2(10+35+12, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO]%100 - PONTUACAO_NIVEL[NIVEL_JOGANDO]%10)/10], 5, 6);
        Nokia5110_PrintBMP2(10+35+18, 11+5,numeros[(PONTUACAO_NIVEL[NIVEL_JOGANDO]%10)], 5, 6);

    }
    Nokia5110_DisplayBuffer();
    SysCtlDelay(20000000);

}
void partida(void){
    int mudanca = 0,recombi = 0;
    while(PONTUACAO < PONTUACAO_NIVEL[NIVEL_JOGANDO] && JOGADAS>0){
       qual_botao2();
       ha_troca();
       imprimir_tabuleiro();
       SysCtlDelay(1000);
       mudanca = ha_combinacao();
       while(mudanca != 0){
           imprimir_tabuleiro();
           SysCtlDelay(10000000);
           renovar_tabuleiro();
           imprimir_tabuleiro();
           SysCtlDelay(900000);
           mudanca = ha_combinacao();
       }
       while(ha_comb_possiveis() == 0){
           recombi++;
       }
       if(recombi && PONTUACAO < PONTUACAO_NIVEL[NIVEL_JOGANDO] && JOGADAS>0){
          SysCtlDelay(10000);
          Nokia5110_PrintBMP2(10, 11,sem_comb, 63, 25);
          Nokia5110_DisplayBuffer();
          SysCtlDelay(20000000);
          Nokia5110_PrintBMP2(10, 11,recomb, 63, 25);
          Nokia5110_DisplayBuffer();
          SysCtlDelay(20000000);
          imprimir_tabuleiro();
          SysCtlDelay(1000000);
       }
       recombi = 0;
    }
    if(PONTUACAO>=PONTUACAO_NIVEL[NIVEL_JOGANDO]){
        SysCtlDelay(20000);
        Nokia5110_PrintBMP2(10, 11,nv_comp, 63, 25);
        if(NIVEL_JOGANDO<=9){
            Nokia5110_PrintBMP2(10+38, 11+4,numeros[NIVEL_JOGANDO], 5, 6);
        }else{
            Nokia5110_PrintBMP2(10+38, 11+4,numeros[(NIVEL_JOGANDO - NIVEL_JOGANDO%10)/10], 5, 6);
            Nokia5110_PrintBMP2(10+38+6, 11+4,numeros[NIVEL_JOGANDO%10], 5, 6);
        }
        Nokia5110_DisplayBuffer();
        SysCtlDelay(20000000);
        if(NIVEL_JOGANDO == NIVEL_ATUAL && NIVEL_ATUAL < 16){
            NIVEL_ATUAL++;
            Nokia5110_PrintBMP2(10, 11,nv_desb, 63, 25);
            if(NIVEL_ATUAL<=9){
                Nokia5110_PrintBMP2(10+38, 11+4,numeros[NIVEL_ATUAL], 5, 6);
            }else{
                Nokia5110_PrintBMP2(10+38, 11+4,numeros[(NIVEL_ATUAL - NIVEL_ATUAL%10)/10], 5, 6);
                Nokia5110_PrintBMP2(10+38+6, 11+4,numeros[NIVEL_ATUAL%10], 5, 6);
            }
            Nokia5110_DisplayBuffer();
            SysCtlDelay(20000000);
        }
        if(NIVEL_ATUAL == 16){
            NIVEL_ATUAL++;
        }
    }else{
        SysCtlDelay(10000);
        Nokia5110_PrintBMP2(10, 11,pont_insu, 63, 25);
        Nokia5110_DisplayBuffer();
        SysCtlDelay(20000000);
        Nokia5110_PrintBMP2(10, 11,nv_inco, 63, 25);
        if(NIVEL_JOGANDO<=9){
            Nokia5110_PrintBMP2(10+38, 11+4,numeros[NIVEL_JOGANDO], 5, 6);
        }else{
            Nokia5110_PrintBMP2(10+38, 11+4,numeros[(NIVEL_JOGANDO - NIVEL_JOGANDO%10)/10], 5, 6);
            Nokia5110_PrintBMP2(10+38+6, 11+4,numeros[NIVEL_JOGANDO%10], 5, 6);
        }
        Nokia5110_DisplayBuffer();
        SysCtlDelay(20000000);
    }
    PONTUACAO = 0;
}

void jogo(void){
    while(NIVEL_ATUAL < 17){
        setup_nivel();
        partida();
    }
    Nokia5110_PrintBMP2(0,0,vitoria,84,48);
    Nokia5110_DisplayBuffer();
    SysCtlDelay(200000000);
}

