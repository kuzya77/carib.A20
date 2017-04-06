#include <types.h>
#include <mem.h>
#include <picture.h>	// color_t
#include <gfx.h>

#include "menuimpl.h"
#include "frame.h"
#include "control.h"
#include "console.h"

extern void fileMenuFree();	// from loadwxf.cpp

static Menu* firstmenu=NULL;

Menu::Menu()
{
  prev = NULL;
  next = firstmenu;

  if (next)
    next->prev = this;

  firstmenu = this;

  tip_frame    = NULL;
  frames_count = 0;
  active_frame = 0;
  picture      = 0;

  rename ("__unnamed");
}

Menu::~Menu()
{
    while(frames_count>0)
        delete frame[0];
  
  if (prev) prev->next = next;
  if (next) next->prev = prev;

  if (this == firstmenu)
    firstmenu = next;
  
    imgFree(picture);    
}

Frame* Menu::curframe () const
{ 
  return frame [active_frame]; 
}

Control* Menu::curctrl () const
{ 
  return frame [active_frame]->curctrl (); 
}

void Menu::nextframe () 
{ 
  active_frame++;

  if (active_frame >= frames_count)
    active_frame = 0;
}

void Menu::prevframe () 
{
  active_frame--;

  if (active_frame < 0)
    active_frame = frames_count ? frames_count-1 : 0;
}

void Menu::left () 
{
  if (curctrl ()) curctrl ()->left ();
}

void Menu::right ()
{
  if (curctrl ()) curctrl ()->right ();
}

void Menu::up ()
{
  if (curctrl ()) curctrl ()->up ();    
}

void Menu::down ()
{
  if (curctrl ()) curctrl ()->down ();
}

void Menu::enter ()
{
  if (curctrl ()) curctrl ()->enter ();
}

void Menu::escape ()
{
  if (curctrl ()) curctrl ()->escape ();
}

void Menu::backspace ()
{
  if (curctrl ()) curctrl ()->backspace ();
}

void Menu::apply ()
{
  if (curctrl ()) curctrl ()->apply ();
}

Frame* Menu::findframe (const char* name) const
{
  if (!name)
    return NULL;

  u32_t hash = istrhash (name);

  for(int i=0; i<frames_count; i++)
    if (hash == frame [i]->name_hash) 
      return frame [i];

  return NULL;
}

void Menu::draw()
{
  cRect oldVP = gfxViewPort();

  if (palette ()) 
    gfxPutPalette (palette ());
  
  for (int i=0;i<framescount();i++)
    frame [i]->draw ();

  gfxViewPort (oldVP);
}

void Menu::select (const char* name)
{
  select (findframe (name));
}

void Menu::select (Frame* _frame)
{
  if (!_frame) 
    return;

  for (int i=0;i<frames_count;i++)
    if (frame [i] == _frame) 
    {
      active_frame = i;
      
      if (frame [i]->curctrl ())
        frame [i]->select (curctrl ());
        
      break;
    }
}

bool Menu::IsActive (const Frame* _frame) const
{
  return frame [active_frame] == _frame;
}

void Menu::SetTipFrame (const char* name)
{
  if (!name) 
    return;

  Frame* frame = findframe (name);

  if (frame) 
    tip_frame = frame;
}

void Menu::SetPalette (const char* name)
{  
    if(picture)
            imgFree(picture);
    picture=name?imgLoad(name,gfxBpp(),IMG_SHARED):0;
}

const RGBQUAD* Menu::palette () const 
{
  return picture ? imgPalette (picture) : NULL;
}

Menu* Menu::findmenu (const char* name)
{
  if (!name) 
    return NULL;

  u32_t hash = istrhash (name);

  for (Menu* menu=firstmenu;menu;menu=menu->next)                       //!!!!!!!!!!
    if (hash == menu->name_hash) 
        return menu;

  return NULL;
}

void Menu::click(int x, int y)
{
    for(int i=0;i<frames_count;i++)
    	if(frame[i]->click(x, y))
    	{
    		active_frame = i;
    		break;
    	}
}

void uiDestroy (Menu* menu)
{
  delete menu;              //для повторной загрузки                        !!!!!!!!!!!!!!!!!!!!
  //menu = NULL;
}

Menu* uiLoadMenu (const char* name)
{
  return Menu::findmenu (name);
}

Console* uiFindConsole (Menu* menu,const char* name)
{
  if (!menu || !name)
    return NULL;

  Frame* frame = menu->findframe (name);

  return dynamic_cast<Console*>(frame);
}

void uiDraw (Menu* menu)
{
  if (menu)
    menu->draw ();
}

void uiClick(Menu* menu, int x, int y)
{
	if(menu)
		menu->click(x, y);
}

void uiPutKey (Menu* menu,MenuKey key)
{
    if(menu)
        switch(key)
        {
            case MENU_KEY_UP:        menu->up (); break;
            case MENU_KEY_DOWN:      menu->down (); break;
            case MENU_KEY_LEFT:      menu->left (); break;
            case MENU_KEY_RIGHT:     menu->right (); break;
            case MENU_KEY_APPLY:     menu->apply (); break;
            case MENU_KEY_SELECT:    menu->enter (); break;
            case MENU_KEY_ESC:       menu->escape (); break;
            case MENU_KEY_BACKSPACE: menu->backspace (); break;
        }
}

bool uiIsActive (Menu* menu,const char* name)
{
  return name && menu->IsActive (menu->findframe (name)); 
}

////////////////////////

static void onShowFrame (const AttrList& list)
{
  Menu* menu = uiLoadMenu (attrs (list,0,""));
  
  if (!menu)
    return;

  Frame* frame = menu->findframe (attrs (list,1,""));

    if(frame)
        frame->show ();
}

static void onHideFrame (const AttrList& list)
{
    Menu* menu = uiLoadMenu (attrs (list,0,""));
    if(!menu)
        return;

    Frame* frame=menu->findframe (attrs (list,1,""));

    if(frame)
        frame->hide();
}

void uiInit ()
{
    cmdAddCmd ("menu_show_frame",onShowFrame);  
    cmdAddCmd ("menu_hide_frame",onHideFrame);
}

void uiShutdown()
{
    Menu* menu;
    Menu* mnext;
    
    //очищаем список меню                    
    for(menu=firstmenu;menu;)
    {
        mnext=menu->next;
        uiDestroy(menu);    
        menu =mnext;
    }

    //очищаем список имен файлов
    fileMenuFree();     
}
