# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define ALPHABET 26
FILE *source_code,*opCode,*output;


typedef struct opCodeUnit opCodeUnit;
typedef struct opCodeUnit{
	char read[8];
	char translate[3];
	struct opCodeUnit* next;
};

typedef struct symbleUnit symbleUnit;
typedef struct symbleUnit{
	char read[16];
	int address;
};

void setOpCode(opCodeUnit*,char*);
void pass1();
void getInst(char*,char*,char*,char*);
int stringX16ToInt(char*);
int commentLine(char*);
void pass2();

int main(){
	
	char sourcefile[20] = "source.txt";
	char opCodefile[20] = "opcode.txt";
	
	if((opCode = fopen(opCodefile,"r")) == NULL){
		printf("Fail to open file %s !",opCodefile);
		exit(0);
	}
	
	opCodeUnit **alphtable = (opCodeUnit**)malloc(sizeof(opCodeUnit*)*ALPHABET);
	int i;
	for(i = 0;i < ALPHABET;i++){
		*(alphtable+i) =  NULL;
	}
	
	char charBuf = NULL;
	char stringBuf[15];
	
	while(charBuf != EOF){ //store the data which are in opCodefile into alphtable
		fscanf(opCode,"%[^\n]",stringBuf);
		int index = (stringBuf[0] >= 'a' && stringBuf[0] <= 'z')?stringBuf[0]-'a':stringBuf[0]-'A';		
		if(*(alphtable + index) == NULL){
			*(alphtable + index) = (opCodeUnit*)malloc(sizeof(opCodeUnit));
			opCodeUnit* point = *(alphtable + index);
			setOpCode(point,stringBuf);
		}
		else{
			opCodeUnit* point = *(alphtable + index);

			while(point->next != NULL){
				point = point->next;
			}
			point->next =  (opCodeUnit*)malloc(sizeof(opCodeUnit));
			point = point->next;
			setOpCode(point,stringBuf);
		}
		charBuf = fgetc(opCode);
	}
	
	fclose(opCode);
	
	
	
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass1();
	
	
	fclose(source_code);
	
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass2();
	
	
	fclose(source_code);
	
	system("PAUSE");
	return 0;
}

void setOpCode(opCodeUnit* point,char* readin){
	point->next = NULL;
	int i;
	for(i = 0;readin[i] != ' ';i++){
		point->read[i] = readin[i];
	}
	point->read[i] = '\0';
	
	point->translate[0] = readin[++i];
	point->translate[1] = readin[++i];
	point->translate[2] = '\0';
}

int STSize = 2,TOP = 0;
symbleUnit* symbleTable;
int location,startAD;

char ProgramName[30];


void pass1(){
	char strBuf[100];
	char charBuf;
	char symbolBuf[16];
	char opCodeBuf[8];
	char InputBuf[16];
	symbleTable = (symbleUnit*)malloc(STSize*sizeof(symbleUnit));
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	int i,j;
	if(!stricmp(opCodeBuf,"START")){
		startAD = stringX16ToInt(InputBuf);
		location = startAD;
	}
	else{
		location = 0;
	}
	
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	while(stricmp(opCodeBuf,"END")){
		
		
		//printf("%s\n",strBuf);
		
		do{
			fscanf(source_code,"%[^\n]",strBuf);
			charBuf = fgetc(source_code);
		}while(commentLine(strBuf));
		getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	}
	

}

int commentLine(char* Line){
	int i;
	for(i = 0;i < strlen(Line);i++){
		if(Line[i] != ' '&&Line[i] != '\t')	
			break;
	}
	if(Line[i] == ';'){
		return 1;
	}
	else{
		return 0;
	}
}

void getInst(char* strBuf,char* symbolBuf,char* opCodeBuf,char* InputBuf){
	int i = 0,j;
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' ';i++,j++){
		symbolBuf[j] = strBuf[i];
	}
	symbolBuf[j] = '\0';
	
	for(;;i++){
		if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')){
			break;
		}
	}
	
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0';i++,j++){
		opCodeBuf[j] = strBuf[i];
	}
	opCodeBuf[j] = '\0';
	
	int commentflag = 0;
	for(j = i;j < strlen(strBuf);j++){
		if(strBuf[j] == ';'){
			commentflag = 1;
			break;
		}
		if(strBuf[j] == '\"'){
			for(;j < strlen(strBuf);j++){
				if(strBuf[j] == '\"'){
					break;
				}
			}
		}
		if(strBuf[j] == '\''){
			for(;j < strlen(strBuf);j++){
				if(strBuf[j] == '\''){
					break;
				}
			}
		}
	}
	
	if(i == strlen(strBuf)||commentflag == 1){
		InputBuf = "0000";
	}
	else{
		for(;;i++){
			if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')){
				break;
			}
		}
		for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0';i++,j++){
			InputBuf[j] = strBuf[i];
		}
		InputBuf[j] = '\0';
	}
	
}

int stringX16ToInt(char* InputBuf){
	int Plus = 0;
	int count = strlen(InputBuf) - 1;
	int i,j;
	for(i = 0;InputBuf[i] != '\0';i++,count--){
		int numBuf = 0;
		if(InputBuf[i] >= '0'&&InputBuf[i] <= '9'){
			numBuf = InputBuf[i] - '0';
		}
		else if(InputBuf[i] >= 'A'&&InputBuf[i] <= 'F'){
			numBuf = InputBuf[i] - 'A' + 10;
		}
		for(j = 0;j < count;j++){
			numBuf *= 16;
		}
		Plus += numBuf;	
	}
	return Plus;
}

void pass2(){
	
	
	
	
}

