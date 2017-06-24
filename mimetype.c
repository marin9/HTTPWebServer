#include <stdio.h>
#include <string.h>
#include "mimetype.h"
#include "file.h"

const struct ftype filetypes[]={{"png", "image/png", "img"},
								  {"jpg", "image/jpg", "img"},
						 		 {"jpeg", "image/jpeg", "img"},
						 		 {"bmp", "image/bmp", "img"},
						 		 {"gif", "image/gif", "img"},
						 		 {"tiff", "image/tiff", "img"},
								 {"ico", "image/x-icon", "img"},
						  
								 {"mp3", "audio/mpeg3", "audio"},
								 {"wav", "audio/wav", "audio"},
						  		 {"ogg", "audio/ogg", "audio"},
				
						  		{"mp4", "video/mp4", "video"},
						  		{"flv", "video/x-flv", "video"},
						  		{"3gp", "video/3gp", "video"},
						  		{"mov", "video/quicktime", "video"},
						  
						  		{"doc", "application/msword", "doc"},
						  		{"dot", "application/msword", "doc"},
						  		{"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", "doc"},
						  		{"ppt", "application/vnd.ms-powerpoint", "doc"},
						  		{"pot", "application/vnd.ms-powerpoint", "doc"},
						  		{"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation", "doc"},
						  		{"xls", "application/vnd.ms-excel", "doc"},
						  		{"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "doc"},
						  		{"pdf", "application/pdf", "doc"},
						  
						  		{"rar", "application/octet-stream", "com"},
						  		{"zip", "application/zip", "com"},
						  		{"iso", "application/x-iso9660-image", "com"},
						  		{"jar", "application/java-archive", "com"},  		
						  
						  		{"exe", "application/octet-stream", "exe"},
						  		{"sh", "text/x-script.sh", "exe"},
								{"o", "text/x-script.sh", "exe"},
						  
						  
						  		{"txt", "text/txt", "txt"},
						  		{"c", "text/txt", "c"},
						  		{"h", "text/txt", "h"},
						  		{"cpp", "text/txt", "cpp"},
						  		{"py", "text/txt", "py"},
						  		{"java", "text/txt", "java"},
						  		{"hpp", "text/txt", "hpp"},
								{"s", "text/txt", "asm"},
								{"S", "text/txt", "asm"},
								{"asm", "text/txt", "asm"},
						  		{"php", "text/txt", "php"},	
								{"html", "text/txt", "html"},	

								{"bin", "application/octet-stream", "bin"},				  					  
						  
						  		{"", "application/octet-stream", "unk"},
};


const char* getFileType(char *name, char *mtype){
	char ext[NAMELEN];
	int i=strlen(name)-1;
	int j=0;

	//find extension
	while(name[i]!='.' && i>0) --i;
	if(name[i]==0){
		return "unk";
	}
	++i;
	
	//get extension
	while(name[i]!=0){
		ext[j]=name[i];
		++j;
		++i;
	}
	ext[j]=0;
	
	i=0;
	//find mimetype
	while(strcmp(filetypes[i].ext, "")!=0){
		if(strcmp(filetypes[i].ext, ext)==0){
			break;	
		}	
		++i;
	}
	
	if(mtype!=NULL) strcpy(mtype, filetypes[i].mimetype);
	return filetypes[i].icon;
}

