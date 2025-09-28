#include <stdio.h>
#include <stdlib.h>
#include "tipos.h"
#include "prototipos.h"

void set(TMaquinaVirtual *mv, int valor){

    int operando;
    char tipo;

    operando = (*mv).registros[5];
    tipo = shr(operando & 0xFF000000, 24);

    if (tipo == 1)
        (*mv).registros[operando & 0x1F] = valor;
    else{
        carga_LAR_MAR(mv, (*mv).registros[(operando & 0x001F0000) >> 16] + (operando & 0x0000FFFF), 4);
        (*mv).registros[2] = valor;
        if (!(*mv).cod_error)
            escribe_memoria(mv);
    }
}


int get(TMaquinaVirtual *mv, char selec_operando){

    int operando, valor;
    char tipo;

    operando = (*mv).registros[4+selec_operando];
    tipo = shr(operando & 0xFF000000, 24);

    switch (tipo){
        case 1: valor = (*mv).registros[operando & 0x1F];
                break;

        case 2: valor = operando;
                valor <<= 16;
                valor >>= 16;           //para retener el signo si es negativo
                break;

        case 3: carga_LAR_MAR(mv, (*mv).registros[(operando & 0x001F0000) >> 16] + (operando & 0x0000FFFF), 4);
                if (!(*mv).cod_error){
                    lee_memoria(mv);
                    valor = (*mv).registros[2];
                }
                break;
    }

    return valor;
}




void cc(TMaquinaVirtual *mv, int num){

    (*mv).registros[17] = 0;
    if (num < 0)
        (*mv).registros[17] |= 0x80000000;
    else
        if (num == 0)
            (*mv).registros[17] |= 0x40000000;
}



int shr(int num, int bits){

    if (bits > 0){
        num >>= 1;
        num &= 0x7FFFFFFF;
        num >>= bits-1;
    }

    return num;
}


void log_a_fis(TMaquinaVirtual *mv, int dir_logica, unsigned int tam_acceso, unsigned short int* dir_fisica){

    unsigned char cod_seg;
    unsigned short int base_seg;

    cod_seg = shr(dir_logica, 16);

    if (cod_seg >= TAM_TABLA)
        (*mv).cod_error = SEGFAULT;
    else{
        base_seg = (*mv).tabla_segmentos[cod_seg].base;
        *dir_fisica = base_seg + dir_logica & 0x0000FFFF;        //asigno igual la dir fisica, si hay error al salir no la voy a usar pues terminara el proceso por segmentation error

        if (*dir_fisica + tam_acceso > base_seg + (*mv).tabla_segmentos[cod_seg].tam || *dir_fisica < base_seg)
            (*mv).cod_error = SEGFAULT;  //con el tamaño de acceso se pasa del bloque o con el offset negativo se baja del bloque
    }
}



void lee_memoria(TMaquinaVirtual *mv){

    int aux=0;
    unsigned short int dir_fisica, tam_acceso;

    dir_fisica = (*mv).registros[1] & 0x0000FFFF;
    tam_acceso = shr((*mv).registros[1] & 0xFFFF0000, 16);

    aux = (*mv).memoria[dir_fisica++];
    for (int i=1; i<tam_acceso; i++){
        aux <<= 8;
        aux |= (*mv).memoria[dir_fisica++] & 0xFF;
    }

    (*mv).registros[2] = aux;
}


void escribe_memoria(TMaquinaVirtual *mv){

    int entrada, aux, mascara = 0xFF000000;
    unsigned short int dir_fisica, tam_acceso;

    dir_fisica = (*mv).registros[1] & 0x0000FFFF;
    tam_acceso = shr((*mv).registros[1] & 0xFFFF0000, 16);
    entrada = (*mv).registros[2];

    mascara = shr(mascara, (4-tam_acceso)*8);
    for (int i=0; i<tam_acceso; i++){
        aux = entrada & mascara;
        aux = shr(aux, (tam_acceso-i-1)*8);
        (*mv).memoria[dir_fisica++] = aux;
        mascara = shr(mascara, 8);
    }
}


void carga_LAR_MAR(TMaquinaVirtual *mv, int dir_logica, unsigned int tam_acceso){

    unsigned short int dir_fisica;

    (*mv).registros[0] = dir_logica;

    (*mv).registros[1] = tam_acceso;
    (*mv).registros[1] <<= 16;

    log_a_fis(mv, dir_logica, tam_acceso, &dir_fisica);

    (*mv).registros[1] |= dir_fisica;

}

void imprime_binario(int num){

    printf("0b");
    for (int i=0; i<32; i++){
        printf("%d", (num & 0x80000000) >> 31);
        num <<= 1;
    }
}

int lee_binario(){

    char bits[33];
    int num = 0;

    scanf("%s", bits);
    for (int i=0; i<32; i++){
        num <<= 1;
        bits[i] = (bits[i]=='1')? 1:0;
        num += bits[i];
    }

    return num;
}


void sys_1(TMaquinaVirtual *mv){

    unsigned short int cant_celdas, tam_celdas, i;
    int  dir_logica;

    dir_logica = (*mv).registros[13];
    cant_celdas = (*mv).registros[12] & 0x0000FFFF;
    tam_celdas = shr((*mv).registros[12] & 0xFFFF0000, 16);

    i=0;
    printf("\n");
    while(i<cant_celdas && !(*mv).cod_error){
        carga_LAR_MAR(mv, dir_logica, tam_celdas);
        if (!(*mv).cod_error){
            printf("[%04X] = ", (*mv).registros[1] & 0xFFFF);
            switch ((*mv).registros[10]){
                case 1: scanf("%d", &(*mv).registros[2]);
                    break;
                case 2: scanf("%c", &(*mv).registros[2]);
                    break;
                case 4: scanf("%o", &(*mv).registros[2]);
                    break;
                case 8: scanf("%X", &(*mv).registros[2]);
                    break;
                case 16: (*mv).registros[2] = lee_binario();
                    break;
            }
            escribe_memoria(mv);
            dir_logica += tam_celdas;
        }
        i++;
    }
    printf("\n");
}


void sys_2(TMaquinaVirtual *mv){

    int dir_logica;
    unsigned short int cant_celdas, tam_celdas, i;
    char mascara;

    dir_logica = (*mv).registros[13];
    cant_celdas = (*mv).registros[12] & 0x0000FFFF;
    tam_celdas = shr((*mv).registros[12] & 0xFFFF0000, 16);

    printf("\n");
    i=0;
    while(i<cant_celdas && !(*mv).cod_error){
        carga_LAR_MAR(mv, dir_logica, tam_celdas);
        if (!(*mv).cod_error){
            lee_memoria(mv);
            printf("[%04X] = ", (*mv).registros[1] & 0xFFFF);
            mascara = 1;
            for (int j=0; j<5; j++){
                if ((*mv).registros[10] & mascara)
                    switch (j){
                        case 0: printf("%-15d\t", (*mv).registros[2]);
                            break;
                        case 1: printf("'%-3c\t", (*mv).registros[2]);
                            break;
                        case 2: printf("0o%-15o\t", (*mv).registros[2]);
                            break;
                        case 3: printf("0x%-15X\t", (*mv).registros[2]);
                            break;
                        case 4: imprime_binario((*mv).registros[2]);
                            break;
                    }
                mascara <<= 1;
            }
            printf("\n");
            dir_logica += tam_celdas;
        }
        i++;
    }
    printf("\n");
}


