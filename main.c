#include <stdio.h>
#include <stdlib.h>

#define TAM_MEMORIA 16384
#define TAM_TABLA 8
#define CANT_REGISTROS 32
#define TAM_REGISTRO 32
#define MASK 1
#define CS 1      //provisorio, puede cambiar
#define DS 1


typedef struct {
    unsigned short int base, tam;} TRegTabla;

typedef struct {
    char memoria[TAM_MEMORIA];
    TRegTabla tabla_segmentos[TAM_TABLA];
    int registros[CANT_REGISTROS];
    char cod_error;} TMaquinaVirtual;           //provisoriamente segmentation error es codigo=1



/*
 int shr(int reg, char tam, char bits){            //USAMOS EL DE JUAN

    int mascara = MASK;

    reg >>= bits;                                   //hago corrimiento aritmetico la cantidad de bits que quiero

    for (int i=0; i<bits; i++)                      //cuando i=bits llegue al primer bit que quiero descartar
        mascara <<= 1;                              //corro la mascara hacia la izq hasta el primer bit que quiero descartar

    mascara ~= mascara;                             //invierto la mascara: a parti de ahora, me va a importar ir corriendo el 0

    for (int i=bits; i<tam; i++){                   //cuando i=TAM_REGISTRO ya recorri todos los bits
        reg &= mascara;                             //seteo ese bit en 0
        mascara <<= 1;                              //corro la mascara un lugar hacia la izq
    }

    return reg;
}
*/


void log_a_fis(TMaquinaVirtual *mv, int dir_logica, unsigned int tam_acceso, unsigned short int* dir_fisica){

    char cod_seg;
    unsigned short int base_seg;

    cod_seg = shr(dir_logica, 16);

    base_seg = (*mv).tabla_segmentos[cod_seg].base;
    *dir_fisica = base_seg + dir_logica & 0x0000FFFF;        //asigno igual la dir fisica, si hay error al salir no la voy a usar pues terminara el proceso por segmentation error

    if (*dir_fisica + tam_acceso > base_seg + (*mv).tabla_segmentos[cod_seg].tam || *dir_fisica < base_seg)
        (*mv).cod_error = 1;  //con el tamaño de acceso se pasa del bloque o con el offset negativo se sale del bloque
}



void lee_memoria(TMaquinaVirtual *mv){

    int aux=0;
    unsigned short int dir_fisica, tam_acceso;

    dir_fisica = (*mv).registros[1] & 0x0000FFFF;
    tam_acceso = shr((*mv).registros[1]) & 0xFFFF0000, 16);

    aux = (*mv).memoria[dir_fisica++];
    for (int i=1; i<tam_acceso; i++){
        aux <<= 8;
        aux &= (*mv).memoria[dir_fisica++];
    }

    (*mv).registros[2] = aux;
}


void escribe_memoria(TMaquinaVirtual *mv){

    int entrada, aux, mascara = 0xFF000000;
    unsigned short int dir_fisica, tam_acceso;

    dir_fisica = (*mv).registros[1] & 0x0000FFFF;
    tam_acceso = shr((*mv).registros[1]) & 0xFFFF0000, 16);
    entrada = (*mv).registros[2];

    mascara = shr(mascara, (4-tam_acceso)*8);
    for (int i=0; i<tam_acceso; i++){
        aux = entrada & mascara;
        aux = shr(aux, (tam_acceso-i-1)*8);
        (*mv).memoria[dir_fisica] = aux;
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




//en el get, si es operador de memoria

    carga_LAR_MAR(mv, (*mv).registros[(op & 0x001F0000) >> 16] + op & 0x0000FFFF, 4);
    lee_memoria(mv);


//en el set, si el operador es de memoria

    carga_LAR_MAR(mv, (*mv).registros[(op & 0x001F0000) >> 16]) + op & 0x0000FFFF, 4);
    (*mv).registros[2] = entrada; //aca va el valor a escribir en memoria
    escribe_memoria(mv);


void sys_1(TMaquinaVirtual *mv){

    unsigned short int cant_celdas, tam_celdas, dir_logica;

    dir_logica = (*mv).registros[13];
    cant_celdas = shr((*mv).registros[12] & 0xFFFF0000, 16);
    tam_celdas = (*mv).registros[12] & 0x0000FFFF;

    for (int i=0; i<cant_celdas; i++){
        carga_LAR_MAR(mv, dir_logica, tam_celdas);
        printf("[%x] = ", dir_logica);
        switch ((*mv).registros[10]){
            case 0: scanf("%d", &(*mv).registros[2]);
                break;
            case 1: scanf("%c", &(*mv).registros[2]);
                break;
            case 2: scanf("%o", &(*mv).registros[2]);
                break;
            case 3: scanf("%X", &(*mv).registros[2]);
                break;
            case 4: scanf("%b", &(*mv).registros[2]);
                break;
        }
        escribe_memoria(mv);
        dir_logica += tam_celdas;
    }
}


void sys_2(TMaquinaVirtual *mv){

    unsigned short int cant_celdas, tam_celdas, dir_logica;
    char mascara = 1;

    dir_logica = (*mv).registros[13];
    cant_celdas = shr((*mv).registros[12] & 0xFFFF0000, 16);
    tam_celdas = (*mv).registros[12] & 0x0000FFFF;

    for (int i=0; i<cant_celdas; i++){
        carga_LAR_MAR(mv, dir_logica, tam_celdas);
        lee_memoria(mv);
        printf("[%x] = ", dir_logica);
        for (int j=0; j<5; j++){
            if ((*mv).registros[10] & mascara)
                switch (j){
                    case 0: printf("%d\t", (*mv).registros[2]);
                        break;
                    case 1: printf("%c\t", (*mv).registros[2]);
                        break;
                    case 2: printf("0%o\t", (*mv).registros[2]);
                        break;
                    case 3: printf("0x%X\t", (*mv).registros[2]);
                        break;
                    case 4: printf("0b%b\t", (*mv).registros[2]);
                        break;
                }
            mascara <<= 1;
        }
        printf("\n");
        dir_logica += tam_celdas;
    }
}


void lee_instruccion(TMaquinaVirtual *mv){

    unsigned short int dir_logica, dir_fisica, cont_bytes=1, aux;

    dir_logica = (*mv).registros[3];
    log_a_fis(mv, dir_logica, 1, &dir_fisica);

    if (!(*mv).cod_error)){

        (*mv).registros[4] = (*mv).memoria[dir_fisica];

        aux = shr((*mv).registros[4] & 0xC0, 6);
        if (aux)
    }

}
