#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TamanoMem 16384
#define SegmentosMem 2
#define CantRegs 32

typedef unsigned char TMemoria[TamanoMem];
typedef struct{
    int Base;
    int Tamano;
}TSegmento;
typedef TSegmento TTabla[SegmentosMem];
typedef int TRegistros[CantRegs];
typedef struct{
    TTabla Tabla;
    TRegistros Registros;
    TMemoria Memoria;
    int ERROR;
    unsigned int Disassembler;
}TMV;
typedef char TReg[6];

int main(int argc, char *argv[]){
    int auxDireccion;
    int i;
    TMV MV;
    char mnemonico[5];
    TReg Regs[CantRegs]={"LAR","MAR","MBR","IP","OPC","OP1","OP2","","","","EAX","EBX","ECX","EDX","EEX","EFX","AC","CC","","","","","","","","","CS","DS","","","",""};
    MV.ERROR=-1;
    CargaArchivoYPreparaTabla(&MV);
    MV.Registros[3]=0;
    MV.Disassembler=1;
    //if (argc > 1) {
       // if (strcmp(argv[1], "-d") == 0) {
           // MV.Disassembler = 1;  // Activar disassembler
       // }
    //}

    while(MV.Registros[3]>=0 && MV.Registros[3]<(MV.Tabla[0].Base+MV.Tabla[0].Tamano) && MV.ERROR<0){//PROBLEMA ACA!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        auxDireccion=MV.Registros[3];
        IP(&MV);
        SelectorOperaciones(&MV,mnemonico,Regs);
        //printf("OP1= %x, OP2= %x\n",MV.Registros[5],MV.Registros[6]);
        if (MV.Disassembler)
            Disassembler(&MV,mnemonico,auxDireccion,Regs);
    }
    if (MV.ERROR!=-1 && MV.Registros[3]<=MV.Tabla[1].Base){//VERIFICA SI EL ERROR NO SE DA POR QUE SIMPLEMENTE SE TERMINO EL CODIGO
        switch(MV.ERROR){
            case 0://ERROR EN LA APRTURA DEL ARCHIVO
                printf("ERROR DE APERTURA DE ARCHIVO");
            break;
            case 2://ERROR DE SEGMENTACION
                printf("ERROR DE SEGMENTACION");
            break;
            case 3:
                printf("ERROR DE LLAMADA A SISTEMA INVALIDA");
            break;
            case 4://ERROR DE DIVISION POR CERO
                printf("ERROR DE DIVISION POR CERO");
            break;
            case 5://EL ARCHIVO NO ES DEL FORMATO CORRECTO
                printf("ERROR DE ARCHIVO");
            break;
            case 6://CARGA ERRONEA DE BINARIO
                printf("ERROR EN CARGA DE NRO BINARIO");
            break;
            case 7://INSTRUCCION INVALIDA
                printf("ERROR INSTRUCCION INVALIDA");
            break;
        }
    }
    else
        printf("\nNO HUBO ERRORES\n");//IMPRESION SEGMENTO MEMORIA\n");
    for(i=MV.Tabla[0].Base;i<MV.Tabla[1].Base+50;i++)
        printf("\ndir %d=%d",i,MV.Memoria[i]);
    for(i=0;i<=32;i++)
        printf("\n%s=%d",Regs[i],MV.Registros[i]);
    return 0;
}

//PREPARACION PARA LA EJECUCION PREPARACION PARA LA EJECUCION PREPARACION PARA LA EJECUCION PREPARACION PARA LA EJECUCION PREPARACION PARA LA EJECUCION PREPARACION PARA LA EJECUCION PREPARACION PARA LA EJECUCION

