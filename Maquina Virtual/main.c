#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipos.h"
#include "prototipos.h"

#define MAX_OPERACIONES 32


void carga_programa(TMaquinaVirtual*, char[]);

void carga_operaciones_y_mnemonicos(TRegOp[]);

void carga_nombres_registros(nombre_r[]);

void disassembler(TMaquinaVirtual, TRegDisassembler, TRegOp[], nombre_r[]);


int main(){

    TMaquinaVirtual maquina_virtual;
    unsigned short int dir_fisica_instruccion;
    unsigned char tipo_op[2], byte;
    int aux_op;
    TRegOp operaciones[MAX_OPERACIONES];
    TRegDisassembler info_dis;
    nombre_r nom_reg[CANT_REGISTROS];

    carga_programa(&maquina_virtual, NOMBRE_ARCH);
    carga_operaciones_y_mnemonicos(operaciones);
    carga_nombres_registros(nom_reg);

    while (maquina_virtual.registros[3] != -1 && !maquina_virtual.cod_error){
        log_a_fis(&maquina_virtual, maquina_virtual.registros[3], 1, &dir_fisica_instruccion);
        info_dis.indice = -1;
        if (!maquina_virtual.cod_error){
            //leo el primer byte de la instruccion
            byte = maquina_virtual.memoria[dir_fisica_instruccion];
            //guardo dir fisica de inicio de instruccion para el disassembler
            info_dis.direccion = dir_fisica_instruccion;
            info_dis.instruccion_completa[++info_dis.indice] = byte;

            //cargo OPC
            maquina_virtual.registros[4] = byte & 0x1F;

            //extraigo tipos de operando
            tipo_op[1] = shr(byte & 0xC0, 6);
            tipo_op[0] = shr(byte & 0x30, 4);

            //leo los operandos, arrancando por OP2
            for (int i=1; i>=0; i--){
                maquina_virtual.registros[5+i] = tipo_op[i] << 24;
                aux_op = 0;                                                                 //uso directamente los registros de operandos
                for (int j=0; j<tipo_op[i]; j++){
                    log_a_fis(&maquina_virtual, ++maquina_virtual.registros[3], 1, &dir_fisica_instruccion);        //preincremento el IP antes de calcular dir fisica
                    byte = maquina_virtual.memoria[dir_fisica_instruccion];
                    aux_op <<= 8;
                    aux_op |= byte;
                    info_dis.instruccion_completa[++info_dis.indice] = byte;
                }
                maquina_virtual.registros[5+i] |= (aux_op & 0x3FFFFFFF);
            }
            if (!maquina_virtual.registros[5] && maquina_virtual.registros[6]){                                     //si habia un solo operando significa que lo guarde en OP2
                maquina_virtual.registros[5] = maquina_virtual.registros[6];                                        //entonces paso OP2 a OP1
                maquina_virtual.registros[6] = 0;
            }

            //muevo IP a proxima instruccion
            maquina_virtual.registros[3]++;


            //si no me cai del CS al leer la instruccion, ejecuto la operacion segun su codigo
            if (!maquina_virtual.cod_error)
                if (operaciones[maquina_virtual.registros[4]].funcion && tipo_op[0] <= 3 && tipo_op[1] <= 3){
                    disassembler(maquina_virtual, info_dis, operaciones, nom_reg);
                    operaciones[maquina_virtual.registros[4]].funcion(&maquina_virtual);
                }
                else
                    maquina_virtual.cod_error = INVALIDOP;
            else{
                maquina_virtual.registros[3] = -1;
                maquina_virtual.cod_error = 0;
            }
        }
        else{
            maquina_virtual.registros[3] = -1;
            maquina_virtual.cod_error = 0;
        }
    }

    //si hay error imprime
    switch (maquina_virtual.cod_error){
        case SEGFAULT:  printf("\nSEGMENTATION FAULT\n");
                        break;
        case INVALIDOP: printf("\nINVALID OPERATION\n");
                        break;
        case DIVZERO:   printf("\nDIVISION BY ZERO\n");
                        break;
    }

    printf("\n");
    return 0;
}


