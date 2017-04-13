#include <types.h>
#include <mem.h>
#include <log.h>

#include "animscene.h"
#include <stuff/cmd.h>
#include "i_anim.h"
#include "move_controller.h"

AnimScene::AnimNode* AnimScene::GetNode(const char* name)
{
    if(!name)
        return NULL;
    u32_t id_crc=strhash(name);
    auto pr=anims.equal_range(id_crc);
    return pr.first!=pr.second?&pr.first->second:NULL;
}

Font* AnimScene::GetFont(const char* name)
{
	auto it = _fonts.find(strhash(name));
    return it==_fonts.end()?NULL:it->second;
}

AnimScene::AnimScene(ParseNode* root):NamedObj(root),
                        ActiveListChanged(false),
                        DrawListChanged(false),
                        PaletteChanged(false),
                        bkground(NULL),
                        bkground2(NULL)
{
    Animation* anim= NULL;
   	Font* fnt	   = NULL;
   	Indicator* ind = NULL;
    try
    {
        logDebug("Load anims");

        const char* bk_name=attrs(root,"bkground");
        if(bk_name)
        {
            bkground = imgLoad(bk_name, gfxBpp(), IMG_SHARED);
            if(!bkground)
            {
                conPrintf("AnimScene::AnimScene(\"%s\") error: can't load bkground \"%s\"\n",name(),bk_name);
                logWarning("AnimScene::AnimScene(\"%s\") error: can't load bkground \"%s\"\n",name(),bk_name);

                throw std::runtime_error("AnimScene: bad bkground");
            }
        }
        const char* bk_name2=attrs(root,"bkground2");
        if(bk_name2)
        {
            bkground2 = imgLoad(bk_name2, gfxBpp(), IMG_SHARED);
            if(!bkground2)
            {
                conPrintf("AnimScene::AnimScene(\"%s\") error: can't load bkground2 \"%s\"\n",name(), bk_name2);
                logWarning("AnimScene::AnimScene(\"%s\") error: can't load bkground2 \"%s\"\n",name(), bk_name2);

                throw std::runtime_error("AnimScene: bad bkground2");
            }
        }

        // download fonts
        for(ParseNode* p = root->first("font");p;p=p->next("font"))
	    {
	    	if(attrs(p,"image", NULL))
	    		fnt = new FontS(p);
	    	else
	    		fnt = new FontM(p);

	    	_fonts[strhash(fnt->name())] = fnt;
	    	fnt = NULL;
		}

        for(ParseNode* node=root->first("animation");node;node=node->next("animation"))
        {
            anim=new Animation(node);
            u32_t id_crc=strhash(anim->name());
            // Проверка на совпадения
            auto pr=anims.equal_range(id_crc);
            if(pr.first!=pr.second)
            {
                conPrintf("AnimScene::AnimScene warning: crc already exist for \"%s\" (%08X)\n",anim->name(),id_crc);
                logWarning("AnimScene::AnimScene warning: crc already exist for \"%s\" (%08X)",anim->name(),id_crc);
            }
            AnimNode anode;
            anode.node=anim;
            anode.IsOwner=true;
            anode.active_node=active_list.end();
            anims[id_crc]=anode;
        }

        for(ParseNode* node=root->first("indicator");node;node=node->next("indicator"))
        {
        	const char* font = attrs(node, "font");
        	Font* fnt = GetFont(font);
        	if(!fnt)
        	{
        		conPrintf("AnimScene::AnimScene indicator: wrong font \"%s\" in indicator \"%s\"\n", font, attrs(node->list(),0));
        		logWarning("AnimScene::AnimScene indicator: wrong font \"%s\" in indicator \"%s\"\n", font, attrs(node->list(),0));
        		throw std::runtime_error("AnimScene::AnimScene font absent");
        	}
        	int mode = attri(node, "mode", 0xFF);
        	ind = mode==0xFF?new Indicator(node, fnt):new DigitalIndicator(node, fnt, mode);
        	u32_t id_crc=strhash(ind->name());

        	AnimNode anode;
            anode.node			= ind;
            anode.IsOwner		= true;
            anode.active_node	= active_list.end();
            anims[id_crc]		= anode;
            _indicators[id_crc] = ind;
        }

        for(ParseNode* node=root->first("static");node;node=node->next("static"))
        {
        	StaticLayer* sl = new StaticLayer(node);
        	u32_t id_crc=strhash(sl->name());

        	AnimNode anode;
            anode.node			= sl;
            anode.IsOwner		= true;
            anode.active_node	= active_list.end();
            anims[id_crc]		= anode;
            _statics[id_crc] 	= sl;
        }
    }
    catch(...)
    {
        delete anim;
        delete fnt;
        delete ind;
        throw;
    }
}

