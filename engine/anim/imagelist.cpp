#include <types.h>
#include <mem.h>
#include <log.h>

#include "imagelist.h"
#include <stuff/cmd.h>

const char  EVENT_TAG_FRAME[]="frame",    //    номер картинки                                        
            EVENT_TAG_CMDS[] ="cmds",     //    команда                                               
            IL_TAG_PATH[]="path",         //    путь к файлу с настройками(он же имя списка картинок) 
            IL_TAG_FPS[]="fps",           //    собственно фпс для данного имжЛиста                   
            IL_TAG_POS[]="pos",           //    перечень координат для каждой картинки                
            IL_TAG_FILES[]="files",       //    для явного задания путей к картинкам                                                      
            IL_TAG_MASK[]="mask",         //    тоже , что и выше...только маска
            IL_TAG_FIRST_BMP[]="first",   //      для маски, первая картинка
            IL_TAG_COUNT_BMP[]="count",   //      для маски, кол-во картинок
            IL_TAG_STEP_BMP[] ="step",    //      для маски, шаг при загрузке
            IL_TAG_REVERS[]=  "revers",   //    типа ,бля,ушатанная загрузка с реверсом
            IL_TAG_EVENT[]="Event",       //    узел для события                                                       
            IL_TAG_ZCOLOR[]="zcolor",     //    если есть то отрисовки происходит с выбиванием
            IL_TAG_CRECT[]="rect";       //    если есть то картинки выводяться с плоскостью  отсечения

ImageList::iMap* ImageList::imap=NULL;

const ImageFrame& ImageFrame::operator =(const ImageFrame& iframe)
{
    imgFree(img);
    img=imgCopy(iframe.img, BPP_DEFAULT, IMG_SHARED); 
    pos=iframe.pos; 
    return *this;
}

//------------------------------------------------------------------------------------
NamedObj::NamedObj(ParseNode* node) 
{ 
    const char* s=attrs(node->list(),0);
    if(s)
        i_name=s;
}

//------------------------------------------------------------------------------------

ImageEvent::ImageEvent(ParseNode* node)
{
    frame=attri(node,EVENT_TAG_FRAME,-1);
    const AttrList& l=attrlist(node,EVENT_TAG_CMDS);
    cmds.resize(l.count());
    for(int i=0;i<l.count();i++)
        cmds[i]=attrs(l,i);
}

void ImageEvent::exec() const
{
    for(cmdArray::const_iterator i=cmds.begin();i!=cmds.end();i++)
    {
//        logDebug("\t exec \"%s\"",i->c_str());
        cmdExec(i->c_str ());
    }
}

//------------------------------------------------------------------------------------

