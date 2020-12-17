#include <stdio.h>
#include <string.h>

FILE *fp;

int print_result(){
//	FILE *fp;
	fp = fopen("/dev/storage", "r");
	if(fp==NULL){
		printf("Greska pri otvaranju fajla\n");
		return -1;
	}
	int b, c;
	fscanf(fp, "%d %d", &b, &c);
	fclose(fp);
	printf("Rezultat je %d, prenos je %d\n", b, c);
	return 0;
}

int operacija_nad_registrima(char regx, char regy, char op){
	if(op == '*') op = 'x';
	char buff[30];
	fp = fopen("/dev/storage", "w");
	sprintf(buff, "reg%c %c reg%c\n", regx, op, regy);
	fputs(buff, fp);
	fclose(fp);

	fp = fopen("/dev/storage", "r");
	int b, c;
	fscanf(fp, "%d %d", &b, &c);
	fclose(fp);

	fp = fopen("/dev/storage", "w");
	sprintf(buff, "reg%c=0x%x\n", regx, b);
	fputs(buff, fp);
	fclose(fp);
}

int main(){
	char buff[30];
	while(1){
		//print_result();
		printf("\nUnesite matematicku operaciju: ");
		scanf("%s", buff);
		if(!strcmp(buff, "exit")) return 0;
		int val[4];
		char op[3];

		int i;
		for(i=0;i<4;i++) val[i]=0;
		for(i=0;i<3;i++) op[i]=0;

		sscanf(buff, "%d%c%d%c%d%c%d", &val[0], &op[0], &val[1], &op[1], &val[2], &op[2], &val[3]);

		//for(i=0;i<4;i++) printf("%d", val[i]);
		for(i=0;i<4;i++){
			fp = fopen("/dev/storage", "w");
			sprintf(buff, "reg%c=0x%x\n", 'a'+i, val[i]);
			//printf("%s\n", buff);
			fputs(buff, fp);
			fclose(fp);
		}

		int op_count;

		if(op[0]!=0) op_count = 1;
		if(op[1]!=0) op_count = 2;
		if(op[2]!=0) op_count = 3;


		if(op_count == 1) operacija_nad_registrima('a', 'b', op[0]);
		else if(op_count == 2){

			if((op[0] == '*' || op[0] == '/')&&(op[1]!= '*' || op[1]!='/')){
				operacija_nad_registrima('a', 'b', op[0]);
				operacija_nad_registrima('a', 'c', op[1]);
			}else{
				operacija_nad_registrima('b', 'c', op[1]);
				operacija_nad_registrima('a', 'b', op[0]);
			}

		}else if(op_count == 3){
			if(op[0]=='*' || op[0] == '/'){
				operacija_nad_registrima('a', 'b', op[0]);
				if(op[1] == '*' || op[1] == '/'){
					operacija_nad_registrima('a', 'c', op[1]);
					operacija_nad_registrima('a', 'd', op[2]);
				}else if(op[2] == '*' || op[2] == '/'){
					operacija_nad_registrima('c', 'd', op[2]);
					operacija_nad_registrima('a', 'c', op[1]);
				}else{
					operacija_nad_registrima('a', 'c', op[1]);
					operacija_nad_registrima('a', 'd', op[2]);
				}
			}else if(op[1] == '*' || op[1] == '/'){
				operacija_nad_registrima('b', 'c', op[1]);
				if(op[2] == '*' || op[2] == '/'){
					operacija_nad_registrima('b', 'd', op[2]);
					operacija_nad_registrima('a', 'b', op[0]);
				}else{
					operacija_nad_registrima('a', 'b', op[0]);
					operacija_nad_registrima('a', 'd', op[2]);
				}
			}else if(op[2] == '*' || op[2] == '/'){
				operacija_nad_registrima('c', 'd', op[2]);
				operacija_nad_registrima('a', 'b', op[0]);
				operacija_nad_registrima('a', 'c', op[1]);
			}else{
				operacija_nad_registrima('a', 'b', op[0]);
				operacija_nad_registrima('a', 'c', op[1]);
				operacija_nad_registrima('a', 'd', op[2]);
			}
		}
		//operacija_nad_registrima('a', 'b', op[0]);
		print_result();

	}
	return 0;
}
