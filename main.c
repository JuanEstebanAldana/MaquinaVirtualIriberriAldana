#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MEM_DEFAULT 16384
#define MAX_PARAMETROS 20
#define TAM_TABLA 8
#define CANT_SEGMENTOS 6
#define CANT_REGISTROS 32
#define TAM_REGISTRO 32
#define MASK 1
#define LONG_ID 6
#define MAX_ARCH 64
#define SEGFAULT 1
#define INVALIDOP 2
#define DIVZERO 3
#define MEMINS 4
#define CS 0
#define DS 1
#define OP1 1
#define OP2 2
#define MASKN 0x80000000
#define MASKZ 0x40000000
#define MAX_INSTRUCCION 7
#define TAM_MNEMONICO 5
#define LONG_NOMBRE_R 5
#define MAX_OPERACIONES 32
#define ESPACIO_INST 25


//TIPOS

typedef struct {
    char arch_fuente[MAX_ARCH], arch_img[MAX_ARCH];
    unsigned int tam_memoria;
    char d;
    unsigned short int tam_ps;} TRegComandos;

typedef struct {
    char identificador[LONG_ID], version;
    unsigned short int tam_segmentos[TAM_TABLA], offset_entry;} THeaderVMX;

typedef struct {
    unsigned short int base, tam;} TRegTabla;

typedef struct {
    char memoria[TAM_MEMORIA];
    TRegTabla tabla_segmentos[TAM_TABLA];
    int registros[CANT_REGISTROS];
    char cod_error;} TMaquinaVirtual;           //provisoriamente segmentation error es codigo=1

typedef void (*ptro_func)(TMaquinaVirtual*);

typedef char mnemonico[TAM_MNEMONICO];

typedef struct{
    ptro_func funcion;
    mnemonico nombre;} TRegOp;

typedef struct {
    unsigned short int direccion;
    unsigned char instruccion_completa[MAX_INSTRUCCION], indice;} TRegDisassembler;

typedef char nombre_r[LONG_NOMBRE_R];



//DISASSEMBLER

