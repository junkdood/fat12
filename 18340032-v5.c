/*文件名不支持中文及符号。*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef int bool; 
#define true 1 
#define false 0 
#define MAX_open_FILEs  10
#define MAX_active_FILEs  100
#pragma pack () /*取消指定对齐，恢复缺省对齐*/
 
typedef unsigned char u8;	//1字节
typedef unsigned short u16;	//2字节
typedef unsigned int u32;	//4字节
typedef unsigned long long u64; //8字节

 
u8 ramFDD144[2880][512]; 
 
bool showhidd = false;
char OEMName[9];
int  BytsPerSec;	//每扇区字节数
int  SecPerClus;	//每簇扇区数
int  RsvdSecCnt;	//Boot记录占用的扇区数
int  NumFATs;	//FAT表个数
int  RootEntCnt;	//根目录最大文件数
int  TotSec;
int  Media;
int  FATSz16;	//FAT扇区数
int  SecPerTrk; 
int  NumHeads;
int  HiddSec;
int  DrvNum;
int  Reserved1;
int  BootSig;
int  VolID;
char VolLab[12];
char FileSysType[9];


 
 
#pragma pack (1) /*指定按1字节对齐*/
 
//偏移11个字节
struct BPB {	
	u8   BS_jmpBOOT_1;
	u16  BS_jmpBOOT_2;
	char  BS_OEMName[8];
	u16  BPB_BytsPerSec;	//每扇区字节数
	u8   BPB_SecPerClus;	//每簇扇区数
	u16  BPB_RsvdSecCnt;	//Boot记录占用的扇区数
	u8   BPB_NumFATs;	//FAT表个数
	u16  BPB_RootEntCnt;	//根目录最大文件数
	u16  BPB_TotSec16;
	u8   BPB_Media;
	u16  BPB_FATSz16;	//FAT扇区数
	u16  BPB_SecPerTrk;
	u16  BPB_NumHeads;
	u32  BPB_HiddSec;
	u32  BPB_TotSec32;	//如果BPB_FATSz16为0，该值为FAT扇区数
	u8   BS_DrvNum;
	u8   BS_Reserved1;
	u8   BS_BootSig;
	u32  BS_VolID;
	char  BS_VolLab[11];
	char  BS_FileSysType[8];
};
//BPB至此结束，长度25字节
 
//根目录条目
typedef struct Entry {	
	char DIR_Name[12];
	u8   DIR_Attr;		//文件属性
	char reserved[10];
	u16  DIR_WrtTime;
	u16  DIR_WrtDate;
	u16  DIR_FstClus;	//开始簇号
	u32  DIR_FileSize;
}Entry;
//根目录条目结束，32字节

typedef struct FILE_open {
	bool empty;
    int posi;
    int read_or_write;
    int activef;
 }  FILE_open;
typedef struct ACTIVE_FILE {
	bool empty;
    Entry f_dir;
    char f_path[100];
    int share_counter;
    int *firstb;
} ACTIVE_FILE;
ACTIVE_FILE activefile[ MAX_active_FILEs];
 

 
 
bool loadfat12(FILE * fat12);
bool savefat12(FILE * fat12); 
void fillBPB(FILE * fat12 , struct BPB* bpb_ptr);	//载入BPB
void loadEntry(Entry* En,int Secnum, int offset );
void saveEntry(Entry* En,int Secnum, int offset );
void printEntry(Entry* En);
void printrootlist();
void printlist(int clusnum);
void showhidden();
void hidehidden();
void printtxt(int clusnum);
int  getFATValue(int num);
void writefat(int num,int value);	//读取num号FAT项所在的两个字节，并从这两个连续字节中取出FAT项的值，
unsigned int  toint(int Secnum , int begin, int len);
void saveint(int Secnum , int begin, int len,unsigned int in);
void CreatUser(int usernum);
void tree(int cutclusnum,int sub);
int findcdpath(char* path,int cutclusnum);
int findprintpath(char* path,int cutclusnum);
bool writetxt(char* path,int cutclusnum);
bool app(char* path,int cutclusnum);
bool cp(char* path,int cutclusnum,int s);
int findemptyclu();
void emptyclu(int clusnum);
bool makedir(char* path,int cutclusnum);
bool touch(char* path,int cutclusnum);
bool removee(char* path,int cutclusnum);
bool isemptydir(int clunum);
void gettxt(int clusnum,char *buff);

