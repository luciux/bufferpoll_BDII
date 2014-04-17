#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffend.h"



int main(){
	
	//union
	
	FILE *arq, *arq2; //Ponteiros para arquivos
	int valor_reg, tam_registro, pos_inicial_regis;
	pos_inicial_regis = valor_reg = tam_registro = 0;
	
	tp_buffer *bufferpoll = (tp_buffer*)malloc(sizeof(tp_buffer)*PAGES);
	
	initbuffer(bufferpoll);
	
	arq = fopen("file/meta.dat", "r");
	if(arq == NULL){	
		printf("Read Error\n");
		return 0;
	}
	fread(&valor_reg, sizeof(int), 1, arq);
	tp_table *s = (tp_table*)malloc(sizeof(tp_table)*valor_reg); //Aloca um vetor com o numero de colunas da tupla
	tam_registro = load_metadata(arq, s, valor_reg);
	fclose(arq);
	
	arq2 = fopen("file/data.dat", "r");
	if(arq2 == NULL){
		printf("Read Error\n");
		return 0;
	}
	load_data(arq2, bufferpoll, tam_registro);
	fclose(arq2);
	
	//para imprimir
	
	//printbufferpoll(bufferpoll, s, 0, valor_reg); //Parametros são: buffer, estrutura dos meta dados, página a ser impressa e quantos campos tem a "tabela"
	
	cabecalho(s, valor_reg);
	pos_inicial_regis = 1 * tam_registro;
	drawline(bufferpoll, s, valor_reg, &pos_inicial_regis, 0); // (buffer, meta, numero de campos, posicao inicial, pagina)
	printf("\n");
	free(s);
	free(bufferpoll);
	return 0;
}
