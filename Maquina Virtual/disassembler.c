#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipos.h"
#include "prototipos.h"


#define ESPACIO_INST 25


void disassembler(TMaquinaVirtual mv, TRegDisassembler info, TRegOp operaciones[], nombre_r nom_reg[]){

    int aux, reg, offset;
    char tipo;

    printf("[%04X] ", info.direccion);

    for (int i=0; i <= info.indice; i++)
        printf("%02X ", info.instruccion_completa[i]);
    printf("%*c", ESPACIO_INST - info.indice * 3, ' ');

    printf("| ");

    printf("%5s\t", operaciones[mv.registros[4]].nombre);

    for (int i=0; i<2; i++){
        aux = mv.registros[5+i];
        tipo = shr(aux & 0xFF000000, 24);
        switch(tipo){
            case 1: printf("%s", nom_reg[aux & 0x1F]);
                    break;
            case 2: aux <<= 16;
                    printf("%d", aux >> 16);
                    break;
            case 3: printf("%c", '[');
                    reg = shr(aux & 0x1F0000, 16);
                    offset = aux & 0xFFFF;
                    if (reg)
                        printf("%s", nom_reg[reg]);
                    if (reg && offset)
                        printf("+");
                    if (offset)
                        printf("%d", offset);
                    printf("]");
                    break;
        }
        if (tipo)
            if (i == 0 && shr(mv.registros[6] & 0xFF000000, 24))
                printf(", ");
            else
                printf("\n");
    }
}

void carga_nombres_registros(nombre_r nom_reg[]){
    strcpy(nom_reg[0], "LAR");
    strcpy(nom_reg[1], "MAR");
    strcpy(nom_reg[2], "MBR");
    strcpy(nom_reg[3], "IP");
    strcpy(nom_reg[4], "OPC");
    strcpy(nom_reg[5], "OP1");
    strcpy(nom_reg[6], "OP2");
    strcpy(nom_reg[7], "");
    strcpy(nom_reg[8], "");
    strcpy(nom_reg[9], "");
    strcpy(nom_reg[10], "EAX");
    strcpy(nom_reg[11], "EBX");
    strcpy(nom_reg[12], "ECX");
    strcpy(nom_reg[13], "EDX");
    strcpy(nom_reg[14], "EEX");
    strcpy(nom_reg[15], "EFX");
    strcpy(nom_reg[16], "AC");
    strcpy(nom_reg[17], "CC");
    strcpy(nom_reg[18], "");
    strcpy(nom_reg[19], "");
    strcpy(nom_reg[20], "");
    strcpy(nom_reg[21], "");
    strcpy(nom_reg[22], "");
    strcpy(nom_reg[23], "");
    strcpy(nom_reg[24], "");
    strcpy(nom_reg[25], "");
    strcpy(nom_reg[26], "CS");
    strcpy(nom_reg[27], "DS");
    strcpy(nom_reg[28], "");
    strcpy(nom_reg[29], "");
    strcpy(nom_reg[30], "");
    strcpy(nom_reg[31], "");
}