ImageList::ImageList(ParseNode* node,const char* fname,int default_fps):ref_count(1)
{
    if(fname)
    {
        i_name=fname;
        if(!imap)
            imap=new iMap;
        u32_t id_crc=crc32(i_name.c_str(),i_name.size()); 
        (*imap)[id_crc]=this;
    }

    i_fps=attri(node,IL_TAG_FPS,default_fps);

    const char* rc_path=attrs(node,IL_TAG_PATH,"");

    const AttrList& flist=attrlist(node,IL_TAG_FILES);

    const AttrList& plist=attrlist(node,IL_TAG_POS);

    int count=flist.count();
    if(count<=0)
        count=attri(node->first(IL_TAG_MASK),IL_TAG_COUNT_BMP,-1);

    if(count<=0)
        count=plist.count()/2;

    if(count<=0)
        throw std::runtime_error("bad count(<=0)");

    bool revers=node->first(IL_TAG_REVERS);   //by lesha

    images.resize(revers?2*count-1:count);

    int p2=std::min(plist.count()/2,count);

    for(int i=0;i<p2;i++)
    {
        images[i].pos.x=plist[i*2];
        images[i].pos.y=plist[i*2+1];
    }

    char filename[512],*s=filename;
    if(rc_path)
    {
        strcpy(filename,rc_path);    
        s+=strlen(rc_path);
    }
    if(flist.count())
    {
        for(int i=0;i<count;i++)
        {
            const char* shortfname=flist[i];
            if(shortfname[0])   //!=""
            {
                strcpy(s,shortfname);
                images[i].img=imgLoad(filename, gfxBpp(), IMG_SHARED|IMG_NOPALETTE);
                if(!images[i].img)
                {
                    logError("ImageList::ImageList error: can't load \"%s\"",filename);
                    conPrintf("ImageList::ImageList error: can't load \"%s\"",filename);
                    throw std::runtime_error("can't load image");
                }
            }
        }
    }
    else
    {
        ParseNode* enode=node->first(IL_TAG_MASK);
        const char *mask=attrs(enode->list(),0);
        if(!mask)
            throw std::runtime_error("there aren't files for load");
        const int step =attri(enode,IL_TAG_STEP_BMP,1);
        const int first=attri(enode,IL_TAG_FIRST_BMP,0);
        for(int i=0;i<count;i++)
        {       
            sprintf(s,mask,first+i*step);
            images[i].img=imgLoad(filename,gfxBpp(),IMG_SHARED|IMG_NOPALETTE);
            if(!images[i].img)
            {
                logError("ImageList::ImageList error: can't load \"%s\"",filename);
                conPrintf("ImageList::ImageList error: can't load \"%s\"",filename);
                throw std::runtime_error("can't load image");
            }
        }              
    }
    if(revers)
        for(int i=0;i<count-1;i++)
            images[2*count-2-i]=images[i];

    for(ParseNode* enode=node->first(IL_TAG_EVENT);enode;enode=enode->next(IL_TAG_EVENT))
    {
        ImageEvent event(enode);
        if(event.frame>=0 && event.frame<(int)images.size() && !event.empty())
            events.push_back(event);
    }
    sort(events.begin(),events.end());
}

void ImageList::draw(int x,int y,int index)   // ЋваЁб®ўЄ  Є авЁ­ЄЁ Ї® гЄ § ­­л¬ Є®®а¤Ё­ в ¬
{
    if(index<0 || index>=count())
        logError("ImageList(\"%s\")::draw invalid index=%d",name(),index);
    else
    {
        ImageFrame& f=images[index];
        gfxPutImage(x+f.pos.x,y+f.pos.y,f.img);
    }
}

bool ImageList::isTransparent() const
{
  return false;
}

//------------------------------------------------------------------------------------

void ImageList::destroy(ImageList* p)
{
    if(p && --p->ref_count<=0)
        delete p;
}

ImageList::~ImageList()
{
    // “­Ёзв®¦Ґ­ЁҐ ббл«®Є ў imap
    if(!i_name.empty() && imap)
    {
        u32_t id_crc=crc32(i_name.c_str(),i_name.size()); // ЁйҐ¬ ў Є авҐ бЇЁбЄ®ў Є авЁ­®Є...
        for(auto pr=imap->equal_range(id_crc);pr.first!=pr.second;pr.first++)
            if(pr.first->second==this)
            {
                imap->erase(pr.first);
                if(imap->empty())   // Ґб«Ё бЇЁб®Є Їгбв, г­Ёзв®¦ Ґ¬ ҐЈ®
                {
                    delete imap;
                    imap=NULL;
                }
                break;
            }
    }
}

ImageList* ImageList::load(ParseNode* node,const char* fname,int fps)
{
    ImageList* il=NULL;
    try
    {
        if(attrs(node,IL_TAG_ZCOLOR))    // ZImageList
        {
            if(node->first(IL_TAG_CRECT))              //Ґбвм Ї«®бЄ®бвм ®вбҐзҐ­Ёп
                 il=new ZImageListRect(node,fname,fps);
            else 
                 il=new ZImageList(node,fname,fps);
        }
        else
        {
            if(node->first(IL_TAG_CRECT))              //Ґбвм Ї«®бЄ®бвм ®вбҐзҐ­Ёп
                  il=new ImageListRect(node,fname,fps);
            else 
                  il=new ImageList(node,fname,fps);
        }
        return il;
    }
    catch(std::runtime_error& re)
    {
        logError("LoadImageList: error %s",re.what());
    }
    catch(std::bad_alloc& ba)
    {
        logError("LoadImageList: error %s",ba.what());
    }
    catch(...)
    {
        logError("LoadImageList: shit happens");
    }
    delete il;
    return NULL;
}

