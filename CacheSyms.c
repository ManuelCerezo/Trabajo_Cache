/*				NOTAS:
- Bus direcciones -> ADDR: 2^10 = 1KB o 1024Bytes de RAM
- Linea = Bloque
- Bloque = 8bytes
- Cantidad de bloques en la RAM = 1024 / 8 = 128 Bloques en la RAM
- 

*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

typedef struct CacheSyms{
    short int ETQ;
    short int datos[8];
}T_LINEA_CACHE;

//Definicion de varialbes globales del sistema. SIN UTILIZAR DE MOMENTO
int Tiempoglobal=0;
int NFallos=0;

//Definicion de funciones 
void actualizarCACHE(T_LINEA_CACHE *cache,unsigned char *RAM,int ADDRint,int NLINE);
int cogerNBLOCK(int ADDRint); //Funcion para pillar el numero de bloque.
int comprobarCACHE(int ETQ,int NLINE,int PLBR,int ADDRint,T_LINEA_CACHE *cache,unsigned char *RAM); //Funcion para comprobar la memoria cache con el ADDR descompuesto.
int cogerETQ(int ADDRint);  //Funciones para pillar los diferentes campos de los ADDRS
int cogerNLINE(int ADDRint);
int cogerPLBR(int ADDRint); 
int conversorHexDec(char *ADDR); //Conversion de direcciones str: HEX a int: DEC
void primeracarga(T_LINEA_CACHE *cache,unsigned char *RAM); //Funcion para hacer una primera carga en la cache (Opcional - Borrado)
void inicializacion(T_LINEA_CACHE *cache); //funcion para inicializar la memoria cache
void imprimircache(T_LINEA_CACHE *cache); //funcion para imprimir la cache
void cargarRAM(unsigned char *RAM,FILE *pfile); //funcion para cargar la memoria RAM del archivo binario

int main(){
	
    T_LINEA_CACHE *cache;
    FILE *pfile =NULL;
    FILE *ADDRfile=NULL; //Puntero al fichero ADDR de direcciones
    char ADDR[4]; //ADDR -> Bus de direcciones
    int ADDRint=0;
    int ETQ = 0;
    int NLINE = 0;
    int PLBR = 0;
    int i=0;
    unsigned char RAM[1024]; //Declaracion de la memoria RAM	
	
    // numPtr = (int *)malloc( tamanyo*sizeof(int) )
    cache = (T_LINEA_CACHE*)malloc(sizeof(T_LINEA_CACHE)*4); //Reserva dinamica de memoria para construir la cache.
    
	//Apertura de ficheros.
    if ((ADDRfile = fopen("accesos_memoria.txt", "r")) == NULL){ // "r" -> Lectura solo del fichero
    printf ( " Error en la apertura. Es posible que el fichero no exista \n ");
    }
    if ((pfile = fopen("RAM.bin", "rb")) == NULL){ // "rb" -> Lectura solo del fichero binario
    printf ( " Error en la apertura. Es posible que el fichero no exista \n ");
    }
    	
	inicializacion(cache);
	cargarRAM(RAM,pfile);
	imprimircache(cache);
	
	for(i=0;i<12;i++){	
		printf("=====================================================================\n");
		fscanf(ADDRfile,"%s",ADDR);
		conversorHexDec(ADDR);
		ADDRint = conversorHexDec(ADDR);
    	PLBR=cogerPLBR(ADDRint);
   		NLINE=cogerNLINE(ADDRint);
    	ETQ = cogerETQ(ADDRint);
		while(comprobarCACHE(ETQ,NLINE,PLBR,ADDRint,cache,RAM) == 1 );
    	imprimircache(cache);
    	sleep(2);
	}
    
    return 0;
}
 
//Funcion de comparacion y direccionamiento
int comprobarCACHE(int ETQ,int NLINE,int PLBR,int ADDRint,T_LINEA_CACHE *cache,unsigned char *RAM){ //Func para comprobar la ETQ en cache

	//printf("Numero --> %X\n",ADDRint);
	
	if(ETQ == cache[NLINE].ETQ){
		printf("T: %d, Acierto de CACHE, ADDR %04X ETQ %X linea %02X palabra %02X DATO %02X\n",Tiempoglobal,ADDRint,ETQ,NLINE,PLBR,cache[NLINE].datos[PLBR]); 
		Tiempoglobal = Tiempoglobal + 10;
		return 0;
		
	}else{
		NFallos ++;
		printf("T: %d, Fallo de CACHE %d, ADDR %04X ETQ %X linea %02X palabra %02X bloque %02X\n",Tiempoglobal,NFallos,ADDRint,ETQ,NLINE,PLBR,cogerNBLOCK(ADDRint));
		printf("Actualizando Cache...: Cargando el Bloque: %d en la linea: %d \n",cogerNBLOCK(ADDRint),NLINE);sleep(2);
		actualizarCACHE(cache,RAM,ADDRint,NLINE);
		Tiempoglobal = Tiempoglobal + 10;
		return 1;
	}
}

void actualizarCACHE(T_LINEA_CACHE *cache,unsigned char *RAM,int ADDRint,int NLINE){ //Funcion para actualizar la cache con el bloque correspondiente de la RAM
	
	//Instanciamiento del bloque j en la linea k. -> Nlinea = j mod k -> Corresponde con NLINE.
	
	int i = 0;
	int Nbloque = cogerNBLOCK(ADDRint);
	
	for(i=0;i<8;i++){ //Actualizacion del bloque de la RAM.
		cache[NLINE].datos[i]=RAM[(Nbloque*8)+i];
	}
	
	//La Etiqueta de la cache se actualiza : Etiqueta = Cociente(Bbloque/4)
	cache[NLINE].ETQ = (int)(Nbloque/4);
	
	//printf("La etiqueta actualizada es: %d\n",cache[NLINE].ETQ);
	/*printf("ADDRint: %d\n",ADDRint);
	printf("El numero de bloque es :%d\n",Nbloque);
	printf("el numero de linea es: %d\n",NLINE);*/
	
}

