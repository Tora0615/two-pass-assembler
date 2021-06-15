# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define ALPHABET 26
# define contain 2000
FILE *source_code,*opCode,*output,*outputSP,*OBJ;


//  ��Ƥ��q�P���� 

//  source.txt
//
//  ��� :  
//  symbol	opCode	Input
//
//  FIRST	STL		RETADR
//  CLOOP	JSUB	RDREC
//  		LDA		LENGTH
//  		COMP	ZERO
//  		JEQ		ENDFIL


//  opcode.txt
//
//  ��� 
//  opcode  �����N�X
// 
//  ADD 	18
//  ADDF 	58
//  ADDR 	90
//  AND 	40
//  CLEAR 	B4
//  COMP 	28
//  COMPF 	88


//  alphtable 
//
//  A : ADD 18 -> ADDF 58 -> ...
//  B : 
//  C : CLEAR B4 -> COMP 28 -> ...
//  ��
//  ... 


//�ŧi opcode �p�椸�����c 
typedef struct opCodeUnit opCodeUnit;
struct opCodeUnit{
	char read[8];
	char translate[3];
	struct opCodeUnit* next;
};
opCodeUnit **alphtable;


//�ŧi symbleUnit �p�椸�����c 
typedef struct symbleUnit symbleUnit;
struct symbleUnit{
	char read[16];
	int address;
};


//��l�ŧi
void setOpCode(opCodeUnit*,char*);
void pass1();
void getInst(char*,char*,char*,char*);
int stringX16ToInt(char*);
int stringX10ToInt(char*);
int commentLine(char*);
void push(char*);
void pass2();
void obj();
void add(char*,int,int,int,char);
void deletes();
opCodeUnit* getopCodeD(char*); 

//------ main �Ψ쪺��� --------- 
//�� opCode ���ӽX�s�J�r������� 
void setOpCode(opCodeUnit* point, char* readin){  //readin ���� stringBuf (�@���) 
	point->next = NULL; //�U�@�ӥ����Vnull 
	int i;
	for(i = 0;readin[i] != ' ';i++){
		point->read[i] = readin[i];
	}
	point->read[i] = '\0';
	
	point->translate[0] = readin[++i];
	point->translate[1] = readin[++i];
	point->translate[2] = '\0';
}


int main(){
	
	char sourcefile[20] = "source.txt";
	char opCodefile[20] = "opcode.txt";
	
	//�}�ɤ��ˬd 
	if((opCode = fopen(opCodefile,"r")) == NULL){
		printf("Fail to open file %s !",opCodefile);
		exit(0);
	}
	
	
	//�ŧi26�Ӧr�������O��}�Y 
	alphtable = (opCodeUnit**)malloc(sizeof(opCodeUnit*)*ALPHABET);
	int i;
	for(i = 0;i < ALPHABET;i++){
		*(alphtable+i) =  NULL;
	}
	char charBuf = NULL;
	char stringBuf[15];  
	//Ū��opCodefile�������O�� (���}�Y alphtable ) 
	while(charBuf != EOF){ 
		fscanf(opCode,"%[^\n]",stringBuf);  //�@��Ū�촫��Ÿ��e�A�æs�bstringBuf (���o�� FIRST STL RETADR)
		int index = (stringBuf[0] >= 'a' && stringBuf[0] <= 'z')?stringBuf[0]-'a':stringBuf[0]-'A';	 //�Ϥ��j�p�g�æ^�� index	
		if(*(alphtable + index) == NULL){ //index���s�b 
			*(alphtable + index) = (opCodeUnit*)malloc(sizeof(opCodeUnit));
			opCodeUnit* point = *(alphtable + index);
			setOpCode(point,stringBuf); //opCode ���ӽX�s�J�r���� 
		}
		else{  //���g�s�L�Aindex�s�b 
			opCodeUnit* point = *(alphtable + index);
			while(point->next != NULL){  //���̥��ݦ�m 
				point = point->next;
			}
			point->next =  (opCodeUnit*)malloc(sizeof(opCodeUnit));
			point = point->next;
			setOpCode(point,stringBuf); //opCode ���ӽX�s�J�r���� 
		}
		charBuf = fgetc(opCode); //Ū�@�Ӧr���A�ت��O���F�⴫��Ÿ�(\n)�Y�� 
	}
	fclose(opCode);
	
	
	//sourcefile�}�ɨóB�zpass1���� 
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass1();
	fclose(source_code);
	
	
	//sourcefile�}�ɨóB�zpass2���� 
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass2();
	fclose(source_code);
   	
	system("PAUSE");
	return 0;
}