void disassembler(TMaquinaVirtual mv, TRegDisassembler info, TRegOp operaciones[], nombre_r nom_reg[], unsigned short int dir_entry){

    int aux, reg, offset;
    char tipo, car;

    printf("%c[%04X] ", (info.direccion == dir_entry)? ">":" ", info.direccion);

    for (int i=0; i <= info.indice; i++)
        printf("%02X ", info.instruccion_completa[i]);
    printf("%*c", ESPACIO_INST - (info.indice+1) * 3, ' ');

    printf("| ");

    printf("%5s\t", operaciones[mv.registros[4]].nombre);

    for (int i=0; i<2; i++){
        aux = mv.registros[5+i];
        tipo = shr(aux & 0xFF000000, 24);
        switch(tipo){
            case 1: car = nom_reg[aux & 0x1F][1];   //extraigo la letra que identifica al registro (A...F)
                    switch (shr(aux & 0xC0, 6)){
                        case 0: printf("%3s", nom_reg[aux & 0x1F]);
                                break;
                        case 1: printf(" %cL", car);
                                break;
                        case 2: printf(" %cH", car);
                                break;
                        case 3: printf(" %cX", car);
                                break;
                    }
                    break;
            case 2: aux <<= 16;
                    printf("%d", aux >> 16);
                    break;
            case 3: switch (shr(aux & 0xC00000, 22)){
                        case 0: printf(" [");
                                break;
                        case 1: printf("w[");
                                break;
                        case 2: printf("b[");
                                break;
                    }
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


void muestra_cadenas(TMquinaVirtual *mv){

    unsigned short int cont, aux_indice, pos_base, tope, celda;
    char car;

    aux_indice = shr((*mv).registros[30], 16);
    pos_base = (*mv).tabla_segmentos[aux_indice].base;
    tope = pos_base + (*mv).tabla_segmentos[aux_indice].tam;

    while (pos_base < tope){
        printf("[%04X] ", pos_base);

        celda = pos_base;
        cont = 0;
        while ((*mv).memoria[celda] != '\0' && cont < 7){
            printf("%02X ", (*mv).memoria[celda++]);
            cont++;
        }
        if (cont < 7)
            printf("%02X ", (*mv).memoria[celda]);
        else
            printf(".. ");
        cont++;
        printf("%*c", ESPACIO_INST - cont * 3, ' ');
        printf("| ");

        celda = pos_base;
        printf("\"");
        while ((*mv).memoria[celda] != '\0'){
            car = (*mv).memoria[celda++];
            if (32 <= car && car <= 126)
                printf("%02X ", car);
            else
                printf(".");
        }
        printf("\"\n");

        pos_base = celda + 1;
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
    strcpy(nom_reg[7], "SP");
    strcpy(nom_reg[8], "BP");
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
    strcpy(nom_reg[28], "ES");
    strcpy(nom_reg[29], "SS");
    strcpy(nom_reg[30], "KS");
    strcpy(nom_reg[31], "PS");
}





//FUNCIONES INTERNAS

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

    unsigned short int cod_seg, base_seg;

    cod_seg = shr(dir_logica, 16);

    if (cod_seg == 0xFFFF)          //cod_seg es -1, no existe el segmento en el proceso
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
                        case 1: if ((*mv).registros[2] >= 32 && (*mv).registros[2] <= 126)
                                    printf("'%-3c\t", (*mv).registros[2]);
                                else
                                    printf("%-3c\t", '.');
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


void sys_3 (TMaquinaVirtual *mv){


    int i;


    for (i=0; i < mv->registros[12]-1; i++){
        carga_LAR_MAR(mv, mv->registros[13],1);
        if (!mv->cod_error){
            scanf("%c",&mv->registros[2]);
            escribe_memoria(mv);
            mv->registros[1]++;
        }
    }
    if (!mv->cod_error){
        carga_LAR_MAR(mv,++mv->registros[13],1);
        mv->registros[2]='\0';
        escribe_memoria(mv);
    }
}


void sys_4 (TMaquinaVirtual *mv){


    carga_LAR_MAR(mv, mv->registros[13],1)
    lee_memoria(mv);
    while (!mv->cod_error && mv->registros[2]!='\0'){
        if (mv->registros[2]!='\n')
            printf("%c",mv->registros[2]);
        else
            printf("\n");
        carga_LAR_MAR(mv, ++mv->registros[13],1);
        lee_memoria(mv);
    }
}


void sys_7 (TMaquinaVirtual *mv){


    system("cls");
}


void sys_F (TMaquinaVirtual *mv, TRegComandos comandos){


    char aux='x';


    if (strlen(comandos.arch_img)){
        while (aux !='g' && aux !='q' && aux !='\n')
            scanf("%c",&aux);
        switch(aux){
            case 'g':   mv->debugger=0;
                        break;
            case 'q':   mv->registros[3]=-1;
                        break;
            case '\n':  mv->debugger=1;
                        break;
        }
    }
}



void push(TMaquinaVirtual *mv){
    int valor,i/*,dirfis*/;
    mv->registros[7]-=4;//1)DECREMENTA EL VALOR DEL SP EN 4
    if (mv->registros[7]<mv->tabla_segmentos[mv->registros[29]].base)//2)VERIFICA SI HAY ERROR
        mv->cod_error=STACKOVERFLOW;//STACK OVERFLOW
    else{
        valor=get(mv->registros[5]);//3,4)OBTIENE EL VALOR DEL OPERANDO, GET TRANSFORMA A 4 BYTES
        carga_LAR_MAR(mv,mv->registros[7],4);
        //dirfis=mv->tabla_segmentos[mv->registros[7]>>16].base + mv->registros[7] & 0x0000FFFF;//EL SP ES UN PUNETERO, LO TRANSFORMO A FISICA LOCALMENTE PORQUE POP Y PUSH NO MODIFICAN LAR Y MAR
        for (i=0;i<4; mv->memoria[mv->registros[1]+4-i] = ((char)(valor >> (8*i))))
            i++;
        ;
    }
}


void pop(TMaquinaVirtual *mv){
    int valor,i/*,dirfis*/;
    mv->registros[7]+=4;
    if ((mv->tabla_segmentos[mv->registros[29]].base + mv->tabla_segmentos[mv->registros[29]].tam - (mv->registros[7] & 0x0000FFFF + mv->tabla_segmentos[mv->registros[29]].base)) < 4)
        mv->cod_error=STACKUNDERFLOW;//STACK UNDERFLOW
    else{
        valor=0;
        carga_LAR_MAR(mv,mv->registros[7],4);
        //dirfis=mv->tabla_segmentos[mv->registros[7]>>16].base + mv->registros[7] & 0x0000FFFF;
        for (i=0;i<4;valor=(valor<<8)+mv->memoria[mv->registros[1]+4-i])//LOS VALORES DEL MAR Y EL LAR DEBEN SER ACTUALIZADOS SEGUN LA CELDA QUE SE LEVANTA O ESCRIBE???
            i++;
        ;
        set(mv,valor);
    }
}


void call(TMaquinaVirtual *mv){
    int aux;
    aux=mv->registros[5];
    mv->registros[5]=mv->registros[3];
    push(mv)
    mv->registros[5]=aux;
    jmp(mv);
}
//ESTO ES TEORICAMENTE CUESTIONABLE PERO DESDE LA PROGRAMACION ES AMPIAMENTE MAS SENCILLO
//LA CONSTRUCCION DE ESTOS DOS UTILIZA UN AUXILIAR YA QUE PASA PODER USAR POP Y PUSH HAY QUE PRECISAR DEL OP1, LO QUE GENERA LA NECESIDAD DE SALVAGUARDAR EL VERDADERO OP1, PARA QUE NO SE MODIFIQUE
void ret(TMaquinaVirtual *mv){
    int aux;
    aux=mv->registros[5];
    mv->registros[5]=0x01000003;//EN EL OPERANDO 1 COLOCA EL REGISTRO IP CON LA INTENCION QUE EL POP LO ACTUALICE Y LUEGO DEVUELVE AL OP1 LO QUE TIENE QUE SER CON EL AUX
    pop(mv);
    mv->registros[5]=aux;
}



//OPERACIONES

void mov(TMaquinaVirtual *mv){
    set(mv, get(mv, OP2));
}

void add(TMaquinaVirtual *mv){
    int sum = get(mv, OP1) + get(mv, OP2);
    set(mv, sum);
    cc(mv, sum);
}

void sub(TMaquinaVirtual *mv){
    int res = get(mv, OP1) - get(mv, OP2);
    set(mv, res);
    cc(mv, res);
}

void mul(TMaquinaVirtual *mv){
    int prod = get(mv, OP1) * get(mv, OP2);
    set(mv, prod);
    cc(mv, prod);
}

void div_mv(TMaquinaVirtual *mv){
    int a, b, cociente;
    b = get(mv, OP2);
    if (b){
        a = get(mv, OP1);
        cociente = a / b;
        set(mv, cociente);
        cc(mv, cociente);
        (*mv).registros[16] = a % b;
    }
    else
        (*mv).cod_error = DIVZERO;
}

void cmp(TMaquinaVirtual *mv){
    int aux = get(mv, OP1) - get(mv, OP2);
    cc(mv, aux);
}

void shl(TMaquinaVirtual *mv){
    int aux = get(mv, OP1) << get(mv, OP2);
    set(mv, aux);
    cc(mv, aux);
}

void shr_mv(TMaquinaVirtual *mv){
    int aux = shr(get(mv, OP1), get(mv, OP2));
    set(mv, aux);
    cc(mv, aux);
}

void sar(TMaquinaVirtual *mv){
    int aux = get(mv, OP1) >> get(mv, OP2);
    set(mv, aux);
    cc(mv, aux);
}

void and(TMaquinaVirtual *mv){
    int aux = get(mv, OP1) & get(mv, OP2);
    set(mv, aux);
    cc(mv, aux);
}

void or(TMaquinaVirtual *mv){
    int aux = get(mv, OP1) | get(mv, OP2);
    set(mv, aux);
    cc(mv, aux);
}

void xor(TMaquinaVirtual *mv){
    int aux = get(mv, OP1) ^ get(mv, OP2);
    set(mv, aux);
    cc(mv, aux);
}

void swap(TMaquinaVirtual *mv){

    int aux_valor, aux_op;
    char aux_tipo;

    aux_valor = get(mv, OP1);
    set(mv, get(mv, OP2));

    aux_op = (*mv).registros[6];
    aux_tipo = shr(aux_op & 0xC0000000, 30);

    if (aux_tipo == 1)
        (*mv).registros[aux_op & 0x1F] = aux_valor;
    else{
        carga_LAR_MAR(mv, (*mv).registros[(aux_op & 0x001F0000) >> 16] + (aux_op & 0x0000FFFF), 4);
        (*mv).registros[2] = aux_valor;
        escribe_memoria(mv);
    }
}

void ldl(TMaquinaVirtual *mv){


    int aux_op1, aux_op2;
    char tipo, subtipo;


    aux_op1 = get(mv, OP1);
    aux_op2 = get(mv, OP2);
    tipo = (mv->registros[5] & 0xFF000000)>>24;
    if (tipo == 1){
        subtipo = (mv->registros[5] & 0b11000000)>>6;
        switch (subtipo){
            case 1: aux_op1 &= 0xFFFFFF00;
                    aux_op2 &= 0x000000FF;
                break;
            case 2: aux_op1 &= 0xFFFF00FF;
                    aux_op2 &= 0x000000FF;
                    aux_op2 <<= 8;
                break;
            default: aux_op1 &= 0xFFFF0000;
                     aux_op2 &= 0x0000FFFF;
                break;//el caso default es para la carga de ax o de eax, que a efecto practicos son lo mismo para ldl
        }
        aux_op1 |= aux_op2;
        set(mv, aux_op1);
    }
    else{
        if ((mv->registros[5] & 0xC00000)>>22 == 0){//el ldl en memoria solo se realiza con modificador l o sin modificador, 4 celdas
            aux_op1 &= 0xFFFF0000;
            aux_op2 &= 0x0000FFFF;
            aux_op1 |= aux_op2;
            set(mv, aux_op1);
        }
    }
}




void ldh(TMaquinaVirtual *mv){//PARA LDH, SOLO SE CARGAN COSAS SI SE TRATA DE UN REGISTRO ENTERO O UNA CELDA DE MEMORIA DE 4, POR LO QUE SOLO ES NECESARIO VERIFICAR SI ESTA CONDICION SE DA.


    int aux_op1, aux_op2;


    if (((mv->registros[5] & 0x01000000) && (mv->registros[5] & 0b11000000 == 0)) || ((mv->registros[5] & 0x03000000) && (mv->registros[5] & 0x110000 == 0))){//uso mascaras invertidas
        aux_op1 = get(mv, OP1);
        aux_op2 = get(mv, OP2);
        aux_op1 &= 0xFFFF;
        aux_op2 <<= 16;
        aux_op1 |= aux_op2;
        set(mv, aux_op1);
    }
}


void rnd(TMaquinaVirtual *mv){
    srand(time(NULL));
    set(mv, rand() % (get(mv, OP2) + 1));
}

void sys(TMaquinaVirtual *mv){
    int aux = get(mv, OP1);
    if (aux == 1)
        sys_1(mv);
    else
        sys_2(mv);
}

void jmp(TMaquinaVirtual *mv){
    (*mv).registros[3] = (*mv).registros[26] + get(mv, OP1);
}

void jz(TMaquinaVirtual *mv){
    if ((*mv).registros[17] & MASKZ)
        jmp(mv);
}

void jp(TMaquinaVirtual *mv){
    if (!(*mv).registros[17])
        jmp(mv);
}

void jn_mv(TMaquinaVirtual *mv){
    if ((*mv).registros[17] & MASKN)
        jmp(mv);
}

void jnz(TMaquinaVirtual *mv){
    if (!((*mv).registros[17] & MASKZ))
        jmp(mv);
}

void jnp(TMaquinaVirtual *mv){
    if ((*mv).registros[17])
        jmp(mv);
}

void jnn(TMaquinaVirtual *mv){
    if (!((*mv).registros[17] & MASKN))
        jmp(mv);
}

void not(TMaquinaVirtual *mv){
    set(mv, ~get(mv, OP1));
}

void stop(TMaquinaVirtual *mv){
    (*mv).registros[3] = -1;
}







//MAIN

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


int str_a_num(char *str){

    int sum=0, i=0;
    unsigned char aux;

    while (str[i] != '\0'){
        aux = str[i] - 48;
        sum = sum * 10 + aux;
        i++
    }

    return sum;
}





int valida_extension(char nombre[], char ext_ref[]){

    char *ext_nombre;

    if (strlen(nombre) < 5)
        return 0;
    else{
        ext_nombre = nombre + strlen(nombre) - strlen(ext_ref);
        return !strcmp(ext_nombre, ext_ref);
    }
}


void carga_ps(int argc, char *argv[], int i, TRegComandos *comandos, TMaquinaVirtual *mv){

    unsigned int vec_punt_param[MAX_PARAMETROS];
    unsigned char cont_vec = 0;
    unsigned short int pos_mem = 0;

    while (i < argc){
        vec_punt_param[cont_vec] = 0x0000;
        vec_punt_param[cont_vec] <<= 16;
        vec_punt_param[cont_vec] += pos_mem;
        for (int j=0; j <= strlen(argv[i]); j++){
            (*mv).memoria[pos_mem] = argv[i][j];            //REVISAR SI DEBE SER POSMEM+J O SOLO POSMEM
            pos_mem++;
        }
        cont_vec++;
        i++;
    }
    for (int k=0; k < cont_vec; k++){
        (*mv).memoria[pos_mem] = vec_punt_param[k];
        pos_mem += 4;
    }

    (*comandos).tam_ps = pos_mem;
}




void procesa_comandos(int argc, char *argv[], TRegComandos *comandos, TMaquinaVirtual *mv){

    int i=0;

    if (argc > 1)
        if (valida_extension(argv[1], ".vmx")){
            strcpy((*comandos).arch_fuente, argv[1]);
            if (argc > 2 && valida_extension(argv[2], ".vmi")){
                strcpy((*comandos).arch_img, argv[2]);
                i = 3;
            }
            else{
                strcpy((*comandos).arch_img, "");
                i = 2;
            }
        }
        else{
            strcpy((*comandos).arch_fuente, "");
            if (valida_extension(argv[1], ".vmi")){
                strcpy((*comandos).arch_img, argv[1]);
                i = 2;
            }
        }

    if (i){
        if (i < argc && !strncmp(argv[i], "m=", 2)){
            (*comandos).tam_memoria = str_a_num(argv[i]+2) * 1024;
            i++;
        }
        else
            (*comandos).tam_memoria = MEM_DEFAULT;
        if (i < argc && !strcmp(argv[i], "-d")){
            (*comandos).d = 1;
            i++;
        }
        else
            (*comandos).d = 0;
        if (i < argc && !strcmp(argv[i], "-p") && strlen((*comandos).arch_fuente)){
            i++;
            carga_ps(argc, argv, i, mv, comandos);
        }
        else
            (*comandos).tam_ps = 0;
    }
}




void carga_segmentos(TMaquinaVirtual *mv, THeaderVMX header, TRegComandos comandos){

    int indice_seg=0, aux;

    //verifico si inicializo la tabla con PS
    if (comandos.tam_ps){
        (*mv).tabla_segmentos[0].base = 0;
        (*mv).tabla_segmentos[0].tam = tam_ps;
        (*mv).registros[31] = 0;
        indice_seg++;
    }
    else
        (*mv).registros[31] = -1;

    //verifico si agrego a la tabla el KS, y si ademas es el primero en la tabla por no haber PS. El KS es el ultimo en el header pero va antes de CS en la memoria principal
    if (header.tam_segmentos[4]){
        (*mv).tabla_segmentos[indice_seg].base = (indice_seg)? (*mv).tabla_segmentos[indice_seg-1].tam : 0;
        (*mv).tabla_segmentos[indice_seg].tam = header.tam_segmentos[4];
        aux = indice_seg << 16;
        (*mv).registros[30] = aux;
        indice_seg++;
    }
    else
        (*mv).registros[30] = -1;

    //analizo el CS en particular porque puede ser que, a falta de PS y KS, inicialice la tabla
    (*mv).tabla_segmentos[indice_seg].base = (indice_seg)? (*mv).tabla_segmentos[indice_seg-1].base + (*mv).tabla_segmentos[indice_seg-1].tam : 0;
    (*mv).tabla_segmentos[indice_seg].tam = header.tam_segmentos[0];
    aux = indice_seg << 16;
    (*mv).registros[26] = aux;
    indice_seg++;

    //analizo los segmentos restantes, a partir del DS que es el segundo especificado en el header, hasta el SE, anteultimo en el header
    for (int i=1; i < CANT_SEGMENTOS-2; i++)
        if (header.tam_segmentos[i]){
            (*mv).tabla_segmentos[indice_seg].base = (*mv).tabla_segmentos[indice_seg-1].base + (*mv).tabla_segmentos[indice_seg-1].tam;
            (*mv).tabla_segmentos[indice_seg].tam = header.tam_segmentos[i];
            aux = indice_seg << 16;
            (*mv).registros[26+i] = aux;
            indice_seg++;
        }
        else
            (*mv).registros[26+i] = -1;
}




void carga_programa(TMaquinaVirtual *mv, TRegComandos comandos){

    FILE *arch;
    unsigned short int celda=0, tam, pos_base;
    char byte, aux_indice;
    unsigned char tam_aux, i;
    THeaderVMX header;
    int sum_tam=0;

    arch = fopen(comandos.arch_fuente, "rb");
    if (arch == NULL){
        printf("El archivo ejecutable no existe.\n");
        (*mv).registros[3] = -1;
    }
    else{
        fread(header.identificador, sizeof(header.identificador)-1, 1, arch);
        header.identificador[LONG_ID-1] = '\0';
        fread(&header.version, sizeof(header.version), 1, arch);
        fread(&tam, 1, 1, arch);
        tam <<= 8;
        fread(&tam_aux, 1, 1, arch);
        tam |= tam_aux;
        header.tam_segmentos[0] = tam;
        sum_tam += tam;
        if (header.version == 2){
            i = 1;
            while (i < CANT_SEGMENTOS-1){
                fread(&tam, 1, 1, arch);
                tam <<= 8;
                fread(&tam_aux, 1, 1, arch);
                tam |= tam_aux;
                header.tam_segmentos[i++] = tam;
                sum_tam += tam;
            }
            fread(&header.offset_entry, sizeof(header.offset_entry), 1, arch);
        }

        if (!strcmp(identificador, "VMX25") && sum_tam <= comandos.tam_memoria)){

            carga_segmentos(mv, header, comandos);

            //carga codigo a memoria
            if ((*mv).registros[26] != -1){     //trivial, pero verifico que el CS sea un segmento del proceso
                aux_indice = shr((*mv).registros[26], 16);
                pos_base = (*mv).tabla_segmentos[aux_indice].base;
                celda = pos_base;
                fread(&byte, sizeof(char), 1, arch);
                while (!feof(arch) && celda - pos_base < (*mv).tabla_segmentos[aux_indice].tam){
                    (*mv).memoria[celda++] = byte;
                    fread(&byte, sizeof(char), 1, arch);
                }
            }


            //CARGA CONST
            if ((*mv).registros[30] != -1){     //verifico que el KS sea un segmento en el proceso
                aux_indice = shr((*mv).registros[30], 16);
                pos_base = (*mv).tabla_segmentos[aux_indice].base;
                celda = pos_base;
                while (!feof(arch) && celda - pos_base < (*mv).tabla_segmentos[aux_indice].tam){
                    (*mv).memoria[celda++] = byte;
                    fread(&byte, sizeof(char), 1, arch);
                }
            }



            //inicializa registros en general
            for (int i=0; i<26; i++)
                (*mv).registros[i] = 0;


            (*mv).registros[3] = (*mv).registros[26] + header.offset_entry;   //registro IP; no verifico que exista CS porque, de no existir, la funcion log_a_fis lo detectara


            aux_indice = shr((*mv).registros[29], 16);
            (*mv).registros[7] = (*mv).tabla_segmentos[aux_indice].base + (*mv).tabla_segmentos[aux_indice].tam;    //registro SP; no verifico que exista SS por el mismo motivo que en el caso del IP


            (*mv).cod_error = 0;
        }
        else
            if (sum_tam > comandos.tam_memoria)
                (*mv).cod_error = MEMINS;
            else{
                printf("El archivo ejecutable es invalido.\n");
                (*mv).registros[3] = -1;
            }
        fclose(arch);
    }
}


void carga_imagen(TMaquinaVirtual *mv, TRegComandos comandos){

    FILE *arch;
    char identificador[LONG_ID], version;
    unsigned short int tam;

    arch = fopen(comandos.arch_img, "rb");
    if (arch == NULL){
        printf("El archivo de imagen no existe");
        (*mv).registros[3] = -1;
    }
    else{
        fread(identificador, sizeof(identificador)-1, 1, arch);
        identificador[LONG_ID-1] = '\0';
        fread(&version, sixeof(version), 1, arch);
        fread(&tam, sizeof(tam), 1, arch);

        if (!strcmp(identificador, "VMI25") && version == 1 && tam <= comandos.tam_memoria){
            for (int i=0; i < CANT_REGISTROS; i++)
                fread(&(*mv).registros[i], TAM_REGISTRO, 1, arch);
            for (int i=0; i < TAM_TABLA; i++)
                fread(&(*mv).tabla_segmentos[i], sizeof(TRegTabla), 1, arch);
            for (int i=0; i < comandos.tam_memoria; i++)
                fread(&(*mv).memoria[i], sizeof(char), 1, arch);
        }
        else
            if (tam > comandos.tam_memoria)
                (*mv).cod_error = MEMINS;
            else{
                printf("El archivo ejecutable es invalido.\n");
                (*mv).registros[3] = -1;
            }
        fclose(arch);
    }
}


void guarda_imagen(TMaquinaVirtual *mv, TRegComandos comandos){

    FILE *arch;
    char identificador[LONG_ID], version;
    unsigned short int tam;

    arch = fopen(comandos.arch_img, "wb");

    strcpy(identificador, "VMI25");
    fwrite(identificador, sizeof(identificador), 1, arch);
    version = 1;
    fwrite(&version, sizeof(version), 1, arch);
    tam = comandos.tam_memoria;
    fwrite(&tam, sizeof(tam), 1, arch);

    for (int i=0; i < CANT_REGISTROS; i++)
        fwrite(&(*mv).registros[i], TAM_REGISTRO, 1, arch);

    for (int i=0; i < TAM_TABLA; i++)
        fwrite(&(*mv).tabla_segmentos[i], sizeof(TRegTabla), 1, arch);

    for (int i=0; i < comandos.tam_memoria; i++)
        fwrite(&(*mv).memoria[i], sizeof(char), 1, arch);

    fclose(arch);
}




int main(int argc, char *argv[]){

    TMaquinaVirtual maquina_virtual;
    TRegComandos comandos;
    unsigned short int dir_fisica_instruccion, dir_entry;
    unsigned char tipo_op[2], byte;
    int aux_op;
    TRegOp operaciones[MAX_OPERACIONES];
    TRegDisassembler info_dis;
    nombre_r nom_reg[CANT_REGISTROS];

    carga_operaciones_y_mnemonicos(operaciones);
    carga_nombres_registros(nom_reg);

    procesa_comandos(argc, argv, &comandos, &maquina_virtual);

    if (strlen(comandos.arch_fuente)){
        carga_programa(&maquina_virtual, comandos.arch_fuente);
        //PUSHEA PILA
    }
    else if (strlen(comandos.arch_img))
        carga_imagen(&maquina_virtual, comandos);
    else{
        printf("No existe archivo .vmx ni .vmi: la ejecucion no se puede llevar a cabo.");
        maquina_virtual.registros[3] = -1;
    }

    dir_entry = maquina_virtual.registros[3];

    if (comandos.d && maquina_virtual.registros[30] != -1)
        muestra_cadenas(&maquina_virtual);


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
                maquina_virtual.registros[5+i] |= (aux_op & 0x00FFFFFF);
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
                    if (comandos.d)
                        disassembler(maquina_virtual, info_dis, operaciones, nom_reg, dir_entry);
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
        case SEGFAULT:          printf("\nSEGMENTATION FAULT\n");
                                break;
        case INVALIDOP:         printf("\nINVALID OPERATION\n");
                                break;
        case DIVZERO:           printf("\nDIVISION BY ZERO\n");
                                break;
        case STACKUNDERFLOW:    printf("\nSTACK UNDERFLOW\n");
                                break;
        case STACKOVERFLOW:     printf("\nSTACK OVERFLOW\n");
                                break;

    }

    printf("\n");
    return 0;
}

