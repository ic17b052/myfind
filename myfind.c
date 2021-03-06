//why the user 160 exists??? first char shouldn't be a number
//test-find.sh row 491 User hugo do not exist;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <pwd.h>

#include <sys/types.h>
#include <dirent.h>

#include <fnmatch.h>

#include <libgen.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <ctype.h>


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <pwd.h>
#include <grp.h>
#include <time.h>



struct optionItem {
	char *name;
	int params;
};

char *option[] = {
		"-user",
		"-name",
		"-type",
		"-print",
		"-ls",
		"-nouser",
		"-path"
	
		//"-group",
		//"-nogroup"
	};


void errorMsg(int i);
void do_dir(const char * dir_name,  char * parms[]);
uid_t getUidFromString(char *id);


int isValidOption(char *option);
int in_array ( char *needle , char *haystack[], int arraySize);
struct optionItem *searchOption(char* optName);

void do_file(const char * dir_name,  char * parms[]);

char **cmdLine;
void lsprint(char* path);
void spclPrint(char *str);



int main(int argc, char* argv[])
{	
	cmdLine = argv;


	int expParam = 0;	
	struct optionItem *optItem = NULL;
	int optionPos = 0;

	for(int i = 0;i<argc;i++){
		switch(i) {
			case 0:
				//expect Program name
				break;
			case 1:
				//expect Path
				break;
			default:
				//expect Option/Parameter
				if(argv[i][0] == '-'){
					//Option
					if(expParam > 0) errorMsg(2); // expect Parameter
					
					optItem = searchOption(argv[i]);
					if(optItem == NULL) errorMsg(3); // unknown Option
					
					optionPos = i; // Store Option Position
					expParam = optItem->params;
					
				}else{
					//Parameter
					if(expParam == 0) errorMsg(4); // Expect Option
					expParam--;
			
				}
				break;	
		}	

		if(optItem != NULL && optionPos != 0 && expParam == 0){
			if(strcmp(optItem->name,"-user") == 0){
				if(getUidFromString(argv[( optionPos + 1 )]) == 0) errorMsg(5);  // unknown User
			}
			
			if(strcmp(optItem->name,"-type") == 0){
				char allowedFileTypes[] = "fdbcpls";
				char *validType = memchr(allowedFileTypes, argv[( optionPos + 1 )][0], strlen(allowedFileTypes));
				
				if(validType == NULL) errorMsg(6);  // unknown file type
			}
		}
	}
	
	if(expParam > 0) errorMsg(7);;  // expect additional option at the end;
	

	do_file(argv[1], &argv[2]);	
	
	
	return 0;
}

void errorMsg(int i){
	
	for(i=0;cmdLine[i]!=NULL;i++){
		fprintf( stderr, "%s ",cmdLine[i]);	
	}

	
	
	switch(i) {
		case 1: fprintf( stderr, "Error: Can not open directory"); exit(i); break;
		case 2: fprintf( stderr, "Error: expect parameter"); exit(i); break;
		case 3: fprintf( stderr, "Error: unknown option"); exit(i); break;
		case 4: fprintf( stderr, "Error: Expect Option"); exit(i); break;
		case 5: fprintf( stderr, "Error: unknown User"); exit(i); break;
		case 6: fprintf( stderr, "Error: unknown file type"); exit(i); break;		
		case 7: fprintf( stderr, "Error: expect additional option at the end"); exit(i); break;
		case 8: fprintf( stderr, "Error"); exit(i); break;
		case 9: fprintf( stderr, "Error"); exit(i); break;		
		default: printf("Another Error"); exit(i); break;
	}
}



void do_dir(const char * dir_name,  char * parms[]){
	
}



void do_file(const char * dir_name,  char * parms[]){
	
	char dirname[255] = { '\0' };
	char filename[255] = { '\0' };
	const char* tmp = dir_name;
	int j = 0;
	
	
	char *lastSlash = strrchr ( dir_name, '/' ); 
	const char *tmpSlash = dir_name;
	int k = 1;
	
	while(lastSlash != tmpSlash){
		k++;
		tmpSlash++;
	}
	
	strncpy(dirname, dir_name, k);
	tmp = tmp + k;
	
	for(int i = 0; i<strlen(tmp); i++){
		if(tmp[i] == '*' || tmp[i] == '?' || tmp[i] == '['|| tmp[i] == '\\') {
			filename[j] = '\\';
			j++;
		}

		filename[j] = tmp[i];
		j++;
	}

	DIR *currentDirectory =  opendir(dirname);

	if(currentDirectory == NULL) errorMsg(1); // Error Opndir
	
	struct dirent *currentDirEnt = NULL;
	
	
	
	
	
	while((currentDirEnt = readdir(currentDirectory)) != NULL){

		int print = 1;
		int found = fnmatch(filename, currentDirEnt->d_name, 0);

		char ganzerPfad[1024] = {'\0'};

		strcpy(ganzerPfad,dirname);
		strcat(ganzerPfad,currentDirEnt->d_name);
		
		struct stat newStatBuffer;
		lstat(ganzerPfad, &newStatBuffer);
		
		if(parms[0] != NULL){

			if( strcmp(parms[0],"-user") == 0){
				uid_t userid = getUidFromString(parms[1]);
				if(newStatBuffer.st_uid != userid)	print = 0;
			}
			
			if( strcmp(parms[0],"-nouser") == 0){
				if(getpwuid(newStatBuffer.st_uid) != NULL ) print = 0;
			}			
			
			if( strcmp(parms[0],"-name") == 0){
				if(fnmatch(parms[1], currentDirEnt->d_name, 0) != 0) print = 0;
			}
			if( strcmp(parms[0],"-path") == 0){
				if(fnmatch(parms[1], ganzerPfad, 0) != 0) print = 0;
			}
			
			if( strcmp(parms[0],"-type") == 0){
				print = 0;
				switch((int)parms[1][0]) {
					case 'f': if(S_ISREG(newStatBuffer.st_mode)) print = 1; break;
					case 'd': if(S_ISDIR(newStatBuffer.st_mode)) print = 1; break;
					case 'b': if(S_ISBLK(newStatBuffer.st_mode)) print = 1; break;
					case 'c': if(S_ISCHR(newStatBuffer.st_mode)) print = 1; break;
					case 'p': if(S_ISFIFO(newStatBuffer.st_mode)) print = 1; break;
					case 'l': if(S_ISLNK(newStatBuffer.st_mode)) print = 1; break;
					case 's':  print = 0; break;
					default: errorMsg(99); break;
				}			
			}
		}

		if(found == 0 && print == 1) {
			int tmpsmthgPrinted = 0;
			if(parms[0] != NULL){
				if( strcmp(parms[0],"-ls") == 0){
					lsprint(ganzerPfad);
					tmpsmthgPrinted = 1;
				}
			}
			
			if(tmpsmthgPrinted == 0){
				printf("%s%s\n",dirname,currentDirEnt->d_name);
			}
		}
	}
}