int STSize = 2, TOP = 0;
int location, startAD=0;
symbleUnit* symbleTable;
int Length;

void pass1(){
	char strBuf[100]; //symbol opCode Input (���t \n )
	char charBuf; // �Y�� \n �Ϊ� 
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
	int i;
	if(!stricmp(opCodeBuf,"START")){
		startAD = stringX16ToInt(InputBuf);
		location = startAD;
		printf("%X\t%s\n",location,strBuf);
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
	printf("\n\n\n");
	printf("symbol table:\n\n");
	for(i = 0;i < TOP;i++){
		printf("%s\t%x\n",(symbleTable+i)->read,(symbleTable+i)->address);
	}
	printf("\n\n\n");
}


// push pass1
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

// pass 1 & 2
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


// ��J strBuf (�u���@��) ��X�� symbolBuf�BopCodeBuf�BInputBuf �r�� 
void getInst(char* strBuf,char* symbolBuf,char* opCodeBuf,char* InputBuf){
	
	
	// ���U��ϰ�}�Y i �Ҧb��m
	// FIRST    STL    RETADR 
	// i
	// ���osymbolBuf 
	int i = 0,j; //j�C�����|�k�s  
	for(j = 0; strBuf[i] != '\t'&&strBuf[i] != ' '; i++,j++){ //����tab�B�Ů�A�~����� 
		symbolBuf[j] = strBuf[i];
	}
	symbolBuf[j] = '\0';   //'\0' �Ŧr���A�Ω�r�ꪺ����
	
	
	// ���U��ϰ�}�Y i �Ҧb��m
	// FIRST    STL    RETADR 
	//      i
	
	//���L�Ů�Btab�� 
	for(;;i++){  //�@�����UŪ 
		if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')){  //����Ū�즹�B����tab�]�����Ů�A�B��e��O tab or �Ů�
			break;
		}
	}
	
	
	// ���U��ϰ�}�Y i �Ҧb��m
	// FIRST    STL    RETADR 
	//          i
	
	// ���oopCodeBuf 
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0';i++,j++){ //����tab�B�Ů�B�r����ݡA�~����� 
		opCodeBuf[j] = strBuf[i];
	}
	opCodeBuf[j] = '\0';
	
	
	
	//���L�Ů�Btab�� 
	for(;;i++){
		if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')){
			break;
		}
	}
	
	
	// ���oInputBuf
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0'&&(strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0');i++,j++){ //����tab�B�Ů�B�r����ݡA�~�����  
		InputBuf[j] = strBuf[i];
	}
	InputBuf[j] = '\0';	
}

//�Q���i��r���� int�A�æ^�� 
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

//�Q�i��r���� int�A�æ^�� 
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







int rear = 0;

typedef struct link link;
struct link{
	char opCTrans[4];
	int Xlocation;
	int LOCAT;
	int add;
	char Cflag;
};
link* queue;



void add(char* opCTrans,int Xlocation,int LOCAT,int add,char Cflag){
	strcpy((queue+rear)->opCTrans,opCTrans);
	(queue+rear)->Xlocation = Xlocation;
	(queue+rear)->LOCAT = LOCAT;
	(queue+rear)->add = add;
	(queue+rear)->Cflag = Cflag;
	rear++;
}







char ProgramName[30];