int main() {
	OEMName[8]='\0';
	VolLab[11]='\0';
	FileSysType[8]='\0';
	FILE* fat12;
	fat12 = fopen("18340032-djf.IMG","rb+");	//打开FAT12的映像文件
	
 	int i;
	struct BPB bpb;
	struct BPB* bpb_ptr = &bpb;
	//载入BPB
	fillBPB(fat12,bpb_ptr);
	//初始化各个全局变量
	/*for(i=0;i<8;i++){
		OEMName[i]=bpb_ptr->BS_OEMName[i];
	}
	printf("OEMName：%s\n",OEMName);
	BytsPerSec = bpb_ptr->BPB_BytsPerSec;
	printf("BytsPerSec：0x%03x\n",BytsPerSec);
	SecPerClus = bpb_ptr->BPB_SecPerClus;
	printf("SecPerClus：0x%02x\n",SecPerClus);
	RsvdSecCnt = bpb_ptr->BPB_RsvdSecCnt;
	printf("RsvdSecCnt：0x%02x\n",RsvdSecCnt);
	NumFATs = bpb_ptr->BPB_NumFATs;
	printf("NumFATs：0x%02x\n",NumFATs);
	RootEntCnt = bpb_ptr->BPB_RootEntCnt;
	printf("RootEntCnt：0x%02x\n",RootEntCnt);
	
	if (bpb_ptr->BPB_TotSec16 != 0) {
		TotSec=bpb_ptr->BPB_TotSec16;
	} else {
		TotSec = bpb_ptr->BPB_TotSec32;
	}
	printf("TotSec：0x%03x\n",TotSec);
	Media=bpb_ptr->BPB_Media;
	printf("Media：0x%02x\n",Media);
	FATSz16 = bpb_ptr->BPB_FATSz16;
	printf("FATSz16：0x%02x\n",FATSz16);
	SecPerTrk=bpb_ptr->BPB_SecPerTrk; 
	printf("SecPerTrk：0x%02x\n",SecPerTrk);
	NumHeads=bpb_ptr->BPB_NumHeads;
	printf("NumHeads：0x%02x\n",NumHeads);
	HiddSec=bpb_ptr->BPB_HiddSec;
	printf("HiddSec：0x%01x\n",HiddSec);
	DrvNum=bpb_ptr->BS_DrvNum;
	printf("DrvNum：0x%01x\n",DrvNum);
	Reserved1=bpb_ptr->BS_Reserved1;
	printf("Reserved1：0x%01x\n",Reserved1);
	BootSig=bpb_ptr->BS_BootSig;
	printf("BootSig：0x%02x\n",BootSig);
	VolID=bpb_ptr->BS_VolID;
	printf("VolID：0x%01x\n",VolID);
	for(i=0;i<11;i++){
		VolLab[i]=bpb_ptr->BS_VolLab[i];
	}
	printf("VolLab：%s\n",VolLab);
	for(i=0;i<8;i++){
		FileSysType[i]=bpb_ptr->BS_FileSysType[i];
	}
	printf("FileSysType：%s\n",FileSysType);*/
	if(bpb_ptr->BPB_BytsPerSec == 512 && bpb_ptr->BPB_SecPerClus == 1 &&(bpb_ptr->BPB_TotSec16==2880||bpb_ptr->BPB_TotSec32==2880)&&bpb_ptr->BS_FileSysType[0]=='F'&&bpb_ptr->BS_FileSysType[1]=='A'&&bpb_ptr->BS_FileSysType[2]=='T'&&bpb_ptr->BS_FileSysType[3]=='1'&&bpb_ptr->BS_FileSysType[4]=='2'&&bpb_ptr->BS_FileSysType[5]==' '&&bpb_ptr->BS_FileSysType[6]==' '&&bpb_ptr->BS_FileSysType[7]==' '){
		if(loadfat12(fat12)){
			printf("successfully Loaded!\n");
		}
		else{
			printf("Load failed!\n");
			fclose(fat12);
			system("pause");
			return 0;
		}
	}
	else{
		printf("Not A FAT12!\n");
		fclose(fat12);
		system("pause");
		return 0;
	}
	for(i=0;i<8;i++){
		OEMName[i]=ramFDD144[0][3+i];
	}
	printf("OEMName：%s\n",OEMName);
	BytsPerSec = toint(0 , 11, 2);
	printf("BytsPerSec：0x%03x\n",BytsPerSec);
	SecPerClus = toint(0, 13, 1);
	printf("SecPerClus：0x%02x\n",SecPerClus);
	RsvdSecCnt = toint(0 , 14, 2);
	printf("RsvdSecCnt：0x%02x\n",RsvdSecCnt);
	NumFATs = toint(0 , 16, 1);
	printf("NumFATs：0x%02x\n",NumFATs);
	RootEntCnt = toint(0 , 17, 2);
	printf("RootEntCnt：0x%02x\n",RootEntCnt);
	if (toint(0 , 19, 2) != 0) {
		TotSec=toint(0 , 19, 2);
	} else {
		TotSec = toint(0 , 32, 4);
	}
	printf("TotSec：0x%03x\n",TotSec);
	Media=toint(0, 21, 1);
	printf("Media：0x%02x\n",Media);
	FATSz16 = toint(0 , 22, 2);
	printf("FATSz16：0x%02x\n",FATSz16);
	SecPerTrk=toint(0 , 24, 2); 
	printf("SecPerTrk：0x%02x\n",SecPerTrk);
	NumHeads=toint(0, 26, 2);
	printf("NumHeads：0x%02x\n",NumHeads);
	HiddSec=toint(0 , 28, 4);
	printf("HiddSec：0x%01x\n",HiddSec);
	DrvNum=toint(0 , 36, 1);
	printf("DrvNum：0x%01x\n",DrvNum);
	Reserved1=toint(0 , 37, 1);
	printf("Reserved1：0x%01x\n",Reserved1);
	BootSig=toint(0 , 38, 1);
	printf("BootSig：0x%02x\n",BootSig);
	VolID=toint(0, 39, 4);
	printf("VolID：0x%01x\n",VolID);
	for(i=0;i<11;i++){
		VolLab[i]=ramFDD144[0][43+i];
	}
	printf("VolLab：%s\n",VolLab);
	for(i=0;i<8;i++){
		FileSysType[i]=ramFDD144[0][54+i];
	}
	printf("FileSysType：%s\n",FileSysType);
	
	//printrootlist();
	//printtxt(19);
	//writefat(2000,0x123);
	//writefat(2001,0xfff);
	//writefat(2002,0x789);
	//printf("%x\n",getFATValue(2000));
	//printf("%x\n",getFATValue(2001));
	//printf("%x\n",getFATValue(2002));
	for(i=0;i<MAX_active_FILEs;i++){
		activefile[i].empty=true;
	}
	CreatUser(1);
	char f[10];
	printf("是否保存回img文件？y保存，任意其他键不保存");
	scanf("%s",f);
	if(f[0]=='y'||f[0]=='Y'){
		if(savefat12(fat12)){
			printf("successfully saved!\n");
		}
		else{
			printf("Save failed!\n");
		}
	} 
	fclose(fat12);
	system("pause");
	return 0;
}
 
