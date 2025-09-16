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
    IniciaMV(MV);
    CargaArchivoYPreparaTabla(MV);
    while(MV.Registros[3]!=-1 || MV.Registros[3]>MV.Tabla[tamancod] || MV->ERROR!=-1){
        IP(MV);
        SelectorOperaciones(MV);
    }
    if (MV->ERROR!=-1 && MV.Registros[3]<=MV.Tabla[tamancod]){//VERIFICA SI EL ERROR NO SE DA POR QUE SIMPLEMENTE SE TERMINO EL CODIGO
        switch(MV->ERROR){
            case 0://ERROR EN LA APRTURA DEL ARCHIVO
            break
            case 1://TRATA DE SETEAR UN INMEDIATO
            break
            case 2://ERROR DE SEGMENTACION
            break
            case 3:
            break
            case 4://ERROR DE DIVISION POR CERO
            break
            case 5:;//EL ARCHIVO NO ES DEL FORMATO CORRECTO
            break
        }
    }

}





//PREPARACION PARA LA EJECUCION

void CargaArchivoYPreparaTabla(TMV MV){
    TEXT *arch;
    float identificador;
    char version,byte;
    unsigned short tamanocod,i;
    if (arch=fopen(nombre,"r")==NULL)
        MV->ERROR=0;//ERROR EN LA APRTURA DEL ARCHIVO
    else{
        fscanf(arch,"%f %c %u",&identificador,&version,&tamanocod);
        if (identificador=="VMX25" && version==1){
            i=0;//POR EL MOMENTO, YA QUE EL SEGMENTO DE CODIGO SIEMPRE VA EN EL 0 DE LA MEMORIA.
            while (!feof(arch)){
                fscanf(arch,"%c",&byte);
                MV.Memoria[i]=byte;
                i++;
            }
            MV.Registros[26]=MV.Registros[3]=0;//SETEA CS
            MV.Registros[27]=i;//SETEA DS

            //ACTUALIZAR TABLA DE SEGMENTOS
            MV.Tabla[0,0]=0;
            MV.Tabla[0,1]=tamanocod;
            MV.Tabla[1,0]=tamanocod;
            MV.Tabla[1,1]=TamanoMem-tamanocod
        else
            MV->ERROR=5;//EL ARCHIVO NO ES DEL FORMATO CORRECTO.
        }
    }
}







//OPERADORES EJECUCION

void IP(TMV MV){//"DESENCRIPTA" LA OPERACION, CARGA LOS REGISTROS OPC, OP1 Y OP 2.
    char instruccion;
    float aux;
    instruccion=MV.Memoria[MV.Registros[3]];
    MV.Registros[4]=0;
    MV.Registros[5]=0;
    MV.Registros[6]=0;
    //PONGO EN CERO LOS REGISTROS POR SI ACASO
    MV.Registros[4]=instruccion && 0x000FFFFF;//CARGA DEL OPC
    MV.Registros[3]++;
    //CARGO EN EL BYTE MAS ALTO DE AMBOS REGISTROS DE OPERANDO EL TIPO DE CADA OPERANDO
    //COMIENZO LA PREPARACION DE LA CARGA DE LOS REGISTROS DE OPERANDO
    instruccion=MV.Memoria[MV.Registros[3]];
    MV.Registros[6]=instruccion&&0xFF000000;
    instruccion=MV.Memoria[MV.Registros[3]];
    MV.Registros[5]=instruccion&&0x00FF0000;
    MV.Registros[5]=MV.Registros[5]<<1*8;
    //PARA AMBOS REGISTROS, VALIENDOME DE UN AUX CARGO LA CANTIDAD DE BYTES CORRESPONDIENTES SEGUN EL TIPO DE DATO,
    //LOS IF SECUENCIALES AHORRAN CODIGO Y ADAPTAN MEDIANTE EL AUX LA SUMA PARA FACILITAR LA CARGA DE LOS OPERANDOS.
    //EL REGISTRO IP TERMINA EN EL CAMPO SIGUIENTE, ES DECIR UN MNEMONICO.
    for (i=6,i>4,i--){
        aux=0;
        if (MV.Registros[i]>0x00000000){ // TIPO 1
            MV.Registros[3]++;
            aux+=MV.Memoria[MV.Registros[3]];
        }
        if (MV.Registros[i]>0x01000000){ //TIPO 2
            MV.Registros[3]++;
            aux = aux<<1*8;
            MV.Registros[i]+=MV.Memoria[MV.Memoria[3]];
        }
        if (MV.Registros[i]>0x02000000){ // TIPO 3
            MV.Registros[3]++;
            aux = aux<<1*8
            MV.Registros[i] += MV.Memoria[MV.Memoria[3]];
        }
        MV.Registros[i] += aux;
    }
}

