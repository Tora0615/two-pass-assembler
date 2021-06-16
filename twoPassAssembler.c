# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# define ALPHABET 26
# define contain 2000
FILE *source_code,*opCode,*output,*outputSP,*OBJ;


//  資料片段與說明 

//  source.txt
//
//  欄位 :  
//  symbol	opCode	Input
//
//  FIRST	STL		RETADR
//  CLOOP	JSUB	RDREC
//  		LDA		LENGTH
//  		COMP	ZERO
//  		JEQ		ENDFIL


//  opcode.txt
//
//  欄位 
//  opcode  對應代碼
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
//  ↓
//  ... 


//宣告 opcode 小單元的結構 
typedef struct opCodeUnit opCodeUnit;
struct opCodeUnit{
	char read[8];
	char translate[3];
	struct opCodeUnit* next;
};
opCodeUnit **alphtable;


//宣告 symble 小單元的結構 
typedef struct symbleUnit symbleUnit;
struct symbleUnit{
	char read[16];
	int address;
};


//初始宣告
void setOpCode(opCodeUnit*,char*);
void pass1();
void getInst(char*,char*,char*,char*);
int stringX16ToInt(char*);
int stringX10ToInt(char*);
int commentLine(char*);
void push(char*);
void pass2();
void genObjCode();
void genSymbolTable();
void genSourceProgram();
void genLocatedSourceProgram();
void add(char*,int,int,int,char);
void deletes();
opCodeUnit* getopCodeD(char*); 

//------ main 用到的函數 --------- 
//把 opCode 跟對照碼存入字母表的函數 
void setOpCode(opCodeUnit* point, char* readin){  //readin 接收 stringBuf (一整行) 
	point->next = NULL; //下一個先指向null 
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
	
	//開檔及檢查 
	if((opCode = fopen(opCodefile,"r")) == NULL){
		printf("Fail to open file %s !",opCodefile);
		exit(0);
	}
	
	
	//宣告26個字母的指令表開頭 
	alphtable = (opCodeUnit**)malloc(sizeof(opCodeUnit*)*ALPHABET);
	int i;
	for(i = 0;i < ALPHABET;i++){
		*(alphtable+i) =  NULL;
	}
	char charBuf = NULL;
	char stringBuf[15];  
	//讀取opCodefile完成指令表 (見開頭 alphtable ) 
	while(charBuf != EOF){ 
		fscanf(opCode,"%[^\n]",stringBuf);  //一次讀到換行符號前，並存在stringBuf (像這樣 FIRST STL RETADR)
		int index = (stringBuf[0] >= 'a' && stringBuf[0] <= 'z')?stringBuf[0]-'a':stringBuf[0]-'A';	 //區分大小寫並回傳 index	
		if(*(alphtable + index) == NULL){ //index不存在 
			*(alphtable + index) = (opCodeUnit*)malloc(sizeof(opCodeUnit));
			opCodeUnit* point = *(alphtable + index);
			setOpCode(point,stringBuf); //opCode 跟對照碼存入字母表 
		}
		else{  //曾經存過，index存在 
			opCodeUnit* point = *(alphtable + index);
			while(point->next != NULL){  //找到最末端位置 
				point = point->next;
			}
			point->next =  (opCodeUnit*)malloc(sizeof(opCodeUnit));
			point = point->next;
			setOpCode(point,stringBuf); //opCode 跟對照碼存入字母表 
		}
		charBuf = fgetc(opCode); //讀一個字元，目的是為了把換行符號(\n)吃掉 
	}
	fclose(opCode);
	
	
	//sourcefile開檔並處理pass1部分 
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass1();
	fclose(source_code);
	
	
	//sourcefile開檔並處理pass2部分 
	if((source_code = fopen(sourcefile,"r")) == NULL){
		printf("Fail to open file %s !",sourcefile);
		exit(0);
	}
	pass2();
	fclose(source_code);
   	
	return 0;
}



int STSize = 2, TOP = 0;
int location, startAD=0;
symbleUnit* symbleTable;
int Length;

void pass1(){
	genLocatedSourceProgram();
	genSymbolTable();
}


