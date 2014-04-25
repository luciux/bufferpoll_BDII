#define SIZE 512
#define PAGES 1024
#define TAM 20

int page_avalible; // Variavel global controlando as páginas utilizadas

typedef struct table_bd{
	char nome[TAM];
	char tipo;
	int tam;
}tp_table;

typedef struct buffer{
   unsigned char db; //Dirty bit
   unsigned char pc; //Pin counter
   unsigned int nrec; //numero de registros armazenados na pagina
   char data[SIZE];
}tp_buffer;

union c_double{
	
	double dnum;
	char double_cnum[sizeof(double)];	
};

union c_int{
	
	int  num;
	char cnum[sizeof(int)];
};

void cpystr(char *, char *, int, int);
void initbuffer(tp_buffer *);
void cria_campo(int, int, char *);
void drawline(tp_buffer *, tp_table *, int, int *, int);
int cabecalho(tp_table *, int);
int printbufferpoll(tp_buffer *, tp_table *, int, int);
int load_metadata(FILE *, tp_table *, int);
void load_data(FILE *, tp_buffer *, int);

void cpystr(char *tg, char *sc, int st, int len) //SC = Ponteiro para string que carregada do arquivo; tg = Ponteiro para uma página do buffer
{
	st = st * len;
	
	int i=st,j=0;
	for (;i<len+st;i++)
	  tg[i]=sc[j++];
	
}

void initbuffer(tp_buffer *bp){
	
	int i;

	for (i = 0;i < PAGES; i++){
		bp->db=0;
		bp->pc=0;
		bp->nrec=0;
		bp++;
	}
	page_avalible = 0; // Variavel global controlando as páginas utilizadas
}

void cria_campo(int tam, int header, char *val){
	int i;
	char aux[40];
	
	if(header){
		aux[0] = '|';
		for(i = 1; i <= TAM && val[i-1] != '\0'; i++){
			aux[i] = val[i-1];
		}
		for(;i <= TAM; i++)
			aux[i] = '=';
		aux[i++] = '|';
		aux[i] ='\0';
		printf("%s", aux);	
		
	}
	else{
		for(i = 0; i < tam; i++){
			printf(" ");
		}
		printf("|");
	}
}
void drawline(tp_buffer *buffpoll, tp_table *s, int num_reg, int *pos_ini, int num_page){
	
	int count, pos_aux, bit_pos;
	union c_double cd;
	union c_int ci;
	
	count = pos_aux = bit_pos = 0;
	
	for(count = 0; count < num_reg; count++){
		pos_aux = *(pos_ini);
		bit_pos = 0;
		
		//---para imprimir com espacos corretos
		
		printf("|");
		
		switch(s[count].tipo){
			
			case 'S':
				
				while(buffpoll[num_page].data[pos_aux] != '\0'){
			
					printf("%c", buffpoll[num_page].data[pos_aux]);
					if ((buffpoll[num_page].data[pos_aux++] & 0xc0) != 0x80) bit_pos++; //Conta apenas bits que possam ser impressos (UTF8)
				}
				
				cria_campo((TAM - (bit_pos)), 0, (char*)' ');
				break;
			
			case 'I':
				while(pos_aux < *(pos_ini) + s[count].tam){
					ci.cnum[bit_pos++] = buffpoll[num_page].data[pos_aux++];
				}
				printf("%-20d|", ci.num); //Controla o número de casas até a centena
				break;
				
			case 'D':
				while(pos_aux < *(pos_ini) + s[count].tam){
					cd.double_cnum[bit_pos++] = buffpoll[num_page].data[pos_aux++]; // Cópias os bytes do double para área de memória da union
				}
				printf("%-20.3lf|", cd.dnum);
				break;
			
			case 'C': 
				printf("%c", buffpoll[num_page].data[pos_aux]);
				if(s[count].tam < strlen(s[count].nome)){
					bit_pos = strlen(s[count].nome);
				}
				else{
					bit_pos = s[count].tam;
				}
				cria_campo((bit_pos - 1), 0, (char*)' ');	
				break;
			
			default: printf("Erro de Impressão\n\n\n");
				break;
		}
		*(pos_ini) += s[count].tam;		
	}
	printf("\n");
}

int cabecalho(tp_table *s, int num_reg){
	int count, aux;
	aux = 0;
	
	for(count = 0; count < num_reg; count++){
		cria_campo(s[count].tam, 1, s[count].nome); // O segundo parâmetro significa se = 1 Cabecalho se = 0 é valor daquele campo
		aux += s[count].tam;
	}
	printf("\n");
	return aux;
}

int printbufferpoll(tp_buffer *buffpoll, tp_table *s, int num_page, int num_reg){
	
	int aux, i;
	
	
	if(buffpoll[num_page].nrec == 0){
		return 0;	
	}
	
	i = aux = 0;
	
	aux = cabecalho(s, num_reg);
	
	
	while(i < buffpoll[num_page].nrec * aux){ // Enquanto i < numero de registros * tamanho de uma instancia da tabela
		drawline(buffpoll, s, num_reg, &i, num_page);
	}
	return 1;
}

int load_metadata(FILE *arq_meta, tp_table *s, int valor_reg){
	
	int count, pos;
	int tam_registro;
	pos = tam_registro = 0;
	char c;
	
	for(count = 0; count < valor_reg; count++){	 //Passa pelo numero de ocorrencias da tabela
		pos = 0;
	
		while(fread(&c, sizeof(char), 1, arq_meta) && c != '\0'){ //le o nome do campo
			s[count].nome[pos] = c;
			pos++;
		}
	
		s[count].nome[pos] = c;	//acrescenta o \0 no final da string		
		fread(&c, sizeof(char), 1, arq_meta); //le o tipo (S, I, D, C)
		s[count].tipo = c;
		fread(&s[count].tam, sizeof(int), 1, arq_meta);
		
		tam_registro += s[count].tam;
		//printf("%s %c %d\n", s[count].nome, s[count].tipo, s[count].tam);
		//printf("TAMANHO DA TUPLA: %d\n", tam_registro);
		
	}
	return tam_registro;
}

void load_data(FILE *arq_data, tp_buffer *bufferpool, int tam_registro){
	
	char *lei_aux;
	int page_count;
	page_count = 0;
	
	lei_aux = (char*)malloc(sizeof(char)*tam_registro);
		
	while(fgetc(arq_data) != EOF && page_avalible < PAGES){ //le os dados do arquivo data
	
		fseek(arq_data, -1, 1);		
		fread(lei_aux, sizeof(char)*tam_registro, 1, arq_data);
		
		if(page_count + tam_registro < SIZE){
			cpystr(bufferpool[page_avalible].data, lei_aux, bufferpool[page_avalible].nrec, tam_registro);
			bufferpool[page_avalible].nrec++;
			page_count += tam_registro;
		}
		else if ((++page_avalible) < PAGES){ // Se ainda tiver páginas vai para a próxima, caso substitui segundo política
			page_count = 0;
			
		}
		else{ // Vai para à política de substituição
			page_avalible = 0;	
		}
	}
	free(lei_aux);
}