void SelectorOperaciones(TMV MV){//EN LA EJECUCION SE LLAMA CON EL SELECTOR DE OPERANCIONES ESPECIALIZADO EN EL REGISTRO OPC Y CON LOS CAMPOS DE LOS REGISTROS QUE CORRESPONDEN A LOS OPERADORES
    switch(MV.Registros[4]){
        case '0x00':SYS();
        break
        case '0x01':JMP();
        break
        case '0x02':JZ();
        break
        case '0x003':JP();
        break
        case '0x04':JN();
        break
        case '0x05':JNZ();
        break
        case '0x06':JNP();
        break
        case '0x07':JNN();
        break
        case '0x08':NOT();
        break
        case '0x0F':STOP();
        break
        case '0x10':MOV();
        break
        case '0x11':ADD();
        break
        case '0x12':SUB();
        break
        case '0x13':MUL();
        break
        case '0x14':DIV();
        break
        case '0x15':CMP();
        break
        case '0x16':SHL();
        break
        case '0x17':SHR();
        break
        case '0x18':SAR();
        break
        case '0x19':AND();
        break
        case '0x1A':OR();
        break
        case '0x1B':XOR();
        break
        case '0x1C':SWAP();
        break
        case '0x1D':LDL();
        break
        case '0x1E':LDH();
        break
        case '0x1F':RND();
        break
    }

}







//OPERADORES INTERNOS DE LA MAQUINA VIRTUAL

void OperaMemoria

int LogAFis(TMV MV, unsigned int dir_log){ //EN LA DIRECCION DEBE RECIBIR 4 BYTES, 2 corresponden a la base del segmento y 2 corresponden al offset dentro de la misma
    int segmento,offset,dir_fis;
    segmento=(dir_log && 0xFFFF0000)>>2*8;//SHIFTEAR PARA EL DATO
    offset=dir_log  && 0x0000FFFF;
    dir_fis=MV.Tabla[segmento].Base+offset;
    if (dir_fis>MV.Tabla[segmento].Base+MV.Tabla[segmento].Tamano || dir_fis<MV.Tabla[segmento].Base)
        MV->ERROR=2;//ERROR DE SEGMENTACION:   CAIDA POSTERIOR || CAIDA ANTERIOR, HAY QUE VERIFICAR EN LA VERIFICACION DE ERRORES SI LA CAIDA ES POR FIN DEL PROGRAMA
    return dir_fis;
}

int get(TMV MV, int OP1){ //GET DEVUELVE EN FORMA DE INMEDIATO LO QUE SEA EL OPERADOR, YA QUE FUE CARGADO CON UN VALOR QUE MEZCLA TIPO DE DATO Y VALOR
    unsigned int base,tipo;
    int valor,aux=0;
    tipo=(OP1>>3*8)&&0x000000FF;
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
    tipo=(OP1>>3*8)&&0x000000FF;//USA UN MNEMONICO COMO OPERADOR INTERNO
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

void CC(TMV MV,float valor){
    MV.Registros[17]=0x00000000;
    if (valor==0)//BIT Z
        MV.Registros[17]=0x40000000;
    if (valor<0)//BIT N
        MV.Registros[17]=0x80000000;
}









//MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS

void SAR(TMV MV){
    short unsigned int i;
    MV.Registros[5]=MV.Registros[5]>>1;
    MV.Registros[5]=MV.Registros[5]&&0x7FFFFFFF;
    for (i=0,i<MV.Registros[6]-1,i++){
        MV.Registros[5]=MV.Registros[5]>>1;
    }
}

void MOV(TMV MV){
    set(MV,MV.Registros[5],get(MV,MV.Registros[6])));
}

