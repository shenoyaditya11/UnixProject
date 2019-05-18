#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<limits.h>
#include<fcntl.h>
#include<wordexp.h>

char *prevDir=NULL;
char *recentDir=NULL;
char *currentDir=NULL;
wordexp_t *extended=NULL;

//extern int glob(char *, int , (int *), glob_t *)

void display(struct dirent *file){
	//stat(file->d_name, &details);
	//printf("%s\t", details->st_mode);			
	unsigned type= (unsigned)file->d_type;
	//printf("%u", type);
	if(type== 4)
		printf("\033[0;34m");
	else if(type==8)
		printf("\033[0;32m");
						
	printf("  |---> %s\n",file->d_name);
	printf("\033[0m");			

}


char *expand(char *searchFileName){
	wordexp(searchFileName, extended, WRDE_SHOWERR);
	return extended->we_wordv[0];
}


void printErr(char *err){
	printf("\033[1;31m");
	printf("%s\n",err);
	printf("\033[0m");


}
void travelDirectory(DIR *dir, char *filter, char *dirName){
	
	wordexp_t *extended= (wordexp_t *)malloc(sizeof(wordexp_t));
	wordexp(filter, extended, WRDE_SHOWERR);
		
	
	int found=0;
	int match=0;
	
	if(dir!= NULL){
		
		struct dirent *file=NULL;
		struct stat *details;
		
		char *fullPath;
		char *halfPath;
		
		
		
		fullPath= getcwd(fullPath,PATH_MAX);
		
		
		if(strcmp(dirName, ".")!=0){
			fullPath=NULL;
			free(fullPath);
			fullPath=(char *)malloc(strlen(dirName));
			for(int i=0;i<=strlen(dirName); i++)
				fullPath[i]= dirName[i];	
		}
		
		for(int i=0;i<=strlen(fullPath); i++){
			currentDir[i]= fullPath[i];
			prevDir[i]=fullPath[i];
		}
				
		halfPath= strrchr(fullPath, '/');
		int i=0;
		int len=halfPath-fullPath;
		//prevDir=fullPath;	
		prevDir[len]='\0';
		
		
		//printf("%s", currentDir);
		printf("\033[0;33m");
		printf("%s\n", ++halfPath);
		printf("\033[0m");
		printf("  |\n");
		
		//readdir api//
		while( (file= readdir(dir))!= NULL){
			i=0;
			while(i<extended->we_wordc){
				if(strcmp(filter, "*"))
					found=strcmp(file->d_name, extended->we_wordv[i]);
			
				if(strcmp(file->d_name, ".")!=0  && strcmp(file->d_name, "..")!=0 && found==0)
				{	
					display(file);
					match=1;
					break;
				}
				i++;
			}
		}
		
		if(match==0){
			printf("\033[0;31m");
			if(strcmp(filter, "*")==0)
				printf("This Directory is empty\n");
			else	
				printf("Cannot find the file named %s\n",filter);
			printf("\033[0m");
		}
		
		
		free(dir);
		free(fullPath);
		
	}
	else{
		printf("cannot access the directoryr\nEither directory do not exsits in present Directory\n\
		or you do not have accesse to directory");
	}
	
	wordfree(extended);
		
}