void CreatUser(int usernum){
	int i;
	printf("User %d is created\n",usernum);
	FILE_open filelist[MAX_open_FILEs]; 
	for(i=0;i<MAX_open_FILEs;i++){
		filelist[i].empty=true;
	}
	u8 t[100];
	u8 currentpath[100];
	currentpath[0]='/';
	currentpath[1]='\0';
	int cutclusnum=0;
	while(1){
		printf("A%s:",currentpath);
		scanf("%s",t);
		if(strcmp(t,"tree")==0){
			//printf("未实现\n");
			tree(cutclusnum,0);
		}
		else if(strcmp(t,"ls")==0){
			if(cutclusnum==0){
				printrootlist();
			}
			else{
				printlist(cutclusnum);
			}
		}
		else if(strcmp(t,"print")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				int flag=findprintpath(tt,0);
				if(flag == -1){
					printf("无效路径！\n");
				}
				else{
					printtxt(flag);
				}
				
			}
			else{
				char tt[100];
				for(i=0;i<strlen(t)+1;i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				int flag=findprintpath(tt,cutclusnum);
				//printf("%d",flag);
				if(flag == -1){
					printf("无效路径！\n");
				}
				else{
					printtxt(flag);
				}
			}
			//printf("未实现\n");
		}
		else if(strcmp(t,"cd")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if (strcmp(t,".")==0){
				
			}
			else if((t[0]=='.'&&t[1]=='.'&&t[2]=='/')||(t[0]=='.'&&t[1]=='/')){
				printf("无效路径！\n");
			}
			else if (strcmp(t,"/")==0){
				cutclusnum=0;
				currentpath[0]='/';
				currentpath[1]='\0';
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				int flag=findcdpath(tt,0);
				if(flag == -1){
					printf("无效路径！\n");
				}
				else{
					cutclusnum=flag;
					for(i=0;i<strlen(t)+1;i++){
						currentpath[i]=t[i];
					}
					int te=strlen(currentpath);
					currentpath[te]='/';
					currentpath[te+1]='\0';
					
				}
				
			}
			else{
				char tt[100];
				for(i=0;i<strlen(t)+1;i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				int flag=findcdpath(tt,cutclusnum);
				//printf("%d",flag);
				if(flag == -1){
					printf("无效路径！\n");
				}
				else{
					cutclusnum=flag;
					if(strcmp(t,"..")==0){
						for(i=strlen(currentpath)-1;currentpath[i-1]!='/';i--){
							//printf("%c",currentpath[i-1]);
						}
						currentpath[i]='\0';
					}
					else{
						int te=strlen(currentpath);
						for(i=te;i<strlen(t)+1+te;i++){
							currentpath[i]=t[i-te];
						}
						te=strlen(currentpath);
						currentpath[te]='/';
						currentpath[te+1]='\0';
					}
					
				}
			}
			//printf("未实现\n");
		}
		else if(strcmp(t,"exit")==0){
			bool b=true; 
			for(i=0;i<MAX_open_FILEs;i++){
				if(filelist[i].empty!=true){
					printf("还有文件未关闭！\n");
					b=false;
				}
			}
			if(b)break;
		}
		else if(strcmp(t,"showhidden")==0){
			showhidden();

		}
		else if(strcmp(t,"hidehidden")==0){
			hidehidden();
		}
		else if(strcmp(t,"mkdir")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if (strcmp(t,".")==0||strcmp(t,"..")==0){
				 printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				if(makedir(tt,0)){
					printf("成功！\n");
				}
				else{
					printf("失败！！\n");
				}
				
			}
			else{
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				if(makedir(tt,cutclusnum)){
					printf("成功！\n");
				}
				else{
					printf("失败！！\n");
				}
			}
			//printf("未实现\n");
		}
		else if(strcmp(t,"touch")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if (strcmp(t,".")==0||strcmp(t,"..")==0){
				 printf("无效名字！\n");
			}
			else if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				if(touch(tt,0)){
					printf("成功！\n");
				}
				else{
					printf("失败！！\n");
				}
				
			}
			else{
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				if(touch(tt,cutclusnum)){
					printf("成功！\n");
				}
				else{
					printf("失败！！\n");
				}
			}
			//printf("未实现\n");
		}
		else if(strcmp(t,"rm")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if (strcmp(t,".")==0||strcmp(t,"..")==0){
				 printf("无效名字！\n");
			}
			else if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				if(removee(tt,0)){
					printf("成功！\n");
				}
				else{
					printf("失败！！\n");
				}
				
			}
			else{
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				if(removee(tt,cutclusnum)){
					printf("成功！\n");
				}
				else{
					printf("失败！！\n");
				}
			}
			//printf("未实现！\n");
		}
		else if(strcmp(t,"write")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				int flag=writetxt(tt,0);
				if(!flag){
					printf("无效路径！\n");
				}
				else{
					
				}
				
			}
			
			else{
				char tt[100];
				for(i=0;i<strlen(t)+1;i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				int flag=writetxt(tt,cutclusnum);
				//printf("%d",flag);
				if(!flag){
					printf("无效路径！\n");
				}
				else{
					
				}
			}
			//printf("未实现！\n");
		}
		else if(strcmp(t,"cp")==0){
			u8 s[100];
			scanf("%s %s",t,s);
			//printf("%s",t);
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			for(i=0;i<strlen(s);i++){
				if(s[i]<='z'&&s[i]>='a')s[i]=s[i]-('a'-'A');
			}
			if(strlen(t)==0||strlen(s)==0){
				printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				int ss=findprintpath(s,0);
				if(ss == -1){
					printf("无效路径！\n");
				}
				else{
					int flag=cp(tt,0,ss);
					if(!flag){
						printf("无效路径！\n");
					}
					else{
						
					}
				}
				
				
			}
			
			else{
				char tt[100];
				for(i=0;i<strlen(t)+1;i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				int ss=findprintpath(s,cutclusnum);
				if(ss == -1){
					printf("无效路径！\n");
				}
				else{
					int flag=cp(tt,cutclusnum,ss);
					//printf("%d",flag);
					if(!flag){
						printf("无效路径！\n");
					}
					else{
						
					}
				}
				
			}
			//printf("未实现！\n");
		}
		
		else if(strcmp(t,"append")==0){
			gets(t); 
			//printf("%s",t);
			while(t[0]==' '){
				for(i=0;i<strlen(t);i++){
					t[i]=t[i+1];
				}
			}
			for(i=0;i<strlen(t);i++){
				if(t[i]<='z'&&t[i]>='a')t[i]=t[i]-('a'-'A');
			}
			if(strlen(t)==0){
				printf("无效名字！\n");
			}
			else if(t[0]=='/'){
				char tt[100];
				for(i=0;i<strlen(t);i++){
					tt[i]=t[i+1];
				}
				int flag=app(tt,0);
				if(!flag){
					printf("无效路径！\n");
				}
				else{
					
				}
				
			}
			
			else{
				char tt[100];
				for(i=0;i<strlen(t)+1;i++){
					tt[i]=t[i];
				}
				tt[i]='\0';
				int flag=app(tt,cutclusnum);
				//printf("%d",flag);
				if(!flag){
					printf("无效路径！\n");
				}
				else{
					
				}
			}
			//printf("未实现！\n");
		}
		else if(strcmp(t,"help")==0){
			printf("tree 输出目录树\n");
			printf("ls 输出当前文件夹的文件\n");
			printf("cd xxx/xxx/xxx 访问相对路径\n");
			printf("cd /xxx/xxx/xxx 前面多一杠表示绝对路径，其他操作同理\n");
			printf("print xxx/xxx/xxx 输出文件内容\n");
			printf("mkdir xxx/xxx/xxx 创建子目录\n");
			printf("touch xxx/xxx/xxx 创建空文件(不需要写后缀.txt，程序会自动帮你加上)\n");
			printf("write xxx/xxx/xxx 将原文件覆盖，写入新内容\n");
			printf("append xxx/xxx/xxx 在原文件后面追加内容\n");
			printf("cp xxx/xxx/xxx(A) xxx/xxx/xxx(B) 将B的内容加到A后面\n");
			printf("rm xxx/xxx/xxx 删除文件或空目录\n");
			printf("showhidden 显示隐藏文件\n");
			printf("hidehidden 不显示隐藏文件\n");
			printf("exit 退出\n");
		}
		else{
			printf("无效语法\n");
		}
	}
	
}
int findemptyclu(){
	int i;
	for(i=2;i<3072;i++){
		if(getFATValue(i)==0)return i;
	}
	return -1; 
}
bool makedir(char* path,int cutclusnum){
	char name[11];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		name[i]=path[0];
		
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		//printf("%c",name[i]);
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
		if(cutclusnum==0){
			int Secnum = 19;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0){
				Entry t;
				Entry* tt=&t;
				loadEntry(tt,Secnum, offset);
				if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
					//printf("a");
					int b=true;
					for(i=0;i<11;i++){
						if(name[i]!=tt->DIR_Name[i]){
							//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
							b=false;
							break;
						}
					}
					
					if(b&&tt->DIR_Attr==0x10){
						
						if (path[0]=='\0'){
							return (unsigned int)(tt->DIR_FstClus);
						}
						else{
							return makedir(path,(unsigned int)(tt->DIR_FstClus));
						}
						
					}
					else{
						 
					}
				}
				offset++;
				if(offset>=16){
					offset=0;
					Secnum++;
					if(Secnum>=19+14){
						break;
					}
				}
			}
		}
		else{
			int Secnum = cutclusnum+31;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0){
				//printf("a");
				Entry t;
				Entry* tt=&t;
				loadEntry(tt,Secnum, offset);
				if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
					//printf("a");
					int b=true;
					for(i=0;i<11;i++){
						if(name[i]!=tt->DIR_Name[i]){
							b=false;
							break;
						}
					}
					if(b&&tt->DIR_Attr==0x10){
						if (path[0]=='\0'){
							return (unsigned int)(tt->DIR_FstClus);
						}
						else{
							return makedir(path,(unsigned int)(tt->DIR_FstClus));
						}
						
					}
					else{
						 
					}
				}
				offset++;
				if(offset>=16){
					if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
						cutclusnum=getFATValue(cutclusnum);
						Secnum = cutclusnum+31;
						offset = 0;
					}
					else{
						break;
					}
				}
			}
		}
	}
	else{
		if(cutclusnum==0){
			int Secnum = 19;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0&&(u8)(ramFDD144[Secnum][offset*32])!=0xe5){
				for(i=0;i<11;i++){
					if(ramFDD144[Secnum][offset*32+i]!=name[i])break;
				}
				if(i==11)return false;
				offset++;
				if(offset>=16){
					offset=0;
					Secnum++;
					if(Secnum>=19+14){
						break;
					}
				}
			}
			Entry t;
			Entry* tt=&t;
			for(i=0;i<11;i++){
				tt->DIR_Name[i]=name[i];
			}
			tt->DIR_Attr=0x10;		//文件属性
			tt->reserved[10];
			//1980+(En->DIR_WrtDate>>9),(u16)(En->DIR_WrtDate<<7)>>12,(u16)(En->DIR_WrtDate<<11)>>11,En->DIR_WrtTime>>11,(u16)(En->DIR_WrtTime<<5)>>10
			
			time_t timep;
    		struct tm *p;
    		time (&timep);
    		p=gmtime(&timep);
    		tt->DIR_WrtTime=0;
			tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
			tt->DIR_WrtDate=0;
			tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
			tt->DIR_FstClus=findemptyclu();
			for(i=0;i<512;i++){
				ramFDD144[tt->DIR_FstClus+31][i]=0;
			}
			writefat(tt->DIR_FstClus,0xfff);	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,Secnum,offset);
			for(i=0;i<11;i++){
				if(i<1)tt->DIR_Name[i]='.';
				else tt->DIR_Name[i]=' ';
			}
			tt->DIR_Attr=0x10;		//文件属性
			tt->reserved[10];
			tt->DIR_FstClus;	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,tt->DIR_FstClus+31,0);
			
			u8 x =tt->DIR_FstClus;
			for(i=0;i<11;i++){
				if(i<2)tt->DIR_Name[i]='.';
				else tt->DIR_Name[i]=' ';
			}
			tt->DIR_Attr=0x10;		//文件属性
			tt->reserved[10];
			tt->DIR_FstClus=cutclusnum;	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,x+31,1);
			return true;
		}
		else{
			int Secnum = cutclusnum+31;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0&&(u8)(ramFDD144[Secnum][offset*32])!=0xe5){
				for(i=0;i<11;i++){
					if(ramFDD144[Secnum][offset*32+i]!=name[i])break;
				}
				if(i==11)return false;
				offset++;
				if(offset>=16){
					if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
						cutclusnum=getFATValue(cutclusnum);
						Secnum = cutclusnum+31;
						offset = 0;
					}
					else{
						break;
					}
				}
			}
			Entry t;
			Entry* tt=&t;
			for(i=0;i<11;i++){
				tt->DIR_Name[i]=name[i];
			}
			tt->DIR_Attr=0x10;		//文件属性
			tt->reserved[10];
			time_t timep;
    		struct tm *p;
    		time (&timep);
    		p=gmtime(&timep);
    		
    		tt->DIR_WrtTime=0;
			tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
			tt->DIR_WrtDate=0;
			tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
			tt->DIR_FstClus=findemptyclu();
			for(i=0;i<512;i++){
				ramFDD144[tt->DIR_FstClus+31][i]=0;
			}
			writefat(tt->DIR_FstClus,0xfff);	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,Secnum,offset);
			
			for(i=0;i<11;i++){
				if(i<1)tt->DIR_Name[i]='.';
				else tt->DIR_Name[i]=' ';
			}
			tt->DIR_Attr=0x10;		//文件属性
			tt->reserved[10];
			tt->DIR_FstClus;	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,tt->DIR_FstClus+31,0);
			u8 x =tt->DIR_FstClus;
			for(i=0;i<11;i++){
				if(i<2)tt->DIR_Name[i]='.';
				else tt->DIR_Name[i]=' ';
			}
			tt->DIR_Attr=0x10;		//文件属性
			tt->reserved[10];
			tt->DIR_FstClus=cutclusnum;	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,x+31,1);
			return true;
		}
	} 
	return false;
}

