#ifndef __ANIMSCENE_H__
#define __ANIMSCENE_H__

#include <types.h>
#include <mem.h>

#include "indicator.h"
#include "static_layer.h"

#include <list>

class ImageList;
class MoveController;

class AnimScene:public NamedObj
{
    struct AnimNode;
    typedef std::list<AnimNode*> ActiveList;

    struct AnimNode
    {
        bool    IsOwner;
        AnimLayer* node;
        ActiveList::iterator active_node;   // ==end() if not active
        bool    IsDraw;
        AnimNode():node(NULL),IsDraw(false) {}
    };

    typedef std::unordered_map<u32_t,AnimNode> 		AnimList; // список всех анимаций сцены
    typedef std::unordered_map<u32_t, Font*>   		FontMap;
    typedef std::unordered_map<u32_t, Indicator*>	IndMap;
    typedef std::unordered_map<u32_t, StaticLayer*>	SlMap;
    typedef std::vector<AnimNode*>         		DrawList;
    typedef std::vector<MoveController>			MoveList;

    FontMap	 	_fonts;
    AnimList 	anims;
    IndMap		_indicators;
    SlMap		_statics;
    MoveList	_movelist;


    bool       ActiveListChanged;               // Проверка на изменение состояния списка активных анимаций
    ActiveList active_list;                     // список активных

    bool       DrawListChanged;                 // Проверка на изменение состояния списка активных анимаций
    DrawList   draw_list;                       // отсортированный список отображаемых анимаций
    
    bool       PaletteChanged;
    IMAGE      bkground;
    IMAGE      bkground2;						// 2nd screen bkground

    static bool CmpLessAnim(const AnimNode*,const AnimNode*);
    AnimNode* 	GetNode(const char* name);
    Font* 	  	GetFont(const char* name);
    ImageList* 	GetImageList(const char* animation);

public:    
    AnimScene(ParseNode*);                      // Создание сцены по узлу
    ~AnimScene();

    void process();                             // Обработка сцены

    void draw();                                // отрисовка сцены
    void hide(const char*);                     // скрыть анимацию по имени
    bool show(const char*, int);    
    bool play(const char*);
    void move(const char* layer, int id, int x, int y, float speed);
    bool merge(const char*,const char*);
    void reset();                               // Сброс всех активных или видимых анимаций

    void setValue(const char* name, const char* value);
    const char* getValue(const char* name);

	bool setText(const char* layer, int id, int x, int y, const char* font, const char* text);
	bool setPicture(const char* layer, int id, int x, int y, const char* anim_name, int idx);
	void remove(const char* layer, int id);
	void clear(const char* layer);

    bool bind(AnimLayer* usr);

    bool exist(const char* name)    { return GetNode(name)!=NULL; }

    void OnSelect();                            // Действия сцены при её выборе
};

#endif
