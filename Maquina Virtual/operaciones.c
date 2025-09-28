#include <stdio.h>
#include <stdlib.h>
#include "tipos.h"
#include "prototipos.h"
#include <time.h>

#define OP1 1
#define OP2 2
#define MASKN 0x80000000
#define MASKZ 0x40000000

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

    aux_op1 = get(mv, OP1);
    aux_op2 = get(mv, OP2);
    aux_op1 &= 0xFFFF0000;
    aux_op2 &= 0xFFFF;
    aux_op1 |= aux_op2;
    set(mv, aux_op1);
}

void ldh(TMaquinaVirtual *mv){

    int aux_op1, aux_op2;

    aux_op1 = get(mv, OP1);
    aux_op2 = get(mv, OP2);
    aux_op1 &= 0xFFFF;
    aux_op2 <<= 16;
    aux_op1 |= aux_op2;
    set(mv, aux_op1);
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