bool touch(char* path,int cutclusnum){
	char name[11];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		name[i]=path[0];
		
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		//printf("%c",name[i]);
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
		if(cutclusnum==0){
			int Secnum = 19;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0){
				Entry t;
				Entry* tt=&t;
				loadEntry(tt,Secnum, offset);
				if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
					//printf("a");
					int b=true;
					for(i=0;i<11;i++){
						if(name[i]!=tt->DIR_Name[i]){
							//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
							b=false;
							break;
						}
					}
					
					if(b&&tt->DIR_Attr==0x10){
						
						if (path[0]=='\0'){
							return (unsigned int)(tt->DIR_FstClus);
						}
						else{
							return touch(path,(unsigned int)(tt->DIR_FstClus));
						}
						
					}
					else{
						 
					}
				}
				offset++;
				if(offset>=16){
					offset=0;
					Secnum++;
					if(Secnum>=19+14){
						break;
					}
				}
			}
		}
		else{
			int Secnum = cutclusnum+31;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0){
				//printf("a");
				Entry t;
				Entry* tt=&t;
				loadEntry(tt,Secnum, offset);
				if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
					//printf("a");
					int b=true;
					for(i=0;i<11;i++){
						if(name[i]!=tt->DIR_Name[i]){
							b=false;
							break;
						}
					}
					if(b&&tt->DIR_Attr==0x10){
						if (path[0]=='\0'){
							return (unsigned int)(tt->DIR_FstClus);
						}
						else{
							return touch(path,(unsigned int)(tt->DIR_FstClus));
						}
						
					}
					else{
						 
					}
				}
				offset++;
				if(offset>=16){
					if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
						cutclusnum=getFATValue(cutclusnum);
						Secnum = cutclusnum+31;
						offset = 0;
					}
					else{
						break;
					}
				}
			}
		}
	}
	else{
		if(cutclusnum==0){
			int Secnum = 19;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0&&(u8)(ramFDD144[Secnum][offset*32])!=0xe5){
				for(i=0;i<11;i++){
					if(ramFDD144[Secnum][offset*32+i]!=name[i])break;
				}
				if(i==11)return false;
				offset++;
				if(offset>=16){
					offset=0;
					Secnum++;
					if(Secnum>=19+14){
						break;
					}
				}
			}
			Entry t;
			Entry* tt=&t;
			for(i=0;i<11;i++){
				tt->DIR_Name[i]=name[i];
			}
			tt->DIR_Name[8]='T';
			tt->DIR_Name[9]='X';
			tt->DIR_Name[10]='T';
			tt->DIR_Attr=0x20;		//文件属性
			tt->reserved[10];
			time_t timep;
    		struct tm *p;
    		time (&timep);
    		p=gmtime(&timep);
    		tt->DIR_WrtTime=0;
			tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
			tt->DIR_WrtDate=0;
			tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
			tt->DIR_FstClus=findemptyclu();
			for(i=0;i<512;i++){
				ramFDD144[tt->DIR_FstClus+31][i]=0;
			}
			ramFDD144[tt->DIR_FstClus+31][1]='\0';
			writefat(tt->DIR_FstClus,0xfff);	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,Secnum,offset);
			return true;
		}
		else{
			int Secnum = cutclusnum+31;
			int offset = 0;
			while (ramFDD144[Secnum][offset*32]!=0&&(u8)(ramFDD144[Secnum][offset*32])!=0xe5){
				for(i=0;i<11;i++){
					if(ramFDD144[Secnum][offset*32+i]!=name[i])break;
				}
				if(i==11)return false;
				offset++;
				if(offset>=16){
					if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
						cutclusnum=getFATValue(cutclusnum);
						Secnum = cutclusnum+31;
						offset = 0;
					}
					else{
						break;
					}
				}
			}
			Entry t;
			Entry* tt=&t;
			for(i=0;i<11;i++){
				tt->DIR_Name[i]=name[i];
			}
			tt->DIR_Name[8]='T';
			tt->DIR_Name[9]='X';
			tt->DIR_Name[10]='T';
			tt->DIR_Attr=0x20;		//文件属性
			tt->reserved[10];
			//1980+(En->DIR_WrtDate>>9),(u16)(En->DIR_WrtDate<<7)>>12,(u16)(En->DIR_WrtDate<<11)>>11,En->DIR_WrtTime>>11,(u16)(En->DIR_WrtTime<<5)>>10
			time_t timep;
    		struct tm *p;
    		time (&timep);
    		p=gmtime(&timep);
    		tt->DIR_WrtTime=0;
			tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
			tt->DIR_WrtDate=0;
			tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
			tt->DIR_FstClus=findemptyclu();
			for(i=0;i<512;i++){
				ramFDD144[tt->DIR_FstClus+31][i]=0;
			}
			ramFDD144[tt->DIR_FstClus+31][1]='\0';
			writefat(tt->DIR_FstClus,0xfff);	//开始簇号
			tt->DIR_FileSize=0;
			saveEntry(tt,Secnum,offset);
			return true;
		}
	} 
	return false;
}