// pass 1
// 堆疊元素 push 
void push(char* symbolBuf){
	int i;
	for(i = 0;i < TOP;i++){
		if(!stricmp((symbleTable+i)->read,symbolBuf)){
			printf("Varible %s is already exist.\n",symbolBuf);
			exit(0);
		}
	}
	if(TOP == STSize){  //如果要push的位置等於原本宣告的上限 
		STSize *= 2;  //改為原本兩倍的大小 
		symbleUnit* temp = (symbleUnit*)malloc(STSize*sizeof(symbleUnit));  //malloc宣告
		for(i = 0;i < TOP;i++){  //copy到新的 
			strcpy((temp+i)->read,(symbleTable+i)->read);
			(temp+i)->address = (symbleTable+i)->address;
		}
		symbleTable = temp;  //新的取代舊的 
	}
	
	//實際上的push 
	strcpy((symbleTable+TOP)->read,symbolBuf);
	(symbleTable+TOP)->address = location;
	TOP++;
}

// 生成包含location的 SourceProgram
void genLocatedSourceProgram(){
	char strBuf[100]; //symbol opCode Input (不含 \n )
	char charBuf; // 吃掉 \n 用的 
	char symbolBuf[16];
	char opCodeBuf[8];
	char InputBuf[16];
	symbleTable = (symbleUnit*)malloc(STSize*sizeof(symbleUnit));
	
	// 一開始遇到註解就一直往下讀，直到不是註解的那行 
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	
	
	printf("Instruction and it's location counter:\n\n");
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf); //讀初始 
	int i;
	
	//不是開始那行(開始那行之後) 
	if(!stricmp(opCodeBuf,"START")){
		startAD = stringX16ToInt(InputBuf);
		location = startAD;
		printf("%X\t%s\n",location,strBuf);  //印該行位置+文字出來 
	}
	else{
		location = 0;
	}
	
	//一行一行一直讀，直到不是註解 
	do{
		fscanf(source_code,"%[^\n]",strBuf); //讀不含換行的一整句 
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf); //1->3
	
	
	//  功能：比較字符串s1和s2，但不分字母的大小寫。
    //  s1<s2，返回值<0
    //  s1=s2，返回值=0
    //  s1>s2，返回值>0
    //  while 非 0 都當True 
    
	//一直讀直到是END那行停止 
	while(stricmp(opCodeBuf,"END")){
		//印這行的位置跟內容 
		printf("%X\t%s\n",location,strBuf);
		
		//依照這行內容以及 OPcode，計算下行所在位置 
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
		
		//讀取 
		do{
			fscanf(source_code,"%[^\n]",strBuf);
			charBuf = fgetc(source_code);
		}while(commentLine(strBuf));
		getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	}
	
	//印END那行 
	printf("\t%s\n",strBuf);
	Length = location-startAD;
	printf("\n\n\n");
}

//生成 SymbolTable
void genSymbolTable(){
	int i;
	printf("symbol table:\n\n");
	for(i = 0;i < TOP;i++){
		printf("%s\t%x\n",(symbleTable+i)->read,(symbleTable+i)->address);
	}
	printf("\n\n\n");
}




// pass 1 & 2
// 檢查讀到的位置是否為註解 (;開頭)，是註解回傳 1 
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

// 輸入 strBuf (只有一行) 輸出成 symbolBuf、opCodeBuf、InputBuf 字串 
void getInst(char* strBuf,char* symbolBuf,char* opCodeBuf,char* InputBuf){
	
	
	// ↓下方區域開頭 i 所在位置
	// FIRST    STL    RETADR 
	// i
	// 取得symbolBuf 
	int i = 0,j; //j每次都會歸零  
	for(j = 0; strBuf[i] != '\t'&&strBuf[i] != ' '; i++,j++){ //不為tab、空格，繼續執行 
		symbolBuf[j] = strBuf[i];
	}
	symbolBuf[j] = '\0';   //'\0' 空字元，用於字串的結束
	
	
	// ↓下方區域開頭 i 所在位置
	// FIRST    STL    RETADR 
	//      i
	
	//略過空格、tab用 
	for(;;i++){  //一直往下讀 
		if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')){  //直到讀到此處不為tab也不為空格，且其前方是 tab or 空格
			break;
		}
	}
	
	
	// ↓下方區域開頭 i 所在位置
	// FIRST    STL    RETADR 
	//          i
	
	// 取得opCodeBuf 
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0';i++,j++){ //不為tab、空格、字串尾端，繼續執行 
		opCodeBuf[j] = strBuf[i];
	}
	opCodeBuf[j] = '\0';
	
	
	
	//略過空格、tab用 
	for(;strBuf[i] != '\0';i++){ //+++++SU
		if((strBuf[i] != '\t'&&strBuf[i] != ' ')&&(strBuf[i-1] == '\t'||strBuf[i-1] == ' ')){
			break;
		}
	}
	
	
	// 取得InputBuf
	for(j = 0;strBuf[i] != '\t'&&strBuf[i] != ' '&&strBuf[i] != '\0';i++,j++){ //不為tab、空格、字串尾端，繼續執行  
		InputBuf[j] = strBuf[i];
	}
	InputBuf[j] = '\0';	
}