int in_array ( char *needle , char *haystack[], int arraySize){
	int inArray = 0;
	for(int i = 0; i<arraySize; i++){
		int cmpVal = strcmp(haystack[i],needle);
		if(cmpVal == 0) inArray = 1;
	}
	return !inArray;
}

struct optionItem *searchOption(char* optName){
	static struct optionItem optArray[] = {
		{.name="-user",.params=1},
		{.name="-name",.params=1},
		{.name="-type",.params=1},
		{.name="-print",.params=0},
		{.name="-ls",.params=0},
		{.name="-nouser",.params=0},
		{.name="-path",.params=1},
		//only for groups with 4 Persons
		//{.name="-group",.minParams=1,.maxParams=1),
		//{.name="-nogroup",.minParams=1,.maxParams=1)
	};
	
	int optionItemLen = (sizeof(optArray)/sizeof(optArray[0]));
	
	for(int i = 0;i<optionItemLen;i++){
		if(strcmp(optArray[i].name,optName) == 0) return &optArray[i];
	}
	
	return NULL;

}



int isValidOption(char *chkOption){

	struct optionItem *optItem = searchOption(chkOption);
	if(optItem != NULL)	return 1;
	
	return 0;
}

uid_t getUidFromString(char *id){
	struct passwd *userinfo;
	
	userinfo = getpwnam(id);
	if(userinfo != NULL){
		return userinfo->pw_uid;
	}

	if(isdigit(id[0]) == 0) return 0;
	
	long int userid = strtol(id, NULL, 10);
	userinfo =  getpwuid(userid);
	if(userinfo != NULL){
		return userinfo->pw_uid;;
	}
	return 0;
}


void lsprint(char* path){
	
	
	//char path[] = "/home/ic17b049/myfind/.git/branches";
	struct stat buf;
	int statRes = lstat(path, &buf);
	
	struct passwd *userInfo;
	struct group *groupInfo;
	
	if(statRes != 0){
		printf("ERROR lstat");
		exit(1);
	}
	
	// Inode
	printf("%6lu ",buf.st_ino); 
	printf(" ");
	
	
	//
	
	printf("%3lu", buf.st_blocks/2);
	printf(" ");
	// File type
	char fileType = '?';
	if(S_ISREG(buf.st_mode)) fileType = '-';
	else if(S_ISDIR(buf.st_mode)) fileType = 'd';
	else if(S_ISBLK(buf.st_mode)) fileType = 'b';
	else if(S_ISCHR(buf.st_mode)) fileType = 'c';
	else if(S_ISFIFO(buf.st_mode)) fileType = 'p';
	else if(S_ISLNK(buf.st_mode)) fileType = 'l';

	
	printf("%c", fileType);
	
	// Permision
	
	printf("%c", (buf.st_mode & S_IRUSR)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWUSR)? 'w' : '-');
	printf("%c", (buf.st_mode & S_IXUSR)? 'x' : '-');

	printf("%c", (buf.st_mode & S_IRGRP)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWGRP)? 'w' : '-');
	printf("%c", (buf.st_mode & S_IXGRP)? 'x' : '-');

	printf("%c", (buf.st_mode & S_IROTH)? 'r' : '-');
	printf("%c", (buf.st_mode & S_IWOTH)? 'w' : '-');
	printf("%c", (buf.st_mode & S_IXOTH)? 'x' : '-');	
	
	printf(" ");	
	
	//Hardlinks
	
	printf("%3i",buf.st_nlink); 
	printf(" ");
	
	
	//Username
	
	userInfo =  getpwuid(buf.st_uid);
	
	
	
	printf("%-8s", (userInfo!=NULL) ? userInfo->pw_name : "999999");		
	printf(" ");		

	//Groupname

	groupInfo = getgrgid(buf.st_gid);
	printf("%-8s", groupInfo->gr_name);		
	printf(" ");		
	
	// Size in Bytes
	printf("%8lu",buf.st_size); 
	printf(" ");	

	// Time
	
	char buff[20];
	time_t time = buf.st_mtime;
	strftime(buff, 20, "%b %e %H:%M", localtime(&time));
	printf("%s",buff);
	printf(" ");
	//Path
	//printf("%s",path);
	spclPrint(path);
	printf("\n");
}

void spclPrint(char *str){
	while(*str != '\0'){
		if(*str == '\\') printf("\\\\");
		else printf("%c",*str);
		str++;
	}
	
}