void saveint(int Secnum , int begin, int len,unsigned int in){
	if(len > 4){
		printf("saveint too long!");
	}
	int i;
	for(i=0;i<len;i++){
		ramFDD144[Secnum][begin+i]=in;
		in>>=8;
	}
}
unsigned int  toint(int Secnum , int begin, int len){
	if(len > 4){
		printf("toint too long!");
		return 0;
	}
	unsigned int t = 0;
	int i;
	for(i=len - 1;i>=0;i--){
		t<<=8;
		t+=ramFDD144[Secnum][begin+i];
		
	}
	return t;
}

bool savefat12(FILE * fat12){
	int i;
	int j;
	for(i=0;i<9;i++){
		for(j=0;j<512;j++){
			ramFDD144[RsvdSecCnt+i+9][j]=ramFDD144[RsvdSecCnt+i][j];
		}
	}
	int check;
	//BPB从偏移11个字节处开始
	check = fseek(fat12,0,SEEK_SET);
	if (check == -1) {
		printf("fseek in writefat12 failed!\n");
		return false;
	}
		
 
	//BPB长度为52字节
	check = fwrite(ramFDD144,1,2880*512,fat12);
	if (check != 2880*512){
		printf("fwrite in writefat12 failed!\n");
		return false;
	}
	return true;
}
 
bool loadfat12(FILE * fat12){
	int check;
	//BPB从偏移11个字节处开始
	check = fseek(fat12,0,SEEK_SET);
	if (check == -1) {
		printf("fseek in loadfat12 failed!\n");
		return false;
	}
		
 
	//BPB长度为52字节
	check = fread(ramFDD144,1,2880*512,fat12);
	if (check != 2880*512){
		printf("fread in loadfat12 failed!\n");
		return false;
	}
	return true;
}
void fillBPB(FILE* fat12 , struct BPB* bpb_ptr) {
	int check;
 
	//BPB从偏移11个字节处开始
	check = fseek(fat12,0,SEEK_SET);
	if (check == -1) 
		printf("fseek in fillBPB failed!\n");
 
	//BPB长度为52字节
	check = fread(bpb_ptr,1,62,fat12);
	if (check != 62)
		printf("fread in fillBPB failed!\n");
}