ImageList* ImageList::load(const char* fname,int fps)
{
    ImageList* il=NULL;
    try
    {
        if(imap)    // ЁйҐ¬ ў г¦Ґ § Јаг¦Ґ­­ле...
        {
            u32_t id_crc=crc32(fname,strlen(fname));
            auto pr=imap->equal_range(id_crc);
            if(pr.first!=pr.second) // ббл«Є  ­ ©¤Ґ­ , ў®§ўа й Ґ¬ Ґс
            {
                il=pr.first->second;
                il->ref_count++;
                return il;
            }
        }

        ParseNode* root=parLoadWXF(fname,512,2048);
        if(root)
        {
            il=load(root,fname,fps);
            parFree(root);
            if(!il)
                logError("LoadImageList: can't load \"%s\"",fname);
        }
        return il;
    }
    catch(std::runtime_error& re)
    {
        logError("LoadImageList: error %s",re.what());
    }
    catch(std::bad_alloc& ba)
    {
        logError("LoadImageList: error %s",ba.what());
    }
    catch(...)
    {
        logError("LoadImageList: shit happens");
    }
    delete il;
    return NULL;
}

//------------------------------------------------------------------------------------

void ZImageList::draw(int x,int y,int index)   // ЋваЁб®ўЄ  Є авЁ­ЄЁ Ї® гЄ § ­­л¬ Є®®а¤Ё­ в ¬
{
    if(index<0 || index>=count())
        logError("ZImageList(\"%s\")::draw invalid index=%d",name(),index);
    else
    {
        ImageFrame& f=images[index];
        gfxPutImageA(x+f.pos.x, y+f.pos.y, f.img);
    }
}

bool ZImageList::isTransparent() const
{
  return true;
}

//_____________________________________________________________________________________
ImageRect::ImageRect(ParseNode *node)
{
      const AttrList& list=attrlist(node,IL_TAG_CRECT);

      int count=list.count()/4;
      
      if(count<=0) 
      {
            char s[100];
            sprintf(s,"ImageRect: bad count in tag '%s'",IL_TAG_CRECT);
            throw std::runtime_error(s);
      }
      rect.resize(count);
      for(int i=0;i<count;i++)
      {
            int   x = list[4*i+0],      
                  y = list[4*i+1],     
                  dy= list[4*i+2],
                  dx= list[4*i+3];     

            rect[i]=cRect(x,y,x+dx,y+dy);       
//            logDebug("load rect(%d %d %d %d)...(%d  %d  %d  %d)",x,x+dx,y,y+dy,
//                      rect[i].left,rect[i].right,rect[i].bottom,rect[i].top);
      }
}

const cRect* ImageRect::getRect(int index,int x,int y)
{
      new_rect.left=rect[index].left+x;
      new_rect. top=rect[index].top+y;

      new_rect.right=rect[index].right+x;
      new_rect.bottom=rect[index].bottom+y;

      return &new_rect;
}
//---------------------------------------------------------------------------------------
ImageListRect::ImageListRect(ParseNode* node,const char* fname,int fps):ImageList(node,fname,fps),ImageRect(node)
{
      if(size()<count())
      {
            char s[100];
            sprintf(s,"ImageListRect: rect count(%d) not equality image count(%d)",size(),count());
            throw std::runtime_error(s);
      }
      for(int i=0;i<size();i++)
      {
            ImageFrame & f=images[i];
            f.pos.x-=rect[i].left;
            f.pos.y-=rect[i].top;
      }
}
void ImageListRect::draw(int x,int y,int index)
{
      if(index<0 || index>=count())
            logError("ImageList(\"%s\")::draw invalid index=%d",name(),index);
      else
      {
            ImageFrame & f=images[index];
            x+=f.pos.x;
            y+=f.pos.y;
            gfxPutImage(x,y,f.img,getRect(index,x,y));
      }
}
//______________________________________________________________________________________
void ZImageListRect::draw(int x,int y, int index)
{
      if(index<0 || index>=count())
            logError("ImageList(\"%s\")::draw invalid index=%d",name(),index);
      else
      {
            ImageFrame & f=images[index];
            x+=f.pos.x;
            y+=f.pos.y;
            gfxPutImage(x,y,f.img,getRect(index,x,y));
      }
}

bool ZImageListRect::isTransparent() const
{
	return true;
}
