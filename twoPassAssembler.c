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
}*symbleTable;

void setOpCode(opCodeUnit*,char*);
void pass1();
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
	
	for(i = 0;i < 26;i++){ //show the alphtable
		if(*(alphtable + i) != NULL){
			opCodeUnit* point = *(alphtable + i);
			while(point != NULL){
				printf("%s %s\n",point->read,point->translate);
				point = point->next;
			}
		}
	}
	
	
	
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

int location = 0,startAD = 0;

void pass1(){
	char strBuf[100];
	char charBuf;
	char opCodeBuf[8];
	fscanf(source_code,"%[^\n]",strBuf);
	charBuf = fgetc(source_code);
	int i,j;
	for(i = 0;i < strlen(strBuf);i++){
		if((strBuf[i] == '\t'||strBuf[i] == ' ')&&(strBuf[i+1] != '\t'&&strBuf[i+1] != ' ')){
			break;
		}
	}
	
	for(j = 0,i+=1;i < 100;j++,i++){
		if((strBuf[i-1] != '\t'&&strBuf[i-1] != ' ')&&(strBuf[i] == '\t'||strBuf[i] == ' '||strBuf[i] == '\n')){
			break;
		}
		opCodeBuf[j] = strBuf[i];
	}
	opCodeBuf[j] = '\0';
	if(!stricmp(opCodeBuf,"START")){
		for(;i < strlen(strBuf);i++){
			if((strBuf[i] == '\t'||strBuf[i] == ' ')&&(strBuf[i+1] != '\t'&&strBuf[i+1] != ' ')){
				break;
			}
		}
		i++;
		int count = -1;
		for(j = i;strBuf[j] != '\0';j++){
			count++; 
		}
		
		for(;strBuf[i] != '\0';i++,count--){
			int numBuf = 0;
			if(strBuf[i] >= '0'&&strBuf[i] <= '9'){
				numBuf = strBuf[i] - '0';
			}
			else if(strBuf[i] >= 'A'&&strBuf[i] <= 'F'){
				numBuf = strBuf[i] - 'A' + 10;
			}
			for(j = 0;j < count;j++){
				numBuf *= 16;
			}
			location += numBuf;
			
		}
		startAD = location;
	}
	else{
		//pass
	}

}

void pass2(){
	
	
	
	
}