void carga_programa(TMaquinaVirtual *mv, char nom_archivo[]){

    FILE *arch;
    //TRegCabecera cabecera;
    unsigned short int celda=0;
    char byte, identificador[LONG_ID], version;
    unsigned char tam_aux;
    short int tam_codigo;

    arch = fopen(nom_archivo, "rb");
    if (arch == NULL){
        printf("El archivo ejecutable no existe.\n");
        (*mv).registros[3] = -1;
    }
    else{
        fread(identificador, sizeof(identificador)-1, 1, arch);
        identificador[LONG_ID-1] = '\0';
        fread(&version, sizeof(version), 1, arch);
        fread(&tam_codigo, 1, 1, arch);
        tam_codigo <<= 8;
        fread(&tam_aux, 1, 1, arch);
        tam_codigo |= tam_aux;

        if (!strcmp(identificador, "VMX25") && version == 1 && tam_codigo <= TAM_MEMORIA){
            //carga codigo a memoria
            fread(&byte, sizeof(char), 1, arch);
            while (!feof(arch)){
                (*mv).memoria[celda++] = byte;
                fread(&byte, sizeof(char), 1, arch);
            }

            //carga tabla de segmentos
            (*mv).tabla_segmentos[CS].base = 0;
            (*mv).tabla_segmentos[CS].tam = tam_codigo;
            (*mv).tabla_segmentos[DS].base = tam_codigo;
            (*mv).tabla_segmentos[DS].tam = TAM_MEMORIA - tam_codigo;

            //inicializa registros en general
            for (int i=0; i<CANT_REGISTROS; i++)
                (*mv).registros[i] = 0;

            (*mv).registros[26] = CS << 16;     //registro CS
            (*mv).registros[27] = DS << 16;     //registro DS
            (*mv).registros[3] = (*mv).registros[26];   //registro IP

        }
        else{
            printf("El archivo ejecutable es invalido.\n");
            (*mv).registros[3] = -1;
        }
        fclose(arch);
    }
    (*mv).cod_error = 0;
}




void carga_operaciones_y_mnemonicos(TRegOp operaciones[]){

    for (int i=0; i<MAX_OPERACIONES; i++){
        operaciones[i].funcion = NULL;
    }
    //carga puntero a operaciones
    operaciones[16].funcion = &mov;
    operaciones[17].funcion = &add;
    operaciones[18].funcion = &sub;
    operaciones[19].funcion = &mul;
    operaciones[20].funcion = &div_mv;
    operaciones[21].funcion = &cmp;
    operaciones[22].funcion = &shl;
    operaciones[23].funcion = &shr_mv;
    operaciones[24].funcion = &sar;
    operaciones[25].funcion = &and;
    operaciones[26].funcion = &or;
    operaciones[27].funcion = &xor;
    operaciones[28].funcion = &swap;
    operaciones[29].funcion = &ldl;
    operaciones[30].funcion = &ldh;
    operaciones[31].funcion = &rnd;
    operaciones[0].funcion = &sys;
    operaciones[1].funcion = &jmp;
    operaciones[2].funcion = &jz;
    operaciones[3].funcion = &jp;
    operaciones[4].funcion = &jn_mv;
    operaciones[5].funcion = &jnz;
    operaciones[6].funcion = &jnp;
    operaciones[7].funcion = &jnn;
    operaciones[8].funcion = &not;
    operaciones[15].funcion = &stop;
    //carga mnemonicos
    strcpy(operaciones[16].nombre, "MOV");
    strcpy(operaciones[17].nombre, "ADD");
    strcpy(operaciones[18].nombre, "SUB");
    strcpy(operaciones[19].nombre, "MUL");
    strcpy(operaciones[20].nombre, "DIV");
    strcpy(operaciones[21].nombre, "CMP");
    strcpy(operaciones[22].nombre, "SHL");
    strcpy(operaciones[23].nombre, "SHR");
    strcpy(operaciones[24].nombre, "SAR");
    strcpy(operaciones[25].nombre, "AND");
    strcpy(operaciones[26].nombre, "OR");
    strcpy(operaciones[27].nombre, "XOR");
    strcpy(operaciones[28].nombre, "SWAP");
    strcpy(operaciones[29].nombre, "LDL");
    strcpy(operaciones[30].nombre, "LDH");
    strcpy(operaciones[31].nombre, "RND");
    strcpy(operaciones[0].nombre, "SYS");
    strcpy(operaciones[1].nombre, "JMP");
    strcpy(operaciones[2].nombre, "JZ");
    strcpy(operaciones[3].nombre, "JP");
    strcpy(operaciones[4].nombre, "JN");
    strcpy(operaciones[5].nombre, "JNZ");
    strcpy(operaciones[6].nombre, "JNP");
    strcpy(operaciones[7].nombre, "JNN");
    strcpy(operaciones[8].nombre, "NOT");
    strcpy(operaciones[15].nombre, "STOP");
}