void AnimScene::process()
{   
    if(ActiveListChanged)   // Список активных задач изменился?
    {
        ActiveList::iterator prev=active_list.end();
        for(ActiveList::iterator i=active_list.begin();i!=active_list.end();i++)
        {
            if((*i)->active_node==active_list.end())   // Удалить из списка активных
            {
                active_list.erase(i);
                i=prev==active_list.end()?active_list.begin():prev;
            }
            prev=i;
        }        
        ActiveListChanged=false;
    }

    u32_t t=MilliSecs();
    // Отработка анимаций
    for(ActiveList::iterator i=active_list.begin();i!=active_list.end();i++)
        if(!(*i)->node->process(t))
        {
            (*i)->active_node=active_list.end();
            ActiveListChanged=true;
        }

    for(size_t i=0; i<_movelist.size();i++)
    	if(!_movelist[i].process(t))
        {
        	_movelist.erase(_movelist.begin()+i);
            i--;
        }
}

bool AnimScene::CmpLessAnim(const AnimNode* n1,const AnimNode* n2)
{
    return n1->node->layer()<n2->node->layer();
}

void AnimScene::draw()
{
    if(DrawListChanged)
    {
        // Удаление недействительных элементов        
        for(DrawList::iterator i=draw_list.end();i!=draw_list.begin();)
        {
            i--;
            if(!(*i)->IsDraw)   // Удалить из списка рисуемых
                draw_list.erase(i);
        }        
        // Сортируем выживших
        sort(draw_list.begin(),draw_list.end(), CmpLessAnim);
        DrawListChanged=false;
    }

    gfxPutImage(0, 0, bkground);
    if(bkground2)
    {
	    gfxSet2ndScreen(true);
    	gfxPutImage(0, 0, bkground2);
    	gfxSet2ndScreen(false);
    }

    for(DrawList::iterator i=draw_list.begin();i!=draw_list.end();i++)
        (*i)->node->draw();
    if(PaletteChanged)
    {
        gfxPutPalette(imgPalette(bkground));
        PaletteChanged=false;
    }
}

void AnimScene::hide(const char* name)
{    
    AnimNode* p=GetNode(name);
    if(p && p->IsDraw)
    {
        p->active_node=active_list.end();
        p->IsDraw=false;
        DrawListChanged=true;
        ActiveListChanged=true;
    }
}

bool AnimScene::show(const char* name, int idx)
{
    AnimNode* p=GetNode(name);
    if(!p)    
        return false;
    if(!p->IsDraw)
    {
        p->IsDraw=true;
        draw_list.push_back(p);
        DrawListChanged=true;
        p->node->reset();
        p->node->setCFrame(idx);
    }
    return true;
}

ImageList* AnimScene::GetImageList(const char* animation)
{
    AnimNode* p=GetNode(animation);
    if(!p)    
        return NULL;
    return ImageList::load(p->node->getRcName(), 0);
}

bool AnimScene::play(const char* name)
{
    AnimNode* p=GetNode(name);
    if(!p)
        return false;
    if(!p->IsDraw)
    {
        p->IsDraw=true;
        draw_list.push_back(p);
        DrawListChanged=true;
    }
    if(p->active_node==active_list.end())
    {
        active_list.push_back(p);
        p->active_node=active_list.end();
        p->active_node--;
//            ActiveListChanged=true;
    }
    p->node->start(MilliSecs());
    return true;
}