int cogerNBLOCK(int ADDRint){		//Funcion para pillar el numero de bloque en la RAM 
	return (((ADDRint)&(1016))>>3); // Numero 1016 en binario ->1111111000
}

int cogerPLBR(int ADDRint){
	return ((ADDRint)&(7)); // El 7 en binario: 0111
}
int cogerNLINE(int ADDRint){
	return (((ADDRint)&(24))>>3); // el 24 en binario :11000 y desplaza para la derecha 3 bits
}
int cogerETQ(int ADDRint){
	return (((ADDRint)&(992))>>5);// el 992 en binario: 1111100000 y desplaza para la derecha 5 bits
}

int conversorHexDec(char *ADDR){ //Funcion de conversion de str: HEX a int: DEC
	//Tabla de correspondencia.
	int A = 10; int i = 0;
	int B = 11; int h = 3; 
	int C = 12; int acc = 0;
	int D = 13; int aux = 0; 
	int E = 14;
	int F = 15;
	//printf("La direccion en la funcion es: %c\n",ADDR); --> Array por puntero perfectamente.
	//printf("El ADDR es: %s\n",ADDR);
	
	for(i=3 ; i>=0; i--){
		
		switch(ADDR[i]){
		case 'A':
			aux = A*pow(16,(h-i));
			break;
		case 'B':
			aux = B*pow(16,(h-i));
			break;
		case 'C':
			aux = C*pow(16,(h-i));
			break;
		case 'D':
			aux = D*pow(16,(h-i));
			break;
		case 'E':
			aux = E*pow(16,(h-i));
			break;
		case 'F':
			aux = F*pow(16,(h-i));
			break;
		default: //Todos aquellos casos que no sean Letra.
			aux = (ADDR[i] - '0')*pow(16,(h-i));
		}
		acc = acc + aux;
	}
	//printf("El numero en decimal es: %d\n",acc);
	return acc;	
}



void cargarRAM(unsigned char *RAM, FILE *pfile){
	int leidos =0;
	int i=0;
	printf(" -Loading Ram Memory\n");
	leidos = fread (RAM, 1, 1024, pfile); //Funcion fread para volcar el arhivo binario a la "RAM memmory"
	
/*	if(leidos == 1024){ 				  //Animacion de carga
		printf(" [");
		for(i=0;i<15;i++){
			printf("=");
			if(i==14){
				printf("] \n");
			}
			sleep(1);
	}
		printf(" -RAM memory sucesfully load\n");
	}*/
}

/*
void primeracarga(T_LINEA_CACHE *cache,unsigned char *RAM){ //Esta funcion no esta implementada en el proyecto.
    int i = 0;
    int j = 0;
    int ACC = 0;
	printf(" \n\n-Loading Cache memory");sleep(1);printf(".");sleep(1);printf(".");sleep(1);printf(".\n");
	printf("-Cache memory Load\n");
	sleep(1);
    for(i=0;i<4;i++){
    	cache[i].ETQ = Nbloque; //Actualizacion de la etiqueta
    	Nbloque++;
        for(j=0;j<8;j++){
            cache[i].datos[j]=RAM[j+ACC];
            if(j == 7){
            	ACC = ACC + j; //Actualizacion del acumulador
			}
        }
        ACC++; //Actualizacion del acumulador 2.0
    }
}*/

void inicializacion(T_LINEA_CACHE *cache){
    int i = 0;
    int j = 0;
    for (i=0; i < 4; i++){
        cache[i].ETQ = 0xFF;
        for(j=0;j<8;j++){
            cache[i].datos[j]=0; //inicializacion de todos los datos a 0
        }
    }
}

void imprimircache(T_LINEA_CACHE *cache){
	printf("--------------------------\n");
	printf("       CACHE MEMORY\n");
	printf("--------------------------\n");
    int i = 0;
    int j = 0;
    for (i=0; i < 4; i++){
    	if(cache[i].ETQ == 0){
    		printf("ETQ: 0X0  ");	
		}else{	
        	printf("ETQ: %#X  ",cache[i].ETQ); //Para que imprima en Hexadecimal es: %#X
        }
        printf("Datos: ");
		for(j=7;j>=0;j--){
            printf("%X ",cache[i].datos[j]);
            if(j==0){
            	printf("\n");
            }
        }		
    }
}




