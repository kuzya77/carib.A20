#ifndef __IMAGELIST_H__
#define __IMAGELIST_H__

#include <types.h>
#include <mem.h>

#include <stuff/anim.h>

#include <vector>
#include <list>
#include <unordered_map>
#include <stuff/menu.h>

class ImageFrame
{
public:
    IMAGE img;
    tPoint pos;
  
    const ImageFrame& operator =(const ImageFrame&);

    ImageFrame():img(NULL)  { pos.x=pos.y=0; }
    ~ImageFrame()           { imgFree(img); }
};

class ImageEvent        // Событие, назначаемое некоемому кадру
{
    typedef std::vector<std::string> cmdArray;

    cmdArray cmds;    // Список команд, ассоциированных с кадром    
public:
    int frame;                  // номер кадра

    ImageEvent(ParseNode*);     // Ссылка на узел, содержащий описание события
    void exec() const;          // Выполнение команд события

    bool operator < (const ImageEvent& ie) const { return frame<ie.frame; } // Сортировка команд по порядку кадров
    bool empty() const { return cmds.empty(); }
};

// Расшареное (т.е. все списки с одними и теми же именами хранятся 1раз)
// хранилище списков рисунков и покадровых событий

class ImageList:public NamedObj
{
    int ref_count;        // Кол-во ссылок на данный имиджлист
    typedef std::unordered_map<u32_t,ImageList*> iMap;
    static iMap *imap;
protected:
    std::vector<ImageFrame> images;    
    int 	i_fps;

    ImageList(ParseNode* node,const char* fname,int default_fps);
public:
    typedef std::vector<ImageEvent> eventsArray;

    eventsArray events;   // Список событий

    virtual void draw(int x,int y,int index);   // Отрисовка картинки по указанным координатам
    virtual bool isTransparent() const;

    int   	count() const { return images.size(); }
    int   	fps() const   { return i_fps; }
    IMAGE get(int i) const  { return images[i].img; }
    

    static ImageList* load(ParseNode* node,const char* fname,int fps);
    static ImageList* load(const char* fname,int fps);

    static void destroy(ImageList* p);

    virtual ~ImageList();
};

class ZImageList: public ImageList
{
public:
    ZImageList(ParseNode* node,const char* fname,int fps):ImageList(node,fname,fps) {}
    void draw(int x,int y,int index);
    bool isTransparent() const;
};

////////////////////////////////////////////////////////////////////////
class ImageRect
{
      ImageRect() {}
protected:
      std::vector<cRect> rect;
      cRect new_rect;
public:
      ImageRect(ParseNode *node);
      const cRect* getRect(int count,int x,int y);
      int   size()  const{return rect.size();}

};

class ImageListRect:public ImageList,public ImageRect
{                                    
public:
    ImageListRect(ParseNode* node,const char* fname,int fps);
    virtual void draw(int x,int y,int index);
};
class ZImageListRect:public ImageListRect
{                                    
public:
    ZImageListRect(ParseNode* node,const char* fname,int fps):ImageListRect(node,fname,fps){};
    void draw(int x,int y,int index);
    bool isTransparent() const;
};


inline ImageList* LoadImageList(ParseNode* node,int fps=30)   { return ImageList::load(node,NULL,fps); }
inline ImageList* LoadImageList(const char* name,int fps=30)  { return ImageList::load(name,fps); }

inline void FreeImageList(ImageList* p) { ImageList::destroy(p); }

#endif