void writefat(int num,int value){
	int fatBase = RsvdSecCnt;
	//FAT项的偏移字节
	int fatPos =num*3/2 ;
	//奇偶FAT项处理方式不同，分类进行处理，从0号FAT项开始
	int type = 0;
	if (num % 2 == 0) {
		type = 0;
	} else {
		type = 1;
	}
	//先读出FAT项所在的两个字节
	//u16 bytes=(u16)((int)(*(ramFDD144+fatBase*512+fatPos+1))<<8)+(int)(*(ramFDD144+fatBase*512+fatPos+1));
	u16 bytes=(u16)(ramFDD144[fatBase][fatPos+1]<<8)+ramFDD144[fatBase][fatPos];
	//u16为short，结合存储的小尾顺序和FAT项结构可以得到
	//type为0的话，取byte2的低4位和byte1构成的值，type为1的话，取byte2和byte1的高4位构成的值
	if (type == 0) {
		ramFDD144[fatBase][fatPos]=value;
		ramFDD144[fatBase][fatPos+1]>>=4;
		ramFDD144[fatBase][fatPos+1]<<=4;
		ramFDD144[fatBase][fatPos+1]+=(value>>8);
	} else {
		ramFDD144[fatBase][fatPos]<<=4;
		ramFDD144[fatBase][fatPos]>>=4;
		ramFDD144[fatBase][fatPos]+=value<<4;
		ramFDD144[fatBase][fatPos+1]=(value>>4);
	}
}
int  getFATValue(int num) {
	//FAT1的偏移字节
	int fatBase = RsvdSecCnt;
	//FAT项的偏移字节
	int fatPos =num*3/2 ;
	//奇偶FAT项处理方式不同，分类进行处理，从0号FAT项开始
	int type = 0;
	if (num % 2 == 0) {
		type = 0;
	} else {
		type = 1;
	}
	//先读出FAT项所在的两个字节
	//u16 bytes=(u16)((int)(*(ramFDD144+fatBase*512+fatPos+1))<<8)+(int)(*(ramFDD144+fatBase*512+fatPos+1));
	u16 bytes=(u16)(ramFDD144[fatBase][fatPos+1]<<8)+ramFDD144[fatBase][fatPos];
	//u16为short，结合存储的小尾顺序和FAT项结构可以得到
	//type为0的话，取byte2的低4位和byte1构成的值，type为1的话，取byte2和byte1的高4位构成的值
	if (type == 0) {
		return ((u16)(bytes<<4))>>4;
	} else {
		return bytes>>4;
	}
}
void saveEntry(Entry* En,int Secnum, int offset ){
	int i;
	for(i=0;i<11;i++){
		ramFDD144[Secnum][offset*32+i]=En->DIR_Name[i];
	}
	saveint(Secnum,offset*32+11,1,En->DIR_Attr);
	for(i=0;i<10;i++){
		ramFDD144[Secnum][offset*32+12+i]=En->reserved[i];
	}
	saveint(Secnum,offset*32+22,2,En->DIR_WrtTime);
	saveint(Secnum,offset*32+24,2,En->DIR_WrtDate);
	saveint(Secnum,offset*32+26,2,En->DIR_FstClus);
	saveint(Secnum,offset*32+28,4,En->DIR_FileSize);
	
}
void loadEntry(Entry* En,int Secnum, int offset ){
	int i;
	for(i=0;i<11;i++){
		En->DIR_Name[i]=ramFDD144[Secnum][offset*32+i];
	}
	En->DIR_Attr=toint(Secnum,offset*32+11,1);
	for(i=0;i<10;i++){
		En->reserved[i]=ramFDD144[Secnum][offset*32+12+i];
	}
	En->DIR_WrtTime=toint(Secnum,offset*32+22,2);
	En->DIR_WrtDate=toint(Secnum,offset*32+24,2);
	En->DIR_FstClus=toint(Secnum,offset*32+26,2);
	En->DIR_FileSize=toint(Secnum,offset*32+28,4);
}
void printEntry(Entry* En){
	if((En->DIR_Attr==0x27&&!showhidd)||(u8)(En->DIR_Name[0])==0xe5){
		return;
	}
	else{
		int i;
		char DIR_Name[12];
		DIR_Name[11]='\0';
		for(i=0;i<11;i++){
			DIR_Name[i]=En->DIR_Name[i];
		}
		printf("文件名：%s 文件属性：0x%02x 最后更改时间： %02d-%02d-%02d %02d:%02d 文件大小：%d字节\n",DIR_Name,En->DIR_Attr,1980+(En->DIR_WrtDate>>9),(u16)(En->DIR_WrtDate<<7)>>12,(u16)(En->DIR_WrtDate<<11)>>11,En->DIR_WrtTime>>11,(u16)(En->DIR_WrtTime<<5)>>10,En->DIR_FileSize);
		//printf("首簇号：%d\n",En->DIR_FstClus);
	}
}
void printrootlist(){
	//showhidden();
	int Secnum = 19;
	int offset = 0;
	while (ramFDD144[Secnum][offset*32]!=0){
		Entry t;
		Entry* tt=&t;
		loadEntry(tt,Secnum, offset);
		printEntry(tt);
		offset++;
		if(offset>=16){
			offset=0;
			Secnum++;
			if(Secnum>=19+14){
				break;
			}
		}
	}
	//hidehidden();
	return;
}
void printlist(int clusnum){
	//showhidden();
	int Secnum = clusnum+31;
	int offset = 0;
	while (ramFDD144[Secnum][offset*32]!=0){
		Entry t;
		Entry* tt=&t;
		loadEntry(tt,Secnum, offset);
		printEntry(tt);
		offset++;
		if(offset>=16&&getFATValue(clusnum)!=0xfff&&getFATValue(clusnum)!=0xff0){
			printlist(getFATValue(clusnum));
			break;
		}
	}
	//hidehidden();
	return;
}
void showhidden(){
	showhidd = true;
}
void hidehidden(){
	showhidd = false;
}
void printtxt(int clusnum){
	int i;
	for(i=0;i<512;i++){
		if(ramFDD144[clusnum+31][i] == '\0')break;
		printf("%c",ramFDD144[clusnum+31][i]);
	}
	
	if(getFATValue(clusnum)==0xfff){
		printf("\n");
		return;
	}
	else if(getFATValue(clusnum)==0xff0){
		printf("坏簇!\n");
	}
	else{
		printtxt(getFATValue(clusnum));
	}
}
void gettxt(int clusnum,char *buff){
	int i;
	int r=0;
	while(1){
		for(i=0;i<512;i++){
			buff[r]=ramFDD144[clusnum+31][i];
			r++;
		}
		if(getFATValue(clusnum)==0xfff){
			break;
		}
		else if(getFATValue(clusnum)==0xff0){
			printf("坏簇!\n");
			break;
		}
		else{
			clusnum=getFATValue(clusnum);
		}
	}
}

