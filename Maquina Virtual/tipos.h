#define TAM_MEMORIA 16384
#define TAM_TABLA 8
#define CANT_REGISTROS 32
#define TAM_REGISTRO 32
#define MASK 1
#define LONG_ID 6
#define NOMBRE_ARCH "sample.vmx"
#define SEGFAULT 1
#define INVALIDOP 2
#define DIVZERO 3
#define CS 0      //provisorio, puede cambiar
#define DS 1
#define MAX_INSTRUCCION 7
#define TAM_MNEMONICO 5
#define LONG_NOMBRE_R 5




typedef struct {
    unsigned short int base, tam;} TRegTabla;

typedef struct {
    char memoria[TAM_MEMORIA];
    TRegTabla tabla_segmentos[TAM_TABLA];
    int registros[CANT_REGISTROS];
    char cod_error;} TMaquinaVirtual;           //provisoriamente segmentation error es codigo=1
/*
typedef struct {
    char identificador[LONG_ID], version;
    short int tam_codigo;} TRegCabecera;
*/
typedef void (*ptro_func)(TMaquinaVirtual*);

typedef char mnemonico[TAM_MNEMONICO];

typedef struct{
    ptro_func funcion;
    mnemonico nombre;} TRegOp;

typedef struct {
    unsigned short int direccion;
    unsigned char instruccion_completa[MAX_INSTRUCCION], indice;} TRegDisassembler;

typedef char nombre_r[LONG_NOMBRE_R];