void CargaArchivoYPreparaTabla(TMV *MV){
    FILE *arch;
    char identificador[6];
    char byte,version;
    int tamanocod,i;
    arch=fopen("C:\\Users\\juane\\OneDrive\\Desktop\\MAQUINA_VIRTUAL\\Archivo.vmx","rb");
    if (arch==NULL)
        MV->ERROR=0;//ERROR EN LA APRTURA DEL ARCHIVO
    else{
        tamanocod=0;
        fread(&identificador,5,1,arch);
        fread(&version,1,1,arch);
        fread(&byte,1,1,arch);
        tamanocod=byte;
        tamanocod<<8;
        fread(&byte,1,1,arch);
        tamanocod=tamanocod|byte;
        if (strcmp(identificador,"VMX25")==0 && version==1){
            i=0;//POR EL MOMENTO, YA QUE EL SEGMENTO DE CODIGO SIEMPRE VA EN EL 0 DE LA MEMORIA.
            while (fread(&byte,1,1,arch)==1){
                MV->Memoria[i]=byte;
                i++;
            }
            //ACTUALIZAR TABLA DE SEGMENTOS
            MV->Tabla[0].Base=0;
            MV->Tabla[0].Tamano=tamanocod;
            MV->Tabla[1].Base=tamanocod+1;
            MV->Tabla[1].Tamano=TamanoMem-tamanocod;
            MV->Registros[26]=MV->Registros[3]=MV->Tabla[0].Base;
            MV->Registros[27]=MV->Tabla[1].Base;
        }
        else
            MV->ERROR=5;//EL ARCHIVO NO ES DEL FORMATO CORRECTO.
    }
}

//OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION OPERADORES EJECUCION

void IP(TMV *MV){//"DESENCRIPTA" LA OPERACION, CARGA LOS REGISTROS OPC, OP1 Y OP 2.
    int aux;
    unsigned short int i,j;
    MV->Registros[4]=MV->Memoria[MV->Registros[3]] & 0b00011111;//CARGA DEL OPC
    //CARGO EN EL BYTE MAS ALTO DE AMBOS REGISTROS DE OPERANDO EL TIPO DE CADA OPERANDO
    //COMIENZO LA PREPARACION DE LA CARGA DE LOS REGISTROS DE OPERANDO
    MV->Registros[6]=((MV->Memoria[MV->Registros[3]] & 0b11000000)>>6)<<3*8;
    MV->Registros[5]=((MV->Memoria[MV->Registros[3]] & 0b00110000)>>4)<<3*8;
    //EL REGISTRO IP TERMINA EN EL CAMPO SIGUIENTE, ES DECIR UN MNEMONICO.
    for (i=6;i>4;i--){
        aux=0;
        for(j=0;j<(MV->Registros[i]&0xFF000000)>>3*8;j++){
            MV->Registros[3]++;
            aux=(aux<<8)|MV->Memoria[MV->Registros[3]];
        }
        MV->Registros[i] = MV->Registros[i]|aux;
    }
    if (MV->Registros[5]==0){
        MV->Registros[5]=MV->Registros[6];
        MV->Registros[6]=0;
    }
    MV->Registros[3]=MV->Registros[3]+1;
}