void tree(int cutclusnum,int sub){
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int i;
				char DIR_Name[12];
				DIR_Name[11]='\0';
				for(i=0;i<11;i++){
					DIR_Name[i]=tt->DIR_Name[i];
				}
				if(strcmp(DIR_Name,"..         ")!=0&&strcmp(DIR_Name,".          ")!=0){
					//printf("%d\n",sub);
					for(i=0;i<sub-1;i++)printf("|     ");
					if(sub>0)printf("|-----");
					printf("%s\n",DIR_Name);
				}
				
				if(tt->DIR_Attr==0x10&&strcmp(DIR_Name,"..         ")!=0&&strcmp(DIR_Name,".          ")!=0){
					//printf("a");
					tree(tt->DIR_FstClus,sub+1);
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int i;
				char DIR_Name[12];
				DIR_Name[11]='\0';
				for(i=0;i<11;i++){
					DIR_Name[i]=tt->DIR_Name[i];
				}
				
				
				
				if(strcmp(DIR_Name,"..         ")!=0&&strcmp(DIR_Name,".          ")!=0){
					//printf("%d",sub);
					for(i=0;i<sub-1;i++)printf("|     ");
					if(sub>0)printf("|-----");
					printf("%s\n",DIR_Name);
				}
				if(tt->DIR_Attr==0x10&&strcmp(DIR_Name,"..         ")!=0&&strcmp(DIR_Name,".          ")!=0){
					//printf("b%s",DIR_Name);
					tree(tt->DIR_FstClus,sub+1);
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0)tree(getFATValue(cutclusnum),sub);
				break;
			}
		}
	}
}
int findcdpath(char* path,int cutclusnum){
	
	char name[11];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		name[i]=path[0];
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		//printf("%c",name[i]);
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
	}
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
						b=false;
						break;
					}
				}
				
				if(b&&tt->DIR_Attr==0x10){
					
					if (path[0]=='\0'){
						return (unsigned int)(tt->DIR_FstClus);
					}
					else{
						return findcdpath(path,(unsigned int)(tt->DIR_FstClus));
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						b=false;
						break;
					}
				}
				if(b&&tt->DIR_Attr==0x10){
					if (path[0]=='\0'){
						return (unsigned int)(tt->DIR_FstClus);
					}
					else{
						return findcdpath(path,(unsigned int)(tt->DIR_FstClus));
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
					cutclusnum=getFATValue(cutclusnum);
					Secnum = cutclusnum+31;
					offset = 0;
				}
				else{
					break;
				}
			}
		}
	}
	return -1; 
}
int findprintpath(char* path,int cutclusnum){
	char name[12];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		if(i>11)return -1; 
		name[i]=path[0];
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
	}
	else{
		for(i=0;i<11;i++){
			if(name[i]=='.'){
				name[8]=name[i+1];
				name[9]=name[i+2];
				name[10]=name[i+3];
				for(i;i<8;i++){
					name[i]=' ';
				}
				break;
			}
		}
		
	}
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
						b=false;
						break;
					}
				}
				
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return findprintpath(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						return (unsigned int)(tt->DIR_FstClus);
						
					}
					else{
						return -1;
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						b=false;
						break;
					}
				}
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return findprintpath(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						return (unsigned int)(tt->DIR_FstClus);
						
					}
					else{
						return -1;
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
					cutclusnum=getFATValue(cutclusnum);
					Secnum = cutclusnum+31;
					offset = 0;
				}
				else{
					break;
				}
			}
		}
	}
	return -1; 
}
void emptyclu(int clusnum){
	if(clusnum!=0xfff&&clusnum!=0xff0)emptyclu(getFATValue(clusnum));
	writefat(clusnum,0);
}
bool removee(char* path,int cutclusnum){
	char name[12];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		if(i>11)return -1; 
		name[i]=path[0];
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
	}
	else{
		for(i=0;i<11;i++){
			if(name[i]=='.'){
				name[8]=name[i+1];
				name[9]=name[i+2];
				name[10]=name[i+3];
				for(i;i<8;i++){
					name[i]=' ';
				}
				break;
			}
		}
		
	}
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
						b=false;
						break;
					}
				}
				
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return removee(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						emptyclu(tt->DIR_FstClus);
						ramFDD144[Secnum][offset*32]=0xe5;
						return 1;
					}
					else if(tt->DIR_Attr==0x10&&path[0]=='\0'){
						if (isemptydir(tt->DIR_FstClus)){
							emptyclu(tt->DIR_FstClus);
							ramFDD144[Secnum][offset*32]=0xe5;
							return 1;
						}
						else{
							printf("子目录非空！\n"); 
							return 0;
						}
						
					}
					else{
						return 0;
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						b=false;
						break;
					}
				}
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return removee(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						emptyclu(tt->DIR_FstClus);
						ramFDD144[Secnum][offset*32]=0xe5;
						return 1;
						
					}
					else if(tt->DIR_Attr==0x10&&path[0]=='\0'){
						if (isemptydir(tt->DIR_FstClus)){
							emptyclu(tt->DIR_FstClus);
							ramFDD144[Secnum][offset*32]=0xe5;
							return 1;
						}
						else{
							printf("子目录非空！\n"); 
							return 0;
						}
						
					}
					else{
						return 0;
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
					cutclusnum=getFATValue(cutclusnum);
					Secnum = cutclusnum+31;
					offset = 0;
				}
				else{
					break;
				}
			}
		}
	}
	return 0; 
}
bool isemptydir(int clunum){
	if (clunum==0)return 0;
	else{
		int Secnum = clunum+31;
		int offset = 2;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			if(ramFDD144[Secnum][offset*32]!=0xe5)return 0;
			offset++;
			if(offset>=16){
				if(getFATValue(clunum)!=0xfff&&getFATValue(clunum)!=0xff0){
					clunum=getFATValue(clunum);
					Secnum = clunum+31;
					offset = 2;
				}
				else{
					break;
				}
			}
		}
		return 1;
	}
}
bool writetxt(char* path,int cutclusnum){
	char name[12];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		if(i>11)return -1; 
		name[i]=path[0];
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
	}
	else{
		for(i=0;i<11;i++){
			if(name[i]=='.'){
				name[8]=name[i+1];
				name[9]=name[i+2];
				name[10]=name[i+3];
				for(i;i<8;i++){
					name[i]=' ';
				}
				break;
			}
		}
		
	}
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
						b=false;
						break;
					}
				}
				
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return writetxt(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						emptyclu(tt->DIR_FstClus);
						tt->DIR_FstClus=findemptyclu();
						int cuclu=tt->DIR_FstClus;
						char buff[100000];
						printf("请输入内容：");
						scanf("%s",buff);
						int j;
						int r=0;
						for(i=0;i<=(strlen(buff)-1)/512;i++){
							for(j=0;j<512;j++){
								ramFDD144[cuclu+31][j]=buff[r];
								r++;
							}
							if(i+1>(strlen(buff)-1)/512){
								writefat(cuclu,0xfff);
								break;
							}
							else{
								writefat(cuclu,0xfff);
								writefat(cuclu,findemptyclu());
								cuclu=getFATValue(cuclu);
								
							}
						} 
						time_t timep;
			    		struct tm *p;
			    		time (&timep);
			    		p=gmtime(&timep);
			    		tt->DIR_WrtTime=0;
						tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
						tt->DIR_WrtDate=0;
						tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
						tt->DIR_FileSize=strlen(buff);
						saveEntry(tt,Secnum,offset);
						
						return 1;
						
					}
					else{
						return false; 
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						b=false;
						break;
					}
				}
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return writetxt(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						emptyclu(tt->DIR_FstClus);
						tt->DIR_FstClus=findemptyclu();
						int cuclu=tt->DIR_FstClus;
						char buff[100000];
						printf("请输入内容：");
						scanf("%s",buff);
						int j;
						int r=0;
						for(i=0;i<=(strlen(buff)-1)/512;i++){
							for(j=0;j<512;j++){
								ramFDD144[cuclu+31][j]=buff[r];
								r++;
							}
							if(i+1>(strlen(buff)-1)/512){
								writefat(cuclu,0xfff);
								break;
							}
							else{
								writefat(cuclu,0xfff);
								writefat(cuclu,findemptyclu());
								cuclu=getFATValue(cuclu);
								
							}
						} 
						time_t timep;
			    		struct tm *p;
			    		time (&timep);
			    		p=gmtime(&timep);
			    		tt->DIR_WrtTime=0;
						tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
						tt->DIR_WrtDate=0;
						tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
						tt->DIR_FileSize=strlen(buff);
						saveEntry(tt,Secnum,offset);
						
						return 1;
						
					}
					else{
						return false; 
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
					cutclusnum=getFATValue(cutclusnum);
					Secnum = cutclusnum+31;
					offset = 0;
				}
				else{
					break;
				}
			}
		}
	}
	return false; 
}
bool app(char* path,int cutclusnum){
	char name[12];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		if(i>11)return -1; 
		name[i]=path[0];
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
	}
	else{
		for(i=0;i<11;i++){
			if(name[i]=='.'){
				name[8]=name[i+1];
				name[9]=name[i+2];
				name[10]=name[i+3];
				for(i;i<8;i++){
					name[i]=' ';
				}
				break;
			}
		}
		
	}
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
						b=false;
						break;
					}
				}
				
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return app(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						char x[100000];
						char buff[100000];
						gettxt(tt->DIR_FstClus,buff);
						emptyclu(tt->DIR_FstClus);
						tt->DIR_FstClus=findemptyclu();
						int cuclu=tt->DIR_FstClus;
						printf("请输入内容：");
						scanf("%s",x); 
						strcat(buff,x);
						int j;
						int r=0;
						for(i=0;i<=(strlen(buff)-1)/512;i++){
							for(j=0;j<512;j++){
								ramFDD144[cuclu+31][j]=buff[r];
								r++;
							}
							if(i+1>(strlen(buff)-1)/512){
								writefat(cuclu,0xfff);
								break;
							}
							else{
								writefat(cuclu,0xfff);
								writefat(cuclu,findemptyclu());
								cuclu=getFATValue(cuclu);
								
							}
						} 
						time_t timep;
			    		struct tm *p;
			    		time (&timep);
			    		p=gmtime(&timep);
			    		tt->DIR_WrtTime=0;
						tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
						tt->DIR_WrtDate=0;
						tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
						tt->DIR_FileSize=strlen(buff);
						saveEntry(tt,Secnum,offset);
						
						return 1;
						
					}
					else{
						return false; 
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						b=false;
						break;
					}
				}
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return app(path,(unsigned int)(tt->DIR_FstClus));
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						char x[100000];
						char buff[100000];
						gettxt(tt->DIR_FstClus,buff);
						emptyclu(tt->DIR_FstClus);
						tt->DIR_FstClus=findemptyclu();
						int cuclu=tt->DIR_FstClus;
						printf("请输入内容：");
						scanf("%s",x); 
						strcat(buff,x);
						int j;
						int r=0;
						for(i=0;i<=(strlen(buff)-1)/512;i++){
							for(j=0;j<512;j++){
								ramFDD144[cuclu+31][j]=buff[r];
								r++;
							}
							if(i+1>(strlen(buff)-1)/512){
								writefat(cuclu,0xfff);
								break;
							}
							else{
								writefat(cuclu,0xfff);
								writefat(cuclu,findemptyclu());
								cuclu=getFATValue(cuclu);
							}
						} 
						time_t timep;
			    		struct tm *p;
			    		time (&timep);
			    		p=gmtime(&timep);
			    		tt->DIR_WrtTime=0;
						tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
						tt->DIR_WrtDate=0;
						tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
						tt->DIR_FileSize=strlen(buff);
						saveEntry(tt,Secnum,offset);
						
						return 1;
						
					}
					else{
						return false; 
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
					cutclusnum=getFATValue(cutclusnum);
					Secnum = cutclusnum+31;
					offset = 0;
				}
				else{
					break;
				}
			}
		}
	}
	return false; 
}
bool cp(char* path,int cutclusnum,int s){
	char name[12];
	int i=0;
	for(i=0;i<11;i++){
		name[i]=' ';
	}
	i=0;
	while(path[0]!='/'&&path[0]!='\0'){
		if(i>11)return -1; 
		name[i]=path[0];
		//if(name[i]<='z'&&name[i]>='a')name[i]=name[i]-('a'-'A');
		int ii;
		for(ii=0;ii<strlen(path);ii++){
			path[ii]=path[ii+1];
		}
		
		i++;
		
	}
	
	if(path[0]=='/'){
		for(i=0;i<strlen(path);i++){
			path[i]=path[i+1];
		}
	}
	else{
		for(i=0;i<11;i++){
			if(name[i]=='.'){
				name[8]=name[i+1];
				name[9]=name[i+2];
				name[10]=name[i+3];
				for(i;i<8;i++){
					name[i]=' ';
				}
				break;
			}
		}
		
	}
	if(cutclusnum==0){
		int Secnum = 19;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						//printf("%cand%c %d\n",name[i],tt->DIR_Name[i],i);
						b=false;
						break;
					}
				}
				
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return cp(path,(unsigned int)(tt->DIR_FstClus),s);
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						char x[100000];
						char buff[100000];
						gettxt(tt->DIR_FstClus,buff);
						emptyclu(tt->DIR_FstClus);
						tt->DIR_FstClus=findemptyclu();
						int cuclu=tt->DIR_FstClus;
						gettxt(s,x); 
						strcat(buff,x);
						int j;
						int r=0;
						for(i=0;i<=(strlen(buff)-1)/512;i++){
							for(j=0;j<512;j++){
								ramFDD144[cuclu+31][j]=buff[r];
								r++;
							}
							if(i+1>(strlen(buff)-1)/512){
								writefat(cuclu,0xfff);
								break;
							}
							else{
								writefat(cuclu,0xfff);
								writefat(cuclu,findemptyclu());
								cuclu=getFATValue(cuclu);
								
							}
						} 
						time_t timep;
			    		struct tm *p;
			    		time (&timep);
			    		p=gmtime(&timep);
			    		tt->DIR_WrtTime=0;
						tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
						tt->DIR_WrtDate=0;
						tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
						tt->DIR_FileSize=strlen(buff);
						saveEntry(tt,Secnum,offset);
						
						return 1;
						
					}
					else{
						return false; 
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				offset=0;
				Secnum++;
				if(Secnum>=19+14){
					break;
				}
			}
		}
	}
	else{
		int Secnum = cutclusnum+31;
		int offset = 0;
		while (ramFDD144[Secnum][offset*32]!=0){
			//printf("a");
			Entry t;
			Entry* tt=&t;
			loadEntry(tt,Secnum, offset);
			if(!((tt->DIR_Attr==0x27&&!showhidd)||(u8)(tt->DIR_Name[0])==0xe5)){
				//printf("a");
				int b=true;
				for(i=0;i<11;i++){
					if(name[i]!=tt->DIR_Name[i]){
						b=false;
						break;
					}
				}
				if(b){
					if (tt->DIR_Attr==0x10&&path[0]!='\0'){
						return cp(path,(unsigned int)(tt->DIR_FstClus),s);
					}
					else if((tt->DIR_Attr==0x20||tt->DIR_Attr==0x27)&&path[0]=='\0'){
						char x[100000];
						char buff[100000];
						gettxt(tt->DIR_FstClus,buff);
						emptyclu(tt->DIR_FstClus);
						tt->DIR_FstClus=findemptyclu();
						int cuclu=tt->DIR_FstClus;
						gettxt(s,x); 
						strcat(buff,x);
						int j;
						int r=0;
						for(i=0;i<=(strlen(buff)-1)/512;i++){
							for(j=0;j<512;j++){
								ramFDD144[cuclu+31][j]=buff[r];
								r++;
							}
							if(i+1>(strlen(buff)-1)/512){
								writefat(cuclu,0xfff);
								break;
							}
							else{
								writefat(cuclu,0xfff);
								writefat(cuclu,findemptyclu());
								cuclu=getFATValue(cuclu);
							}
						} 
						time_t timep;
			    		struct tm *p;
			    		time (&timep);
			    		p=gmtime(&timep);
			    		tt->DIR_WrtTime=0;
						tt->DIR_WrtTime=((8+p->tm_hour)<<11)+((p->tm_min)<<5);
						tt->DIR_WrtDate=0;
						tt->DIR_WrtDate=((1900+p->tm_year-1980)<<9)+((1+p->tm_mon)<<5)+(p->tm_mday);
						tt->DIR_FileSize=strlen(buff);
						saveEntry(tt,Secnum,offset);
						
						return 1;
						
					}
					else{
						return false; 
					}
					
				}
				else{
					 
				}
			}
			offset++;
			if(offset>=16){
				if(getFATValue(cutclusnum)!=0xfff&&getFATValue(cutclusnum)!=0xff0){
					cutclusnum=getFATValue(cutclusnum);
					Secnum = cutclusnum+31;
					offset = 0;
				}
				else{
					break;
				}
			}
		}
	}
	return false; 
}
