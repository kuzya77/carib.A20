#include <log.h>

#include <binfile.h>
#include <string.h>
#include <stuff/cmd.h>
#include <stuff.h>

#include "utils.h"
#include "cmdargs.h"

const int TMP_BUF_SIZE = 1024;
const int CMD_NAME_SIZE = 128;

enum CmdType 
{
  CMD_FLOAT,
  CMD_INT,
  CMD_UINT,
  CMD_STRING,
  CMD_BOOL,
  CMD_COMMAND
};

struct Var
{
  CmdType type;
  cmd_fn  fn, update_fn;

  union {
    int*   i;
    unsigned int*  ui;
    float* f;
    char*  s;
    bool*  b;
    void*  param;
  };
};

struct CmdEntry
{
  char     name [CMD_NAME_SIZE];
  u32_t    hash;
  CmdEntry *prev,*next; 
  Var      var;
};

static CmdEntry* first = NULL;
static Var*      cur_exec = NULL;

static void cmdSetVar (const AttrList& args) //add var pointer
{
  if (cur_exec && args.count () >= 1) 
  {
    switch (cur_exec->type)
    {
      case CMD_INT:    *cur_exec->i  = args [0]; break;
      case CMD_UINT:   *cur_exec->ui = args [0]; break;
      case CMD_FLOAT:  *cur_exec->f  = args [0]; break;
      case CMD_BOOL:   *cur_exec->b  = args [0]; break;
      case CMD_STRING: strcpy (cur_exec->s,args [0]); break;
      default: break;
    }
  }
}

static CmdEntry* cmdFind (u32_t hash)
{
  for (CmdEntry* i=first;i;i=i->next)
    if (i->hash == hash)
      return i;

  return NULL;
}

static CmdEntry* cmdFind (const char* name)
{
  return name?cmdFind (istrhash (name)):NULL;
}

static bool cmdCreateEntry (const char* name,CmdType type,cmd_fn fn,cmd_fn update_fn,void* param)
{
  if (!name || !fn)
    return false;

  CmdEntry* cmd = cmdFind (name);

  if (!cmd)
  {
    cmd = new CmdEntry;

    if (!cmd)
    {
      logWarning ("Cmd: No memory for add new entry");
      return false;
    }  

    strcpy (cmd->name, name);

    cmd->hash  = istrhash (name);
    cmd->prev  = NULL;
    cmd->next  = first;

    if (first)
      first->prev = cmd;

    first = cmd;
  }  

  cmd->var.type      = type;
  cmd->var.fn        = fn;
  cmd->var.update_fn = update_fn;
  cmd->var.param     = param;

  return true;
}

bool cmdAddCmd (const char* name,cmd_fn fn)
{
  return cmdCreateEntry (name,CMD_COMMAND,fn,NULL,NULL);
}

bool cmdAddVar (const char* name,int* var,cmd_fn update_fn)
{
  return cmdCreateEntry (name,CMD_INT,cmdSetVar,update_fn,var);  
}

bool cmdAddVar (const char* name, unsigned int* var,cmd_fn update_fn)
{
  return cmdCreateEntry (name,CMD_UINT,cmdSetVar,update_fn,var);  
}

bool cmdAddVar (const char* name,float* var,cmd_fn update_fn)
{
  return cmdCreateEntry (name,CMD_FLOAT,cmdSetVar,update_fn,var); 
}

bool cmdAddVar (const char* name,bool* var,cmd_fn update_fn)
{
  return cmdCreateEntry (name,CMD_BOOL,cmdSetVar,update_fn,var);  
}

bool cmdAddVar (const char* name,char* var,cmd_fn update_fn)
{
  return cmdCreateEntry (name,CMD_STRING,cmdSetVar,update_fn,var);
}

static void cmdRemove (CmdEntry* cmd)
{
  if (!cmd)
    return ;

  if (cmd->prev) cmd->prev->next = cmd->next;
  else           first           = cmd->next;

  if (cmd->next) cmd->next->prev = cmd->prev;

  delete cmd;
}

void cmdRemove (const char* mask)
{
  if (!mask)
    return;

  for (CmdEntry* i=first;i;)
    if (maskcmp (i->name,mask))
    {
      CmdEntry* next = i->next;

      cmdRemove (i);

      i = next;
    }  
    else i = i->next;
}

static void cmdExec (CmdArgs& args)
{
  if (!args.count ())
    return;

  CmdEntry* cmd = cmdFind (attrs (args,0));

  if (cmd && cmd->var.fn)
  {
    cur_exec = &cmd->var;

    args.shift (1);

    cmd->var.fn (args);

    cur_exec = NULL;
  }
}

void cmdExec (const char* command)
{  
  if (!command)
    return;

  CmdArgs args (command);

  cmdExec (args);
} 