void SelectorOperaciones(TMV *MV,char mnemonico[4], TReg Regs){//EN LA EJECUCION SE LLAMA CON EL SELECTOR DE OPERANCIONES ESPECIALIZADO EN EL REGISTRO OPC Y CON LOS CAMPOS DE LOS REGISTROS QUE CORRESPONDEN A LOS OPERADORES
    switch(MV->Registros[4]){
        case 0x00:
            SYS(MV);
            strcpy(mnemonico,"SYS");
        break;
        case 0x01:
            JMP(MV);
            strcpy(mnemonico,"JMP");
        break;
        case 0x02:
            JZ(MV);
            strcpy(mnemonico,"JZ");
        break;
        case 0x003:
            JP(MV);
            strcpy(mnemonico,"JP");
        break;
        case 0x04:
            JN(MV);
            strcpy(mnemonico,"JN");
        break;
        case 0x05:
            JNZ(MV);
            strcpy(mnemonico,"JNZ");
        break;
        case 0x06:
            JNP(MV);
            strcpy(mnemonico,"JNP");
        break;
        case 0x07:
            JNN(MV);
            strcpy(mnemonico,"JNN");
        break;
        case 0x08:
            NOT(MV);
            strcpy(mnemonico,"NOT");
        break;
        case 0x0F:
            STOP(MV);
            strcpy(mnemonico,"STOP");
        break;
        case 0x10:
            MOV(MV);
            strcpy(mnemonico,"MOV");
        break;
        case 0x11:
            ADD(MV);
            strcpy(mnemonico,"ADD");
        break;
        case 0x12:
            SUB(MV);
            strcpy(mnemonico,"SUB");
        break;
        case 0x13:
            MUL(MV);
            strcpy(mnemonico,"MUL");
        break;
        case 0x14:
            DIV(MV);
            strcpy(mnemonico,"DIV");
        break;
        case 0x15:
            CMP(MV);
            strcpy(mnemonico,"CMP");
        break;
        case 0x16:
            SHL(MV);
            strcpy(mnemonico,"SHL");
        break;
        case 0x17:
            SHR(MV);
            strcpy(mnemonico,"SHR");
        break;
        case 0x18:
            SAR(MV);
            strcpy(mnemonico,"SAR");
        break;
        case 0x19:
            AND(MV);
            strcpy(mnemonico,"AND");
        break;
        case 0x1A:
            OR(MV);
            strcpy(mnemonico,"OR");
        break;
        case 0x1B:
            XOR(MV);
            strcpy(mnemonico,"XOR");
        break;
        case 0x1C:
            SWAP(MV);
            strcpy(mnemonico,"SWAP");
        break;
        case 0x1D:
            LDL(MV);
            strcpy(mnemonico,"LDL");
        break;
        case 0x1E:
            LDH(MV);
            strcpy(mnemonico,"LDH");
        break;
        case 0x1F:
            RND(MV);
            strcpy(mnemonico,"RND");
        break;
        default:
            MV->ERROR=7;
            MV->Disassembler=0;
    }
}

void Disassembler(TMV *MV,char mnemonico[],int Direccion,TReg Regs[CantRegs]){//PROCESO DISASEMBLER, GUARDO EN STRINGS LOS OPERANDOS
    typedef char TImpresion[15];
    int i,largo,aux;
    TImpresion operandos[2];
    unsigned char stringaux[9];
    printf("[%04X] ",Direccion);
    largo=1+((MV->Memoria[Direccion]&0b11000000)>>6)+((MV->Memoria[Direccion]&0b00110000)>>4);
    for(i=0;i<largo;i++){//LARGO ES LA CANTIDAD DE BYTES CORRESPONDIENTE A LA SUMA DEL LARGO DE LOS DOS OPERANDOS, SE HACE CON MENOS O IGUAL YA QUE SE NECESITA UNO MAS, DE LA INSTRUCCION
        printf("%02X ",MV->Memoria[Direccion]);
        Direccion++;
    }
    for(i=0;i<10-largo;i++)
        printf(" ");
    printf("| ");
    for(i=5;i<7;i++){
        stringaux[0]='\0';
        aux=MV->Registros[i]>>3*8;
        switch(aux){
            case 1:
                strcpy(operandos[i-5],Regs[MV->Registros[i]&0x00FFFFFF]);
            break;
            case 2:
                itoa(MV->Registros[i]&0x00FFFFFF,stringaux,10);
                strcpy(operandos[i-5],stringaux);
            break;
            case 3:
                strcpy(operandos[i-5],"[");

                if (MV->Registros[i]&0x00FF0000)
                    strcat(operandos[i-5],Regs[(MV->Registros[i]&0x00FF0000)>>2*8]);

                if (MV->Registros[i]&0x0000FFFF){
                    strcat(operandos[i-5],"+");
                    itoa(MV->Registros[i]&0x0000FFFF,stringaux,10);
                    strcat(operandos[i-5],stringaux);
                }
                strcat(operandos[i-5],"]");
            break;
            case 0:
                strcpy(operandos[i-5],"");
            break;
        }
    }
    printf("%s %s, %s\n",mnemonico,operandos[0],operandos[1]);
}

//OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL OPERADORES INTERNOS DE LA MAQUINA VIRTUAL

int LogAFis(TMV *MV,int dir_log){ //EN LA DIRECCION DEBE RECIBIR 4 BYTES, 2 corresponden a la base del segmento y 2 corresponden al offset dentro de la misma
    //recibi 0x000D0000
    int segmento,offset,dir_fis,i;
    segmento=(dir_log & 0x00FF0000)>>2*8;//SHIFTEAR PARA SACAR EL REGISTRO
    segmento=MV->Registros[segmento];//segmento queda con en valor que hay guardado en el registro
    offset=dir_log & 0x0000FFFF;
    dir_fis=segmento+offset;
    for(i=0;i<SegmentosMem;i++){
        if(segmento>=MV->Tabla[i].Base && segmento<=MV->Tabla[i].Tamano){
            segmento=i;
        }
    }
    if (dir_fis<MV->Tabla[segmento].Base || dir_fis>MV->Tabla[segmento].Base+MV->Tabla[segmento].Tamano){
        MV->ERROR=2;//ERROR DE SEGMENTACION:   CAIDA POSTERIOR || CAIDA ANTERIOR, HAY QUE VERIFICAR EN LA VERIFICACION DE ERRORES SI LA CAIDA ES POR FIN DEL PROGRAMA
        MV->Disassembler=0;
    }
    return dir_fis;
}
//PARA GET Y SET FALTA EL MAR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int get(TMV *MV, int OP1){ //GET DEVUELVE EN FORMA DE INMEDIATO LO QUE SEA EL OPERADOR, YA QUE FUE CARGADO CON UN VALOR QUE MEZCLA TIPO DE DATO Y VALOR
    unsigned int tipo;
    int valor;
    tipo=OP1 & 0xFF000000;
    valor=OP1 & 0x00FFFFFF;
    if (tipo==0x01000000)
        return MV->Registros[valor];
    else{
        if (tipo==0x02000000)
            return valor;
        else{
            MV->Registros[0]=LogAFis(MV,valor);
            MV->Registros[2]=0;
            MV->Registros[2]=MV->Registros[2]|MV->Memoria[MV->Registros[0]+3];
            MV->Registros[2]=MV->Registros[2]|MV->Memoria[MV->Registros[0]+2]<<1*8;
            MV->Registros[2]=MV->Registros[2]|MV->Memoria[MV->Registros[0]+1]<<2*8;
            MV->Registros[2]=MV->Registros[2]|MV->Memoria[MV->Registros[0]]<<3*8;
            return MV->Registros[2];
        }
    }
}

//PARA ECX EN LECTURA Y ESCRITURA PUEDO USAR UN DATO EXTRA, Y HACER IF SECUENCIALES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void set(TMV *MV,int OP1,int OP2){
    unsigned int tipo;
    int valor,aux1,aux2;
    tipo=OP1 & 0xFF000000;
    valor=OP1 & 0x00FFFFFF;
    if (tipo==0x01000000)  //GUARDA EN EL REGISTRO LO CONTENIDO EN OP2
        MV->Registros[valor]=OP2;
    else{//GUARDA EN MEMORIA LO CONTENIDO EN OP2
        MV->Registros[0]=LogAFis(MV,valor);   //GUARDA EN EL SEGMENTO DE MEMORIA CALCULADO LO CONTENIDO EN OP2
        MV->Registros[2]=OP2;
        MV->Memoria[MV->Registros[0]+3]=OP2 & 0x000000FF;
        MV->Memoria[MV->Registros[0]+2]=(OP2 & 0x0000FF00)>>1*8;
        MV->Memoria[MV->Registros[0]+1]=(OP2 & 0x00FF0000)>>2*8;
        MV->Memoria[MV->Registros[0]]=(OP2 & 0xFF000000)>>3*8;
    }
}



