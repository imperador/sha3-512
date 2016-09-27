#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define BSIZE 1600

int exclusiveOR(int x, int y);
void buildStateArray(char*** A, char* stringEntrada, int w);
void cleanStdin();
void stringFromstateArray(char*** A, char* stringEntrada, int w);
void thetaStep(char***A, int w);
void roStep(char***A, int w);
void piStep(char***A, int w);
void quiStep(char***A, int w);
void iotaStep(char***A, int w, int roundIndex);
void keccak_p(char***A, char stringEntrada[BSIZE], int b, int roundIndex);
void Rnd(char***A, int roundIndex, int w);
void sha3_512(char***A, char stringEntrada[BSIZE+10]);
void sponge(int rate, char*** A, char nString[BSIZE+10], int d);
void pad10_1(char* padString, int x, int m);
void keccak(char*** A, char stringEntrada[BSIZE+10], int d);
int rc(int t);

void main(int argc, char const *argv[]){	
	// Declaração
	char stringEntrada[BSIZE+10];
	int w,i,j;
	char*** A;

	A = (char***) malloc(sizeof(char***)*5);
	for (i = 0; i < 5; ++i){
		A[i] = (char**) malloc(sizeof(char**)*5);
		for (j = 0; j < 5; ++j){
			A[i][j] = (char*) malloc(sizeof(char*)*w);
		}
	}
	scanf("%s",stringEntrada);
	printf("LEU\n");
	sha3_512(A,stringEntrada);
	printf("Resultado %s\n", stringEntrada);
}

void sha3_512(char*** A, char stringEntrada[BSIZE+10]){
	char stringAux[3] = "01\0";

	strcat(stringEntrada,stringAux);
	keccak(A,stringEntrada,512);
	//keccak_p(A,stringEntrada,strlen(stringEntrada),1);
}

void keccak(char*** A, char stringEntrada[BSIZE+10], int d){
	int c = d*2;
	int r = BSIZE - c;
	sponge(r,A,stringEntrada,d);
}

void sponge(int rate, char*** A, char nString[BSIZE+10], int d){
	char pString[BSIZE+10], sString[BSIZE+1];
	char padString[rate], cString[BSIZE - rate];
	char ** pStringsVector;
	char zString[BSIZE*2],zTruncString[rate+1];
	int n,c,i,j=0,k=0;

	// P = N pad()
	strcpy(pString,nString);
	pad10_1(padString,rate,strlen(nString));
	strcat(pString,padString);

	printf("%s\n", pString);
	printf("%d\n", rate);

	n = strlen(pString)/rate;
	c = BSIZE - rate;

	printf("%d\n", n);

	// 0^c
	for (i = 0; i < c; ++i){
		cString[i] = '0';
	}
	cString[i] = '\0';

	// P Strings
	pStringsVector = (char **)malloc(sizeof(char*)*n);
	for (i = 0; i < n; ++i){
		pStringsVector[i] = (char*)malloc(sizeof(char)*(rate+c+2));
		for (j = 0; j < rate; ++j){
			pStringsVector[i][j] = pString[k];
			k++;
		}
		pStringsVector[i][j] = '\0';
		printf("%s\n", pStringsVector[i]);

		strcat(pStringsVector[i],cString);
	}

	// S = 0^b
	for (i = 0; i < BSIZE; ++i){
		sString[i] = '0';
	}

	for (i = 0; i < n-1; ++i){
		for (j = 0; j < BSIZE; ++j){
			sString[j] = sString[j] ^ pStringsVector[i][j];
		}
		keccak_p(A,sString,BSIZE,24);
	}

	// Do Z Things

	zString[0] = '\0';
	do{
		// Z = Z + Trunc_r(S)
		for(i = 0; i < rate; ++i){
			zTruncString[i] = sString[i];
		}
		zTruncString[i] = '\0';
		strcat(zString,zTruncString);

		if (strlen(zString) >= d){
			zString[d] =  '\0';
			strcpy(nString,zString);
			return;
		}
		keccak_p(A,sString,BSIZE,24);
	} while (1);
}


void keccak_p(char***A, char stringEntrada[BSIZE], int b, int roundNumber){
	int i,l, roundIndex;
	int limite;
	int w = b/25;

	l = log2(b/25);
	limite = 12 + (2*l);
	
	buildStateArray(A,stringEntrada,w);

	for(roundIndex = (limite - roundNumber); roundIndex < (limite -1); ++roundIndex){
		Rnd(A,roundIndex,w);
	}
	stringFromstateArray(A,stringEntrada,w);

}

void Rnd(char*** A, int roundIndex, int w){
	thetaStep(A,w);
	roStep(A,w);
	piStep(A,w);
	quiStep(A,w);
	iotaStep(A,w,roundIndex);
}

// Cosntrói o state array
void buildStateArray(char*** A, char* stringEntrada, int w){
	int i,j,k;
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A[i][j][k] = stringEntrada[w*(w*j + i) + k];
			}
		}
	}
}