void cmdExecf (const char* format,...)
{
  va_list list;

  va_start (list,format);

  CmdArgs args;

  args.vparse (format,list);

  cmdExec (args);
  
  va_end   (list);
}

void cmdSet (const char* var_name,const char* attr)
{  
  if (var_name)
    cmdExecf ("%s '%s'",var_name,attr?attr:"");
}

void cmdSet (const char* var_name,int attr)
{
  if (var_name)
    cmdExecf ("%s %d",var_name,attr);
}

void cmdSet (const char* var_name,unsigned int attr)
{
  if (var_name)
    cmdExecf ("%s %u",var_name,attr);
}

void cmdSet (const char* var_name,float attr)
{
  if (var_name)
    cmdExecf ("%s %f",var_name,attr);
}

static void update (CmdEntry* cmd)
{
  if (!cmd->var.update_fn || !cmd->var.param) 
    return;

  const char* format = "";

  switch (cmd->var.type)
  {
    case CMD_INT:     format = "%s %d"; break;
    case CMD_UINT:    format = "%s %u"; break;
    case CMD_FLOAT:   format = "%s %f"; break;
    case CMD_BOOL:    format = "%s %d"; break;
    case CMD_STRING:  format = "%s %s"; break;
    default:          break;
  }
 
  cmd->var.update_fn (CmdArgs (format,cmd->name,cmd->var.param));
}

template <class T,class T1>
T get_value (const char* var_name,T1 (*conv)(const char*))
{
  CmdEntry* cmd = cmdFind (var_name);

  if (!cmd)
    return 0;  

  update (cmd);

  if (cmd->var.param)
  {
    switch (cmd->var.type)
    {
      case CMD_INT:     return (T)*cmd->var.i;
      case CMD_UINT:    return (T)*cmd->var.ui;
      case CMD_FLOAT:   return (T)*cmd->var.f;
      case CMD_BOOL:    return (T)*cmd->var.b;
      case CMD_STRING:  return conv (cmd->var.s);
      default:          return T ();
    }
  }  

  return 0;
}

int cmdGeti (const char* var_name)
{
  return get_value<int> (var_name,atoi);
}

unsigned int cmdGetui (const char* var_name)
{
  return get_value<unsigned int> (var_name,atoi);
}

float cmdGetf (const char* var_name)
{
  return get_value<float> (var_name,atof);
}

const char* cmdGets(const char* var_name,char* buf)
{
	static char tmp_buf [256];

  	if(!buf)
    	buf = tmp_buf;  

  	CmdEntry* cmd = cmdFind (var_name);

  	if(cmd)
	{
  		update (cmd);
  		if(cmd->var.param)
    		switch (cmd->var.type)
    		{
				case CMD_INT:     return sprintf (buf,"%d",*cmd->var.i), buf;
      			case CMD_UINT:    return sprintf (buf,"%u",*cmd->var.ui), buf;
      			case CMD_FLOAT:   return sprintf (buf,"%f",*cmd->var.f), buf;
      			case CMD_BOOL:    return *cmd->var.b ? "1" : "0";
      			case CMD_STRING:  return cmd->var.s;
    		}
  	}
  	return "";
}

bool cmdLoad (const char* fname)
{  
  if (!fname)
    return false;

  BINFILE file = bfOpen (fname,OPEN_RO);

  if (!file)
    return false;

  char buf [TMP_BUF_SIZE];

  while (bfGets(file,buf,sizeof (buf)))
    cmdExec (buf);

  bfClose (file);

  return true;
}

bool cmdSave (const char* fname,const char* mask)
{
  if (!mask)
    mask = "*";

  char buf [TMP_BUF_SIZE];

  for (char* dst=buf;*mask;*dst++=toupper (*mask++));

  mask = buf;

  if (fname)
  {
    BINFILE file = bfOpen (fname,OPEN_CR);

    if (file)
    {
      for (CmdEntry* i=first;i;i=i->next)
        if (i->var.param && i->var.type != CMD_COMMAND && maskcmp(i->name,mask))
        {
          char buf [1024];
          *buf = 0;

          if (i->var.param)
          {
            switch (i->var.type)
            {
              case CMD_INT:     sprintf (buf,"%s %i\n",i->name,*i->var.i); break;
              case CMD_UINT:    sprintf (buf,"%s %d\n",i->name,*i->var.ui); break;
              case CMD_FLOAT:   sprintf (buf,"%s %.3f\n",i->name,*i->var.f); break;
              case CMD_BOOL:    sprintf (buf,"%s %d\n",i->name,(int)*i->var.b); break;
              case CMD_STRING:  sprintf (buf,"%s '%s'\n",i->name,i->var.s); break;
              default:          break;
            }

            bfWrite (file,buf,strlen(buf));
          }
        }    

      bfClose (file);
    }
  }

  return false;
}