bool AnimScene::merge(const char* name, const char* prev)
{
    AnimNode* p=GetNode(name);
    if(!p)
        return false;
    if(!p->IsDraw)
    {
        p->IsDraw=true;
        draw_list.push_back(p);
        DrawListChanged=true;
    }
    if(p->active_node==active_list.end())
    {
        active_list.push_back(p);
        p->active_node=active_list.end();
        p->active_node--;
//            ActiveListChanged=true;
    }
    AnimNode* pnode = GetNode(prev);
    if(pnode && pnode->active_node!=active_list.end())
    {
    	p->node->start(pnode->node->getStart());
		p->node->setCFrame(pnode->node->getCFrame());
    }
    else
	    p->node->start(MilliSecs());
    return true;
}

void AnimScene::reset()
{
    for(ActiveList::iterator i=active_list.begin();i!=active_list.end();i++)
        (*i)->active_node=active_list.end();
    active_list.clear();
    for(auto& i:draw_list)
        i->IsDraw=false;
    draw_list.clear();
}

void AnimScene::OnSelect()
{
    if(imgPalette(bkground))
        PaletteChanged=true;
}

AnimScene::~AnimScene()
{
    imgFree(bkground);
    for(auto& i:anims)
        if(i.second.IsOwner)
            delete i.second.node;

	for(auto& it:_fonts)
		delete it.second;
}

bool AnimScene::bind(AnimLayer* usr)
{
    u32_t id_crc=strhash(usr->name());
    // Проверка на совпадения
    auto pr=anims.equal_range(id_crc);
    if(pr.first!=pr.second)
    {
        logWarning("AnimScene::AnimScene warning: crc already exist for \"%s\" (%08X)",usr->name(),id_crc);
        return false;
    }
    AnimNode anode;
    anode.node=usr;
    anode.IsOwner=false;
    anode.active_node=active_list.end();
    anims[id_crc]=anode;
    return true;
}

void AnimScene::setValue(const char* name, const char* value)
{
	IndMap::iterator it = _indicators.find(strhash(name));
	if(it!=_indicators.end())
		it->second->set(value);
}

const char* AnimScene::getValue(const char* name)
{
	IndMap::iterator it = _indicators.find(strhash(name));
	return it!=_indicators.end()?it->second->get():NULL;
}

bool AnimScene::setText(const char* layer, int id, int x, int y, const char* font, const char* text)
{
	SlMap::iterator it = _statics.find(strhash(layer));
	if(it==_statics.end())
		return false;
   	Font* fnt = GetFont(font);
    return fnt && it->second->create(id, x, y, fnt, text);
}

bool AnimScene::setPicture(const char* layer, int id, int x, int y, const char* anim_name, int frame)
{
	SlMap::iterator it = _statics.find(strhash(layer));
	if(it==_statics.end())
		return false;

	ImageList* il = GetImageList(anim_name);
	if(!il)
		return false;
    
    if(frame<0)
		frame = il->count()-frame;
	IMAGE img = frame>=0 && frame<il->count()?il->get(frame):NULL;
	bool transparent = il->isTransparent();
	ImageList::destroy(il);
	return img && it->second->create(id, x, y, img, transparent);
}

void AnimScene::move(const char* layer, int id, int x, int y, float speed)
{
	SlMap::iterator it = _statics.find(strhash(layer));
	if(it==_statics.end())
		return;
	_movelist.push_back(MoveController(it->second, id, x, y, speed));
}

void AnimScene::remove(const char* layer, int id)
{
	SlMap::iterator it = _statics.find(strhash(layer));
	if(it!=_statics.end())
		it->second->remove(id);
}

void AnimScene::clear(const char* layer)
{
	SlMap::iterator it = _statics.find(strhash(layer));
	if(it!=_statics.end())
		it->second->clear();
}
