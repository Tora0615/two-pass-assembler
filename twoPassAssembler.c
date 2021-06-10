# include <stdio.h>
# include <stdlib.h>
FILE *source_code,*opCode,*output;

struct opCodeTable{
	
	
	
};

int main(){
	
	char sourcefile[20] = "source.txt";
	char opCodefile[20] = "opcode.txt";
	
	if((opCode = fopen(opCodefile,"r")) == NULL){
		printf("Fail to open file %s !",opCodefile);
		exit(0);
	}
	fclose(opCode);
	
	
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	fclose(source_code);
	
	system("PAUSE");
	return 0;
}
