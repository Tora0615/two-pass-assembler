# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define ALPHABET 26
FILE *source_code,*opCode,*outputSP,*objectCode;


//相同開頭指令做成一個 Linked List  如 ADD 18 -> ADDF 58 -> ADDR 90 -> AND 40 
typedef struct opCodeUnit opCodeUnit;
typedef struct opCodeUnit{
	char read[8];
	char translate[3];
	struct opCodeUnit* next;
};
opCodeUnit **alphtable;

typedef struct symbleUnit symbleUnit;
typedef struct symbleUnit{
	char read[16];
	int address;
};

void setOpCode(opCodeUnit*,char*);
void pass1();
void getInst(char*,char*,char*,char*);
int stringX16ToInt(char*);
int stringX10ToInt(char*);
int commentLine(char*);
void push(char*);
opCodeUnit* getopCodeD(char*); 

void pass2();

int main(){
	
	char sourcefile[20] = "source.txt";
	char opCodefile[20] = "opcode.txt";
	
	if((opCode = fopen(opCodefile,"r")) == NULL){
		printf("Fail to open file %s !",opCodefile);
		exit(0);
	}
	
	// 初始化 ALPHABET 大小(26)的 List，作為每個字母的Linked List開頭 
	alphtable = (opCodeUnit**)malloc(sizeof(opCodeUnit*)*ALPHABET);
	int i;
	for(i = 0;i < ALPHABET;i++){
		*(alphtable+i) =  NULL;
	}
	
	char charBuf = NULL;
	char stringBuf[15];
	
	//store the data which are in opCodefile into alphtable
	while(charBuf != EOF){ 
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
	
	
	//第一階段 
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass1();
	fclose(source_code);
	
	
	//第二階段 
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
int Length;

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
	printf("Instruction and it's location counter:\n\n");
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	int i,j;
	if(!stricmp(opCodeBuf,"START")){
		startAD = stringX16ToInt(InputBuf);
		location = startAD;
		printf("%X\t%s\n",location,strBuf);
		strcpy(ProgramName,symbolBuf);
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
		
		printf("%X\t%s\n",location,strBuf);
		if(symbolBuf[0] != '\0'){
			push(symbolBuf);
		}
		
		if(!stricmp(opCodeBuf,"BYTE")){
			if(InputBuf[0] == 'X'){	
				location += 1;
			}
			else if(InputBuf[0] == 'C'){
				
				location += 3;
			} 
		}
		else if(!stricmp(opCodeBuf,"WORD")){
			location += 3;
		}
		else if(!stricmp(opCodeBuf,"RESB")){
			int num = stringX10ToInt(InputBuf);
			location += num;
		}
		else if(!stricmp(opCodeBuf,"RESW")){
			int num = stringX10ToInt(InputBuf);
			location += (3*num);
		}
		else{
			opCodeUnit* point = getopCodeD(opCodeBuf);
			if(point == NULL){
				printf("can't find %s in the table.\n",opCodeBuf);
				exit(0);
			}
			location += 3;
		}
		
		do{
			fscanf(source_code,"%[^\n]",strBuf);
			charBuf = fgetc(source_code);
		}while(commentLine(strBuf));
		getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	}
	printf("\t%s\n",strBuf);
	Length = location-startAD;
	//printf("%X\n",Length);
	printf("\n\n\n");
	
	
	//印出 symbol table
	printf("symbol table:\n\n");
	for(i = 0;i < TOP;i++){
		printf("%s\t%x\n",(symbleTable+i)->read,(symbleTable+i)->address);
	}
	printf("\n\n\n");
}

void push(char* symbolBuf){
	
	int i;
	for(i = 0;i < TOP;i++){
		if(!stricmp((symbleTable+i)->read,symbolBuf)){
			printf("Varible %s is already exist.\n",symbolBuf);
			exit(0);
		}
	}
	
	if(TOP == STSize){
		STSize *= 2;
		symbleUnit* temp = (symbleUnit*)malloc(STSize*sizeof(symbleUnit));
		for(i = 0;i < TOP;i++){
			strcpy((temp+i)->read,(symbleTable+i)->read);
			(temp+i)->address = (symbleTable+i)->address;
		}
		symbleTable = temp;
	}
	
	strcpy((symbleTable+TOP)->read,symbolBuf);
	(symbleTable+TOP)->address = location;
	TOP++;
}

//註解相關 
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
	
	
	for(;;i++){
		if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')||strBuf[i]=='\0'){
			break;
		}
	}
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0'&&(strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0');i++,j++){
		InputBuf[j] = strBuf[i];
	}
	InputBuf[j] = '\0';	
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

int stringX10ToInt(char* InputBuf){
	int num = 0;
	int count = strlen(InputBuf) - 1;
	int i,j;
	for(i = 0;InputBuf[i] != '\0';i++,count--){
		int numBuf = 0;
		numBuf = InputBuf[i] - '0';
		for(j = 0;j < count;j++){
			numBuf *= 10;
		}
		num += numBuf;	
	}
	return num;
}