// Cosntrói o state array
void stringFromstateArray(char*** A, char* stringEntrada, int w){
	int i,j,k;
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				stringEntrada[w*(w*j + i) + k] = A[i][j][k];
			}
		}
	}
}

// computa a paridade de cada uma das 5w (320, quando w = 64) colunas de 5 bits e aplica um XOR em duas colunas próximas usando um padrão
void thetaStep(char*** A, int w){
	int i, j, k;
	int C[5][w], D[5][w], A2[5][5][w];

	// First step of Theta function, building the matrix C
	for (i = 0; i < 5; ++i){
		for (k = 0; k < w; ++k){
			C[i][k] = A[i][0][k] ^ A[i][1][k] ^ A[i][2][k] ^ A[i][3][k] ^ A[i][4][k];
			//C[i][k] = exclusiveOR(A[i][0][k],exclusiveOR(A[i][1][k],exclusiveOR(A[i][2][k],exclusiveOR(A[i][3][k],exclusiveOR(A[i][4][k])))));
		}
	}

	// Second step: Building the matrix D
	for (i = 0; i < 5; ++i){
		for (k = 0; k < w; ++k){
			D[i][k]= C[(i-1)%5][k] ^ C[(i+1)%5][(k-1)%w];
		}
	}

	// Gera o resultado dos testes de paridade
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A2[i][j][k] = A[i][j][k] ^ D[i][k];
			}
		}
	}

	// Atribui os valores à matriz A
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A[i][j][k] = A2[i][j][k];
			}
		}
	}
	printf("~~THETA\n");
}

void roStep(char*** A, int w){
	int aux, k, i = 1, j = 0, t;
	int A2[5][5][w];

	for (k = 0; k < w; ++k){
		A2[0][0][k] = A[0][0][k];
	}

	for (t = 0; t < 24; ++t){
		for (k = 0; k < w; ++k){
			A2[i][j][k] = A[i][j][(j- ((t+1)*(t+2)/2)) % w];
			aux = i;
			i = j;
			j = (((2*aux) + (3*j)) % 5);
		}
	}

	// Atribui os valores à matriz A
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A[i][j][k] = A2[i][j][k];
			}
		}
	}
	printf("~~RO\n");
}

void piStep(char***A, int w){
	int i, j ,k;

	int A2[5][5][w];

	// Atribuicao dos valores rearranjados das lanes
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A2[i][j][k] = A[(i+(3*j)) % 5][i][k];
			}
		}
	}

	// Atribui os valores à matriz A
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A[i][j][k] = A2[i][j][k];
			}
		}
	}
	printf("~~PI\n");
}

void quiStep(char***A, int w){
	int i, j ,k;

	int A2[5][5][w];

	// Atribuicao dos valores rearranjados das lanes
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A2[i][j][k] = A2[i][j][k] ^ ((A[(i+1)%5][j][k] ^ 1) * (A[(i+2)%5][j][k]));
			}
		}
	}

	// Atribui os valores à matriz A
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A[i][j][k] = A2[i][j][k];
			}
		}
	}
	printf("~~QUI\n");	
}

void iotaStep(char***A, int w, int roundIndex){
	int i,j,k,l;
	int RC[w],A2[5][5][w];;

	l = log2(w);

	// Atribuicao dos valores rearranjados das lanes
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A2[i][j][k] = A[i][j][k];
			}
		}
	}

	// Montagem do vetor RC
	for (k = 0; k < w; ++k){
		RC[k] = 0;
	}

	for (j = 0; j <= l; ++j){
		RC[(2^j) -1] = rc(j + (7*roundIndex));
	}

	for (j = 0; j < w; ++j){
		A2[0][0][j] = A2[0][0][j] ^ RC[j];
	}

	// Atribuicao dos valores finais a matriz A
	for (i = 0; i < 5; ++i){
		for (j = 0; j < 5; ++j){
			for (k = 0; k < w; ++k){
				A[i][j][k] = A2[i][j][k];
			}
		}
	}

	printf("~~IOTA\n");	
}


int rc(int t){
	int R[9] = {0,1,0,0,0,0,0,0,0}, i;
	int modulo = t % 255, j;
	if (!modulo){
		return 1;
	}
	for (i = 1; i <= modulo; ++i){
		R[0] =0;
		R[0] = R[0] ^ R[8];
		R[4] = R[4] ^ R[8];
		R[5] = R[5] ^ R[8];
		R[6] = R[6] ^ R[8];
		for (j = 0; j < 8; ++j){
			R[j+1] = R[j];
		}
	}

	return R[1];
}

void pad10_1(char* padString, int x, int m){
	int i, j = ((-1*m)-2) % x;
	
	if (j<0){
		j *= -1;
	}

	padString[0] = '1';
	for (i= 0; i < j; ++i){
		padString[i+1] = '0';
	}
	padString[i+1] = '1';
	padString[i+2] = '\0';
}

void cleanStdin(){
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}