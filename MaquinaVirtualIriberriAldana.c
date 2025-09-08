#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TamanoMem 1600
#define SegmentosMem 2
#define CantRegs 32

typedef char TMemoria[TamanoMem];
typedef struct{
    short unsigned int Base;
    short unsigned int Tamano;
}TSegmento;
typedef TSegmento TTabla[SegmentosMem];
typedef int TRegistros[CantRegs];
typedef struct TMV{
    TTabla Tabla;
    TRegistros Registros;
    TMemoria Memoria;
    unsigned int *ERROR;
}


int main(){
    TMV MV;
}

//OPERADORES INTERNOS DE LA MAQUINA VIRTUAL

int LogAFis(TMV MV, unsigned int dir_log){ //EN LA DIRECCION DEBE RECIBIR 4 BYTES, 2 corresponden a la base del segmento y 2 corresponden al offset dentro de la misma
    int segmento,offset,dir_fis;
    segmento=(dir_log && 0xFFFF0000)>>2*8;//SHIFTEAR PARA EL DATO
    offset=dir_log  && 0x0000FFFF;
    dir_fis=MV.Tabla[segmento].Base+offset;
    if (dir_fis>MV.Tabla[segmento].Base+MV.Tabla[segmento].Tamano || dir_fis<MV.Tabla[segmento].Base)
        MV->ERROR=2;//ERROR DE SEGMENTACION:   CAIDA POSTERIOR || CAIDA ANTERIOR
    return dir_fis;
}

int get(TMV MV, int OP1){ //GET DEVUELVE EN FORMA DE INMEDIATO LO QUE SEA EL OPERADOR, YA QUE FUE CARGADO CON UN VALOR QUE MEZCLA TIPO DE DATO Y VALOR
    unsigned int base,tipo;
    int valor,aux=0;
    tipo=SLR((OP1&&FF000000),3*8);
    valor=OP1&&0x00FFFFFF;
    if (tipo==2){
        return valor;
    else{
        if (tipo==1)
            return MV.Registros[valor];
        else{
            base=LogAFis(MV,valor);
            aux+=MV.Memoria[base];
            aux+=MV.Memoria[base+1]<<1*8;
            aux+=MV.Memoria[base+2]<<2*8;
            aux+=MV.Memoria[base+3]<<3*8;
            return aux;
        }
    }
}

void set(TMV MV,int OP1,int OP2){//OP2 DEBE SER UN INMEDIATO, LLAMAR CON GET
    unsigned int base,tipo;
    int valor;
    tipo=SLR((OP1&&FF000000),3*8);
    valor=OP1&&0x00FFFFFF;
    if (tipo==2){
        MV->ERROR=1;//TRATA DE SETEAR UN INMEDIATO, NO TIENE SENTIDO, ESTA MAL LA INSTRUCCION
    }
    else{
        if (tipo==1){  //GUARDA EN EL REGISTRO NRO LO CONTENIDO EN OP2
            MV.Registros[valor]=OP2;
        }
        else{
            base=LogAFis(MV,valor);   //GUARDA EN EL SEGMENTO DE MEMORIA CALCULADO LO CONTENIDO EN OP2
            MV.Memoria[base+0]=OP2 && 0x000000FF;
            MV.Memoria[base+1]=(OP2 && 0x0000FF00)>>1*8;
            MV.Memoria[base+2]=(OP2 && 0x00FF0000)>>2*8;
            MV.Memoria[base+3]=(OP2 && 0xFF000000)>>3*8;
        }
    }
}

void CC(TMV MV,int resultado){
    MV.Registros[17]=0x00000000;
    if (resultado==0)
        MV.Registros[17]=0x80000000;
    if (resultado<0)
        MV.Registros[17]=0x40000000;
}

//MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS

int SLR(int OP1,int OP2){   //DEBE RECIBIR 2 INMEDIATOS, ES PURAMENTE MATEMATICO
    short unsigned int i;
    OP1=OP1>>1;
    OP1=OP1&&0x7FFFFFFF;
    for (i=0,i<OP2-1,i++){
        OP1=OP1>>1;
    }
    return OP1;
}

void MOV(TMV MV, int OP1, int OP2){  //LLAMADA CON: (MV,MV.Registros[5],MV.Registros[6]) ES DECIR, EL VALOR QUE HAY EN EL REGISTRO OP1 Y EL VALOR QUE HAY EN EL OP2, AMBOS ENTEROS QUE TIENEN: 1 BYTE DE TIPO Y 3 DE VALOR
    set(MV,OP1,get(MV,OP2)));
}

void ADD(TMV MV, int OP1, int OP2){
    set(MV,OP1,get(MV,OP1)+get(MV,OP2));
    CC(MV,get(MV,OP1));
}

void SUB(TMV MV, int OP1, int OP2){
    set(MV,OP1,get(MV,OP1)-get(MV,OP2));
    CC(MV,get(MV,OP1));
}

void MUL(TMV MV, int OP1, int OP2){
    set(MV,OP1,get(MV,OP1)*get(MV,OP2));
    CC(MV,get(MV,OP1));
}

void DIV(TMV MV, int OP1, int OP2){
    set(MV,OP1,get(MV,OP1) / get(MV,OP2));//GUARDA EL COCIENTE EN OP1
    set(MV,0x00030010,get(MV,OP1) % get(MV,OP2));//GUARDA EL RESTO EN AC    GUARDA CON LA DIVISION POR CERO
    CC(MV,get(MV,OP1));
}

//PARA ERRORES, LOS CARGAMOS EN LA MV Y EVALUAMOS POSTERIOR A LA EJECUCION DE LA INSTRUCCION QUE GENERA EL ERROR.


//CAMBIOS:
//PREPARAR PROCEDIMIENTO DE CARGA DE LAR MAR MBR

/*RESUMEN: En los registros se cargan valores con el formato: 1 byte de tipo y 3 de valor, este valor puede ser la direccion de memoria, el nro de registro o simplemente el valor del inmediato.
Get obitiene el valor, decodificando segun el tipo y devolviendo solo el valor. Solo tiene un operando, que es del que devuelve el valor numerico.
Set guarda en el operenado 1, sea lo que sea el mismo, el valor que se encuentre en operando 2, el mismo debe ser un inmediato, por lo que para set, se llama con el valor del registro y en op1 y con get del op2, es decir el inmediato guardado en op2.
Por la forma en que estan planteados los mnemonicos, los mismos se llaman siempre con: MV,VALOR DEL REG 1,VALOR DEL REG 2, es decir MV.Registros[nro].
El procedimiento cc atualiza el registro cc en base a lo guardado en un operando, se realiza posterior a las operaciones para que el valor sea el actualizado.
Guardamos los errores en la maquina virtual, verificando en el ciclo principar para cortar la ejecucion.
