#ifndef __STUFF_COMMAND__
#define __STUFF_COMMAND__

#include <mem.h>
#include <stuff/attr.h>

//handler
typedef void (*cmd_fn)(const AttrList&);

//add and update commands
bool cmdAddCmd (const char* name,cmd_fn);
bool cmdAddVar (const char* name,int*,cmd_fn update_fn=NULL);
bool cmdAddVar (const char* name,unsigned int*,cmd_fn = NULL);
bool cmdAddVar (const char* name,float*,cmd_fn =NULL);
bool cmdAddVar (const char* name,bool*,cmd_fn =NULL);
bool cmdAddVar (const char* name,char*,cmd_fn =NULL);

//remove by mask
void cmdRemove (const char* mask); //remove group

//execute command
void cmdExec  (const char* command);
void cmdExecf (const char* command,...);

//set vars
void        	cmdSet   (const char* var_name,const char*);
void        	cmdSet   (const char* var_name,int);
void        	cmdSet   (const char* var_name,unsigned int);
void        	cmdSet   (const char* var_name,float);
const char* 	cmdGets  (const char* var_name,char* buf=NULL);
int         	cmdGeti  (const char* var_name);
unsigned int	cmdGetui (const char* var_name);
float       	cmdGetf  (const char* var_name);

//load/save command
bool cmdLoad (const char* file_name);
bool cmdSave (const char* file_name,const char* mask=NULL);

#endif