void CC(TMV *MV,float valor){
    MV->Registros[17]=0x00000000;
    if (valor==0)//BIT Z!
        MV->Registros[17]=0x40000000;
    if (valor<0)//BIT N
        MV->Registros[17]=0x80000000;
}

void ImprimeBinario(int nro){
    if (nro==0)
        printf("0");
    while (nro!=0){
        printf("%d",nro*0x80000000);
        nro<<1;
    }
}

int LeeBinario(int bits){
    int nro,aux=0;
    unsigned short int i;
    for (i=0;i<bits;i++){
        scanf("%d",&nro);
        if (nro==0 || nro==1){
            aux+=nro;
            aux=aux<<1;
        }
        else
            return 0xFFFFFFFF;
    }
    return aux;
}

//MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS MNEMONICOS

void SAR(TMV *MV){
    short unsigned int i;
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])>>1);
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])&0x7FFFFFFF);
    for (i=0;i<MV->Registros[6]-1;i++){
        set(MV,MV->Registros[5],get(MV,MV->Registros[5])>>1);
    }
}

void MOV(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[6]));
}

void ADD(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])+get(MV,MV->Registros[6]));
    CC(MV,get(MV,MV->Registros[5])-get(MV,MV->Registros[6]));
}

void SUB(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])-get(MV,MV->Registros[6]));
    CC(MV,get(MV,MV->Registros[5])-get(MV,MV->Registros[6]));
}

void MUL(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])*get(MV,MV->Registros[6]));
    CC(MV,get(MV,MV->Registros[5])-get(MV,MV->Registros[6]));
}

void DIV(TMV *MV){
    if (MV->Registros[6]==0)
        MV->ERROR=4;//ERROR DE DIVISION POR CERO
    else{
        set(MV,MV->Registros[5],get(MV,MV->Registros[5]) / get(MV,MV->Registros[6]));//GUARDA EL COCIENTE EN OP1
        set(MV,MV->Registros[16],get(MV,MV->Registros[5]) % get(MV,MV->Registros[6]));//GUARDA EL RESTO EN AC
        CC(MV,MV->Registros[5]-MV->Registros[6]);
    }
}

void CMP(TMV *MV){
    CC(MV,get(MV,MV->Registros[5])-get(MV,MV->Registros[6]));
}

void SHL(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])<<get(MV,MV->Registros[6]));
}

void SHR(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])>>get(MV,MV->Registros[6]));
}

void AND(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])&get(MV,MV->Registros[6]));
}

void OR(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])|get(MV,MV->Registros[6]));
}

void XOR(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[5])^get(MV,MV->Registros[6]));
}

void SWAP(TMV *MV){
    set(MV,MV->Registros[5],get(MV,MV->Registros[6]));
    set(MV,MV->Registros[6],get(MV,MV->Registros[5]));
}

void LDH(TMV *MV){
    MV->Registros[MV->Registros[5]&0x00FFFFFF] = (get(MV,MV->Registros[5])&0x0000FFFF) | ((get(MV,MV->Registros[6])<<2*8) & 0xFFFF0000);
}

void LDL(TMV *MV){
    MV->Registros[MV->Registros[5]&0x00FFFFFF] = (get(MV,MV->Registros[5])&0XFFFF0000) | (get(MV,MV->Registros[6]) & 0x0000FFFF);
}

void  RND(TMV *MV){
    int aux;
    aux==rand() % get(MV,MV->Registros[6]);
	set(MV,MV->Registros[5],aux);
}