int main(int argc, char* argv[]){

	currentDir=(char *)malloc(PATH_MAX);
	prevDir=(char *)malloc(PATH_MAX);
	extended= (wordexp_t *)malloc(sizeof(wordexp_t));

	if(argc>1){
		if(strcmp(argv[1], "-help")==0){
			printf("\033[1;32m");
			printf("'^' (use without qoutes) to explore current working directory\n");
			printf("'<' (use without qoutes) to explore a directory above present directory\n");
			printf("'s' (use without qoutes) to search file in present directory\n");
			printf("'e' (use without qoutes) to visit the directory present in directory \n");
			printf("'o' (use without qoutes) to open file \n");
			printf("'n' (use without qoutes) to create directory or file \n");
			printf("'r' (use without qoutes) to rename file \n");
			printf("'d' (use without qoutes) to delete file \n");
			printf("\033[0m");
		}
		else if(strcmp(argv[1], "-s")==0){
			DIR *dir= opendir(".");
			//printf("%s", argv[2]);
			if(argc>=3)
				travelDirectory(dir, argv[2], ".");
				//printf("%s", argv[2]);
				
			else
				printErr("-s option need file name as argument to search");
					
		}
		else if(strcmp(argv[1], "-e")==0){
			
			if(argc>=3){
				currentDir[0]='.';
				currentDir[1]='\0';
				chdir(currentDir);
				strcat(currentDir, "/");
				strcat(currentDir, argv[2]);
				DIR *dir= opendir(currentDir);
				//printf("%s", currentDir);
				travelDirectory(dir, "*", currentDir);
			}
			else
				printErr("-e option need directory name to visit \n");
				
			
					
		}
		
		else
			printErr("Only option availabel is -help\n");
		
		exit(0);	
	}
		DIR *dir=NULL;
		char input='^';
		char *searchFileName= (char *)malloc(PATH_MAX);
		char *newFileName= (char *)malloc(PATH_MAX);
		int len=0;
		do{
		
			switch(input){
			
				case '^':
					 dir=opendir(".");
					 travelDirectory(dir, "*", ".");
					 break;	
				case '<':
					 dir=opendir(prevDir);
					 chdir(prevDir);
					 travelDirectory(dir, "*",prevDir);
					 break;
				case 's':
					printf("Enter file or Director to search: ");
					scanf("%s", searchFileName);
					dir=opendir(currentDir);
					travelDirectory(dir, searchFileName, currentDir);
				break;
				case 'e':
					printf("Enter the Directory to traverse: ");
					scanf("%s", searchFileName);
					chdir(currentDir);
					searchFileName= expand(searchFileName);
					len= strlen(currentDir);
					strcat(currentDir, "/");
					strcat(currentDir, searchFileName);
					dir=opendir(currentDir);
					if(dir==NULL){
						currentDir[len]='\0';
						printf("\033[1;31m");
						printf("%s is not the directory\n", searchFileName);
						printf("\033[0m");
						break;
					}
					//printf("%s", currentDir);
					travelDirectory(dir, "*", currentDir);
					chdir(currentDir);
					break;
				case 'o':
					printf("Enter the file to access: ");
					scanf("%s", searchFileName);
					len= strlen(currentDir);
					strcat(currentDir, "/");
					strcat(currentDir, searchFileName);
					if(!fork()){
						execlp("gedit", "gedit", expand(currentDir), NULL);
						
					}
					currentDir[len]='\0';	
					break;
				case 'r':
					printf("Enter the old and new file/directory name : ");
					scanf("%s", searchFileName);
					scanf("%s", newFileName);
					searchFileName= expand(searchFileName);
					if(rename(searchFileName, newFileName)==0){
						dir= opendir(currentDir);
						travelDirectory(dir, "*", currentDir);
					}
					else
						printErr("failed to change name\neither its permission or the file do not\
						 exsits\n");
					
					break;
				case 'd':
					printf("File f, Directory d, Cancle c [d/f/c]: ");
					scanf(" %c", &input);
					printf("Enter name to delete : ");
					scanf("%s", searchFileName);
					searchFileName= expand(searchFileName);
					if(input=='f'){
						printf("Are you sure you want to delete file %s [y/n]: ", searchFileName);
						input='y';
						scanf(" %c", &input);
						if(input=='y'){
							if(remove(searchFileName)==0)
								printf("file removed\n"); 	
							else
							printErr("failed to remove file\neither its permission or the file do not\
							 exsits\n");
							
								
						}
					}
					else{
						if(rmdir(searchFileName)==0)
							printf("removed directory\n");
						else
					
							printErr("failed to remove file\nreasons might be permission ,directory do not exsits or directory is not empty \n");
							
							
					
					}
					
					break;	
				case 'n':
					//create a file or folder//
					printf("File f, Directory d, Cancle c [d/f/c]: ");
					scanf(" %c", &input);
					if(input=='d'){
						printf("Folder Name : ");
						scanf("%s", searchFileName);
						if(mkdir(searchFileName, 0777)==0)
							printf("New directory created\n");
						else{
						
						printErr("failed to create directory\neither its permission or the folder\
						 exsits\n");
						
						}	
					}
					else if(input=='f'){
						printf("File Name : ");
						scanf("%s", searchFileName);
						int file=open(searchFileName, O_RDWR|O_CREAT, 0764);
						if(file!=-1){
							printf("New file created\n");
							close(file);
						}
						else
						
						printErr("failed to create file\neither its permission or the file \
						 exsits\n");
						
							
					}
					
					break;
								 		
				}
			scanf("%c", &input);
			
		
		}while(input!='~');

}