void ADD(TMV MV){
    set(MV,MV.Registros[5],get(MV,MV.Registros[5])+get(MV,MV.Registros[6]));
    CC(MV,MV.Registros[5]-MV.Registros[6]);
}

void SUB(TMV MV){
    set(MV,MV.Registros[5],get(MV,MV.Registros[5])-get(MV,MV.Registros[6]));
    CC(MV,MV.Registros[5]-MV.Registros[6]);
}

void MUL(TMV MV){
    set(MV,MV.Registros[5],get(MV,MV.Registros[5])*get(MV,MV.Registros[6]));
    CC(MV,MV.Registros[5]-MV.Registros[6]);
}

void DIV(TMV MV){
    if (MV.Registros[6]==0)
        MV->ERROR=4;//ERROR DE DIVISION POR CERO
    else{
        set(MV,MV.Registros[5],get(MV,MV.Registros[5]) / get(MV,MV.Registros[6]));//GUARDA EL COCIENTE EN OP1
        set(MV,MV.Registros[16],get(MV,MV.Registros[5]) % get(MV,MV.Registros[6]));//GUARDA EL RESTO EN AC
        CC(MV,MV.Registros[5]-MV.Registros[6]);
    }
}

void cmp(TMV MV){
    CC(MV,MV.Registros[5]-MV.Registros[6]);
}

void SHL(TMV MV){
    MV.Registros[5]=MV.Registros[5]<<MV.Registros[6];
}

void SHR(TMV MV){
    MV.Registros[5]=MV.Registros[5]>>MV.Registros[6];
}

void AND(TMV MV){
    MV.Registros[5]=MV.Registros[5]&&MV.Registros[6];
}

void OR(TMV MV){
    MV.Registros[5]=MV.Registros[5]||MV.Registros[6];
}

void XOR(TMV MV){
    if (MV.Registros[5]!=MV.Registros[6])
        MV.Registros[5]=1;
    else
        MV.Registros[5]=0;
}

void SWAP(TMV MV){
    int aux;
    aux=get(MV,MV.Registros[5]);
    MV.Registros[5]=MV.Registros[6];
    MV.Registros[6]=aux;
}

void LDH(TMV MV){
    int aux;
    MV.Registros[5]=MV.Registros[5]&&0x0000FFFF
    aux=MV.Registros[6];
    MV.Registros[5]+=aux<<2*8;
}

void LDL(TMV MV){
    int aux;
    MV.Registros[5]=MV.Registros[5]&&0xFFFF0000;
    aux=MV.Registros[6];
    MV.Registros[5]+=aux&&0x0000FFFF;
}

void  RND(TMV MV){
	MV.Registros[5]=rand() % MV.Registros[6];
}

void SYS(TMV MV){//HACER PROCEDIMIENTO DE CARGA DE LAR, MAR Y MBR.

}

void JMP(TMV MV){
    MV.Registros[3]=get(MV,MV.Registros[5]);
}

void JZ(TMV MV){
    if (MV.Registros[17]&&0x40000000==0)
        MV.Registros[3]=MV.Registros[5];
}
void JP(TMV MV){
    if ((MV.Registros[17]&&0x40000000==0)&&(MV.Registros[17]&&0x80000000==0))
        MV.Registros[3]=MV.Registros[5];
}

void JN(TMV MV){
    if ((MV.Registros[17]&&0x40000000==1)&&(MV.Registros[17]&&0x80000000==0))
        MV.Registros[3]=MV.Registros[5];
}

void JNZ(TMV MV){
    if (MV.Registros[17]&&0x40000000==0)
        MV.Registros[3]=MV.Registros[5];
}

void JNP(TMV MV){
    if (MV.Registros[17]&&0x40000000==1 && MV.Registros[17]&&0x80000000==0 || MV.Registros[17]&&0x40000000==0 && MV.Registros[17]&&0x80000000==1)
        MV.Registros[3]=MV.Registros[5];
}

void JNN(TMV MV){
    if (MV.Registros[17]&&0x80000000==0)
        MV.Registros[3]=MV.Registros[5];
}

void NOT(TMV MV){
    MV.Registros[5]=-1*MV.Registros[5]
}

void STOP(TMV MV){
    MV.Registros[3]=-1;
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