//十六進位字串轉 int，並回傳 
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

//十進位字串轉 int，並回傳 
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

//去字母表搜尋此 opCode， 並回傳記憶體位置 
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
	else{  //若有錯誤opCode直接退出 
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
	genSourceProgram();
	printf("\n\n");
	genObjCode();
}

// pass 2 -  SourceProgram 生成
void genSourceProgram(){
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

	//略過一開始註解們 
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);

	strcpy(ProgramName,symbolBuf); //抓出 ProgramName
	
	//第一行 
	location = startAD;
	fprintf(outputSP,"%X\t%s\n",location,strBuf);
	printf("%X\t%s\n",location,strBuf);

	//略過註解們 
	do{
		fscanf(source_code,"%[^\n]",strBuf);
		charBuf = fgetc(source_code);
	}while(commentLine(strBuf));
	getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);

	//駐列初始化 
	queue = (link*)malloc(sizeof(link)*contain);

	//沒到END之前一直做 
	while(stricmp(opCodeBuf,"END")){
		// 找這個opCode的位置 
		point = getopCodeD(opCodeBuf);
		
		// opCode不存在 (位置=NULL) 
		if (point == NULL){
			strcpy(opCTrans,"");  //複製空的的對應碼到 opCTrans 
			if(!stricmp(opCodeBuf,"BYTE")){  //當opCodeBuf 等於 "BYTE" (stricmp 相等為 0) 
				//看開頭 (input like X'F1' or C'EOF') 
				if(InputBuf[0] == 'X'){  //X直接摳來用 
					char bufX16[3];
					bufX16[0] = InputBuf[2];
					bufX16[1] = InputBuf[3];
					bufX16[2] = '\0';
					fprintf(outputSP,"%x\t%s\t%s\t%s\t\t%s%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX16ToInt(bufX16));
					printf("%x\t%s\t%s\t%s\t\t%s%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX16ToInt(bufX16));
					add(opCTrans,stringX16ToInt(bufX16),location,1,'B');  //加入駐列 
				}
				else if(InputBuf[0] == 'C'){  //C 則是把字元的ascii 轉16進位 
					char bufX16[4];
					bufX16[0] = InputBuf[2];
					bufX16[1] = InputBuf[3];
					bufX16[2] = InputBuf[4];
					bufX16[3] = '\0';
					fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%02X%02X%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,bufX16[0],bufX16[1],bufX16[2]);
					printf("%X\t%s\t%s\t%s\t\t%s%02X%02X%02X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,bufX16[0],bufX16[1],bufX16[2]);
					add(bufX16,0,location,3,'C');  //加入駐列  
				} 
			}
			else if(!stricmp(opCodeBuf,"WORD")){  //WORD轉十六進位 
				fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%06X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX10ToInt(InputBuf));
				printf("%X\t%s\t%s\t%s\t\t%s%06X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,stringX10ToInt(InputBuf));
				add(opCTrans,stringX10ToInt(InputBuf),location,3,'W');  //加入駐列  
			}
			else if(!stricmp(opCodeBuf,"RESB")){  //啥都不用幹，印出來就好 
				fprintf(outputSP,"%X\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);
				printf("%X\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);   
			}
			else if(!stricmp(opCodeBuf,"RESW")){  //啥都不用幹，印出來就好  
				fprintf(outputSP,"%X\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);
				printf("%X\t%s\t%s\t%s\n",location,symbolBuf,opCodeBuf,InputBuf);   
			}
		}else{ //opCode存在 
			strcpy(opCTrans,point->translate); //複製 opcode的對應碼到 opCTrans
			objcode = 0;
			addLocate = 0;
			int i;
			for(i = 0;i < TOP;i++){
				if(!stricmp((symbleTable+i)->read, InputBuf)){
					break;
				}
			}
			
			//???? 
			if(i != TOP){
				addLocate=(symbleTable+i)->address;
				fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
				printf("%X\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
			}else if(InputBuf[strlen(InputBuf)-2] == ','&&InputBuf[strlen(InputBuf)-1] == 'X'){
				fprintf(outputSP,"%X\t%s\t%s\t%s\t%s",location,symbolBuf,opCodeBuf,InputBuf,opCTrans);
				printf("%X\t%s\t%s\t%s\t%s",location,symbolBuf,opCodeBuf,InputBuf,opCTrans);
				InputBuf[strlen(InputBuf)-2] = '\0';
				for(i = 0;i < TOP;i++){
					if(!stricmp((symbleTable+i)->read, InputBuf)){
						break;
					}
				}
				addLocate=(symbleTable+i)->address;
				addLocate += 32768;
				fprintf(outputSP,"%04X\n",addLocate);
				printf("%04X\n",addLocate);
			}else{
				fprintf(outputSP,"%X\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
				printf("%X\t%s\t%s\t%s\t\t%s%04X\n",location,symbolBuf,opCodeBuf,InputBuf,opCTrans,addLocate);
			}
			
			add(opCTrans,addLocate,location,3,'E'); //else +=3
		}

		
		//對照類型，看 location要加多少 
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
		
		

		//讀最後一行 
		do{
			fscanf(source_code,"%[^\n]",strBuf);
			charBuf = fgetc(source_code);
		}while(commentLine(strBuf));
		getInst(strBuf,symbolBuf,opCodeBuf,InputBuf);
	}
	fprintf(outputSP,"\t%s\n",strBuf);
	fclose(outputSP);
}



//pass 2 - objcode 生成 
void genObjCode(){
	OBJ = fopen("object_program.txt","w");
	
	// ohjectcode H part  
	fprintf(OBJ,"H%s\t%06X %06X\n",ProgramName,startAD,Length); 
	printf("H%s\t%06X %06X\n",ProgramName,startAD,Length); 
	
	// ohjectcode T part 
	int i,j;
	int sum = 0;
	int start = (queue+0)->LOCAT;
	int startindex = 0;
	int len = 0;
	for(i = 0;i < rear;i++){  //i每次+1是新讀一個駐列的元素。 rear是駐列尾端。 
		sum = (queue+i)->LOCAT - start; //駐列總長 (應該是字元?) 
		if(sum > 27){
			fprintf(OBJ,"T%06X %02X ",start,len);
			printf("T%06X %02X ",start,len);
			for(j = startindex;j<i;j++){  
				// 以 Cflag 類型判斷   //CASE代表??? 
				switch ((queue+j)->Cflag){
					case 'C' :  
						fprintf(OBJ,"%02X%02X%02X ", (queue+j)->opCTrans[0], (queue+j)->opCTrans[1], (queue+j)->opCTrans[2]);
						printf("%02X%02X%02X ", (queue+j)->opCTrans[0], (queue+j)->opCTrans[1], (queue+j)->opCTrans[2]);
						break;
					case 'B':
						fprintf(OBJ,"%s%02X ", (queue+j)->opCTrans, (queue+j)->Xlocation);
						printf("%s%02X ", (queue+j)->opCTrans, (queue+j)->Xlocation);
						break;
					case 'W':
						fprintf(OBJ,"%s%06X ", (queue+j)->opCTrans, (queue+j)->Xlocation);
						printf("%s%06X ", (queue+j)->opCTrans, (queue+j)->Xlocation);
						break;
					case 'E':
						fprintf(OBJ,"%s%04X ", (queue+j)->opCTrans, (queue+j)->Xlocation);
						printf("%s%04X ", (queue+j)->opCTrans, (queue+j)->Xlocation);
						break;
				}
			}
			fprintf(OBJ,"\n");
			printf("\n");
			start = (queue+i)->LOCAT;
			startindex = i;
			len = 0;
		}
		len += (queue+i)->add;
	}
	
	fprintf(OBJ,"T%06X %02X ",start,len);
	printf("T%06X %02X ",start,len);
	for(j = startindex;j<i;j++){
		switch ((queue+j)->Cflag){
			case 'C' :
				fprintf(OBJ,"%02X%02X%02X ",(queue+j)->opCTrans[0],(queue+j)->opCTrans[1],(queue+j)->opCTrans[2]);
				printf("%02X%02X%02X ",(queue+j)->opCTrans[0],(queue+j)->opCTrans[1],(queue+j)->opCTrans[2]);
				break;
			case 'B':
				fprintf(OBJ,"%s%02X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
				printf("%s%02X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
				break;
			case 'W':
				fprintf(OBJ,"%s%06X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
				printf("%s%06X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
				break;
			case 'E':
				fprintf(OBJ,"%s%04X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
				printf("%s%04X ",(queue+j)->opCTrans,(queue+j)->Xlocation);
			break;
		}
	}
	fprintf(OBJ,"\n"); 
	printf("\n"); 
	
	// ohjectcode E part
	fprintf(OBJ,"E%06X\n",startAD); 
	printf("E%06X\n",startAD); 
}