opCodeUnit* getopCodeD(char* opCodeBuf){
	opCodeUnit* point;
	if(opCodeBuf[0] >= 'A'&&opCodeBuf[0] <= 'Z'){
		point = *(alphtable + opCodeBuf[0]-'A');
		while(point != NULL){
			if(!stricmp(point->read,opCodeBuf)){
				break;
			}
			point = point->next;
		}
				if(point == NULL){
		}
	}
	else if(opCodeBuf[0] >= 'a'&&opCodeBuf[0] <= 'z'){
		point = *(alphtable + opCodeBuf[0]-'a');
		while(point != NULL){
			if(!stricmp(point->read,opCodeBuf)){
				break;
			}
			point = point->next;
		}
	}
	else{
		printf("can't find %s in the table.\n",opCodeBuf);
		exit(0);
	}

	return point;
}


void pass2(){
	outputSP = fopen("source_program.txt","w");
	objectCode = fopen("objCode.txt","w");
	fprintf(objectCode,"H%s\t%06X%06X\n",ProgramName,startAD,Length);
	char strBuf[100];
	char charBuf;
	char symbolBuf[16];
	char opCodeBuf[8];
	char InputBuf[16];
	
	char opCTrans[4];
	opCodeUnit* point;
	int objcode = 0;
	int addLocate;
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));

	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);


	location = startAD;
	fprintf(outputSP,"%X\t%s\n",location,strBuf);

	
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	int now = location;
	fprintf(objectCode,"T%06X",location);
	while(stricmp(opCodeBuf,"END")){
		int prelo = location;
		point = getopCodeD(opCodeBuf);
		if (point == NULL){
			strcpy(opCTrans,"");
			if(!stricmp(opCodeBuf,"BYTE")){
				if(InputBuf[0] == 'X'){
					char bufX16[3];
					bufX16[0] = InputBuf[2];
					bufX16[1] = InputBuf[3];
					bufX16[2] = '\0';
					fprintf(outputSP,"%x\t%s\t%s\t%s\t\t%s%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX16ToInt(bufX16));
				}
				else if(InputBuf[0] == 'C'){
					char bufX16[4];
					bufX16[0] = InputBuf[2];
					bufX16[1] = InputBuf[3];
					bufX16[2] = InputBuf[4];
					bufX16[3] = '\0';
					fprintf(outputSP,"%x\t%s\t%s\t%s\t\t%s%02X%02X%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,bufX16[0],bufX16[1],bufX16[2]);
				} 
			}
			else if(!stricmp(opCodeBuf,"WORD")){
				fprintf(outputSP,"%x\t%s\t%s\t%s\t\t%s%06X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX16ToInt(InputBuf));
			}
			else if(!stricmp(opCodeBuf,"RESB")){
				fprintf(outputSP,"%x\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);
			}
			else if(!stricmp(opCodeBuf,"RESW")){
				fprintf(outputSP,"%x\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);
			}
			else{
				fprintf(outputSP,"%x\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
			}	
		}else{
			strcpy(opCTrans,point->translate);
			objcode = 0;
			addLocate = 0;
			int i;
			for(i = 0;i < TOP;i++){
				if(!stricmp((symbleTable+i)->read, InputBuf)){
					break;
				}
			}
			if(i != TOP){
				addLocate=(symbleTable+i)->address;
				fprintf(outputSP,"%x\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
			}
			else if(InputBuf[strlen(InputBuf)-2] == ','&&InputBuf[strlen(InputBuf)-1] == 'X'){
				fprintf(outputSP,"%x\t%s\t%s\t%s\t%s",location,symbolBuf,opCodeBuf,InputBuf,opCTrans);
				InputBuf[strlen(InputBuf)-2] = '\0';
				for(i = 0;i < TOP;i++){
					if(!stricmp((symbleTable+i)->read, InputBuf)){
						break;
					}
				}
				addLocate=(symbleTable+i)->address;
				addLocate += 32768;
				fprintf(outputSP,"%04X\n",addLocate);
			}
		}


		if(!stricmp(opCodeBuf,"BYTE")){
			if(InputBuf[0] == 'X'){	
				location += 1;
			}
			else if(InputBuf[0] == 'C'){
				
				location += 3;
			} 
		}
		else if(!stricmp(opCodeBuf,"WORD")){
			location += 3;
		}
		else if(!stricmp(opCodeBuf,"RESB")){
			int num = stringX10ToInt(InputBuf);
			location += num;
		}
		else if(!stricmp(opCodeBuf,"RESW")){
			int num = stringX10ToInt(InputBuf);
			location += (3*num);
		}
		else{
			opCodeUnit* point = getopCodeD(opCodeBuf);
			location += 3;
		}
		if(location - now > 31){
			fprintf(objectCode,"\nT%06X",location);
			now = location;
		}
		do{
			fscanf(source_code,"%[^\n]",strBuf);
			charBuf = fgetc(source_code);
		}while(commentLine(strBuf));
		getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	}
	fprintf(outputSP,"\t%s\n",strBuf);
	
	fclose(outputSP);
	fclose(objectCode);
}

