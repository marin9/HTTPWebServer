#ifndef FILETYPE_H
#define FILETYPE_H


struct ftype{
	char ext[8];
	char mimetype[128];
	char icon[8];
};

const char* getFileType(char *name, char *mtype);

#endif