void SYS(TMV *MV){
    int aux;
    if (get(MV,MV->Registros[5])==2){//WRITE
        printf("IMPRIME: ");
        switch (MV->Registros[10]){//FORMATO SEGUN EAX
            //CONCEPTUALMENTE, IMPRIMO LO QUE HAY EN UN OPERANDO DE MEMORIA, SIEMPRE EL REGISTRO D
            case 0x08:
                printf("%X\n",get(MV,0x030D0000));//HEXA
            break;
            case 0x04:
                printf("%o\n",get(MV,0x030D0000));//OCTA
            break;
            case 0x10:
                printf("%d\n",get(MV,0x030D0000));//DECIMAL
            break;
            case 0x02:
                printf("%c\n",get(MV,0x030D0000));//CARACTER
            break;
            case 0x01:
                ImprimeBinario(get(MV,0x030D0000));//IMPRIME BINARIO IMPRIME EL NRO BIT X BIT
            break;
        }
    }
    else{
        if (get(MV,MV->Registros[5])==1){//READ
            printf("LEE: ");
            switch (MV->Registros[10]){
                case 0x08:
                    scanf("%X",&aux);
                    set(MV,0x030D0000,aux);
                break;
                case 0x04:
                    scanf("%o",&aux);
                    set(MV,0x030D0000,aux);
                break;
                case 0x10:
                    scanf("%d",&aux);
                    set(MV,0x030D0000,aux);
                break;
                case 0x02:
                    scanf("%c",&aux);
                    set(MV,0x030D0000,aux);
                break;
                case 0x01:
                    aux=LeeBinario((((MV->Registros[13])>>2*8) & 0x0000FFFF)*(MV->Registros[12]*0x0000FFFF));//LECTURA BINARIO HACE INGRESAR LA CANTIDAD DE BITS, NECESITA UN INT
                    if (aux==0xFFFFFFFF)
                        MV->ERROR=6;
                    set(MV,MV->Registros[13]|0x03000000,aux);
                break;
                printf("\n");
            }
        }
        else
            MV->ERROR=3;//LLAMADA A SISTEMA INCORRECTA
    }
}

void JMP(TMV *MV){
    MV->Registros[3]=get(MV,MV->Registros[5]);
}

void JZ(TMV *MV){
    if ((MV->Registros[17] & 0x40000000)==0)
        MV->Registros[3]=MV->Registros[5];
}
void JP(TMV *MV){
    if ((MV->Registros[17] & 0x40000000)==0&&(MV->Registros[17] & 0x80000000)==0)
        MV->Registros[3]=MV->Registros[5];
}

void JN(TMV *MV){
    if ((MV->Registros[17] & 0x40000000)==1&&(MV->Registros[17] & 0x80000000)==0)
        MV->Registros[3]=MV->Registros[5];
}

void JNZ(TMV *MV){
    if ((MV->Registros[17] & 0x40000000)==0)
        MV->Registros[3]=MV->Registros[5];
}

void JNP(TMV *MV){
    if ((MV->Registros[17] & 0x40000000)==1 && (MV->Registros[17] & 0x80000000)==0 || (MV->Registros[17] & 0x40000000)==0 && (MV->Registros[17] & 0x80000000)==1)
        MV->Registros[3]=MV->Registros[5];
}

void JNN(TMV *MV){
    if ((MV->Registros[17] & 0x80000000)==0)
        MV->Registros[3]=MV->Registros[5];
}

void NOT(TMV *MV){
    MV->Registros[5]=-1*MV->Registros[5];
}

void STOP(TMV *MV){
    MV->Registros[3]=-1;
}

/*NECESITO SABER SI SIEMPRE QUE SE GUARDA O LEE DE MEMORIA HAY QUE ACTUALIZAR LAR MAR Y MBR O SOLO CUANDO HAY LLAMADAS A SISTEMA.
TENGO QUE MODIFICAR LAS LLAMADAS A SISTEMA PARA LEER DISTINTAS CANTIDADES SEGUN ECX? COMO LO HAGO?
COMO ES EL FORMATO QUE INDICA QUE HAY QUE ACTIVAR EL DISSASEMBLER?
COMO EJECUTO?????????????????????????????*/