void pass2(){
	outputSP = fopen("source_program.txt","w");
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

	strcpy(ProgramName,symbolBuf);
	location = startAD;
	fprintf(outputSP,"%X\t%s\n",location,strBuf);

	
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);

	queue = (link*)malloc(sizeof(link)*contain);

	while(stricmp(opCodeBuf,"END")){
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
					add(opCTrans,stringX16ToInt(bufX16),location,1,'B');
				}
				else if(InputBuf[0] == 'C'){
					char bufX16[4];
					bufX16[0] = InputBuf[2];
					bufX16[1] = InputBuf[3];
					bufX16[2] = InputBuf[4];
					bufX16[3] = '\0';
					fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%02X%02X%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,bufX16[0],bufX16[1],bufX16[2]);
					add(bufX16,0,location,3,'C');
				} 
			}
			else if(!stricmp(opCodeBuf,"WORD")){
				fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%06X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX16ToInt(InputBuf));
				add(opCTrans,stringX16ToInt(InputBuf),location,3,'W');
			}
			else if(!stricmp(opCodeBuf,"RESB")){
				fprintf(outputSP,"%X\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);
			}
			else if(!stricmp(opCodeBuf,"RESW")){
				fprintf(outputSP,"%X\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);
			}
			else{
				fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
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
				fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
			}
			else if(InputBuf[strlen(InputBuf)-2] == ','&&InputBuf[strlen(InputBuf)-1] == 'X'){
				fprintf(outputSP,"%X\t%s\t%s\t%s\t%s",location,symbolBuf,opCodeBuf,InputBuf,opCTrans);
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
			add(opCTrans,addLocate,location,3,'E'); //else +=3
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
			location += 3;
		}

		do{
			fscanf(source_code,"%[^\n]",strBuf);
			charBuf = fgetc(source_code);
		}while(commentLine(strBuf));
		getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	}
	fprintf(outputSP,"\t%s\n",strBuf);
	fclose(outputSP);
	obj();
}

//objcode �ͦ� (2) 
void obj(){
	OBJ = fopen("objCode.txt","w");
	fprintf(OBJ,"H%s\t%06X%06X\n",ProgramName,startAD,Length); //h part  
	int i,j;
	int sum = 0;
	int start = (queue+0)->LOCAT;
	int startindex = 0;
	int len = 0;
	for(i = 0;i < rear;i++){
		sum = (queue+i)->LOCAT - start;
		if(sum > 29){
			fprintf(OBJ,"T%06X %02X ",start,len);
			for(j = startindex;j<i;j++){
				switch ((queue+j)->Cflag){
					case 'C':fprintf(OBJ,"%02X%02X%02X ",(queue+j)->opCTrans[0],(queue+j)->opCTrans[1],(queue+j)->opCTrans[2]);
						break;
					case 'B':fprintf(OBJ,"%s%02X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
						break;
					case 'W':fprintf(OBJ,"%s%06X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
						break;
					case 'E':fprintf(OBJ,"%s%04X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
						break;
				}
			}
			fprintf(OBJ,"\n");
			start = (queue+i)->LOCAT;
			startindex = i;
			len = 0;
		}
		len += (queue+i)->add;
	}
	
	fprintf(OBJ,"T%06X %02X ",start,len);
	for(j = startindex;j<i;j++){
		switch ((queue+j)->Cflag){
			case 'C':fprintf(OBJ,"%02X%02X%02X",(queue+j)->opCTrans[0],(queue+j)->opCTrans[1],(queue+j)->opCTrans[2]);
				break;
			case 'B':fprintf(OBJ,"%s%02X",(queue+j)->opCTrans,(queue+j)->Xlocation);
				break;
			case 'W':fprintf(OBJ,"%s%06X",(queue+j)->opCTrans,(queue+j)->Xlocation);
				break;
			case 'E':fprintf(OBJ,"%s%04X",(queue+j)->opCTrans,(queue+j)->Xlocation);
			break;
		}
	}
	fprintf(OBJ,"\n"); //t part 
	fprintf(OBJ,"E%06X\n",startAD); //e part
}
