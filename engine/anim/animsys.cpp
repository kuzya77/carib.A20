#include <types.h>
#include <mem.h>
#include <log.h>

#include <stuff/menu.h>
#include <stuff/cmd.h>

#include "animsys.h"

AnimScene* AnimSys::GetNode(const char* name)
{
    if(!name)
        return NULL;
    u32_t id_crc=strhash(name);
    auto pr=scenes.equal_range(id_crc);
    return pr.first!=pr.second?pr.first->second:NULL;
}

bool AnimSys::select(const char* scene_name)
{
    AnimScene* scene=GetNode(scene_name);
    if(scene)
    {
        CurrentScene=scene;
        scene->OnSelect();
        return true;
    }
    return false;
}

bool AnimSys::load(const char* file)
{   
    AnimScene* scene=NULL;
    ParseNode* root=parLoadWXF(file,512,2048);;
    if(!root)
    {
        logError("AnimSys::load error: can't load \"%s\"",file);
        conPrintf("\n\tAnimSys::load error: can't load \"%s\"",file);
        return false;
    }
    try
    {
        logDebug("AnimSys load(\"%s\")",file);
        for(ParseNode* node=root->first("animscene");node;node=node->next("animscene"))
        {
            scene=new AnimScene(node);
            u32_t id_crc = strhash(scene->name());
            logDebug("load scene\"%s\"",scene->name());
            // Проверка на совпадения
            auto pr=scenes.equal_range(id_crc);
            if(pr.first!=pr.second)
            {
                logWarning("AnimSys::load warning: crc already exist for \"%s\" (%08X)",scene->name(),id_crc);
                conPrintf("\n\tAnimSys::load warning: crc already exist for \"%s\" (%08X), scene skipped",scene->name(),id_crc);
                delete scene;
                scene=NULL;
                continue;
            }
            scenes[id_crc]=scene;
        }
        for(ParseNode* node=root->first("include");node;node=node->next("include"))
            load(attrs(node->list(),0,""));
        parFree(root);
        return true;
    }
    catch(std::bad_alloc& ba)
    {
        logError("AnimSys::load error %s",ba.what());
    }
    catch(std::runtime_error& re)
    {
        logError("%s",re.what());
    }
    catch(...)
    {
        logError("AnimSys::load error: shit happens");
    }
    delete scene;
    parFree(root);
    return false;
}

AnimSys::~AnimSys()
{
    for(auto& i:scenes)
        delete i.second;    
}

static AnimSys* animSys=NULL;

bool animSelectScene(const char* name)
{
    if(animSys)
        return animSys->select(name);
    return false;
}

void animProcess()
{
    if(animSys)
        animSys->process();
}

void animDraw()
{
    if(animSys)
        animSys->draw();
}

void animReset()
{
    if(animSys)
        animSys->reset();
}

bool animExist(const char* name)
{
    return animSys?animSys->exist(name):false;
}

void animHide(const char* name)
{
    if(animSys)
        animSys->hide(name);
}

bool animShow(const char* name, int idx)
{
    return animSys && animSys->show(name, idx);
}

bool animPlay(const char* name)
{
    return animSys?animSys->play(name):false;
}

bool animMerge(const char* name, const char* prev_anim)
{
	return animSys && animSys->merge(name, prev_anim);
}

bool animBind(AnimLayer* usr)
{
    return usr && animSys && animSys->bind(usr);
}

bool animLoad(const char* file_name)
{
    if(animInit())
        return animSys->load(file_name);
    return false;
}

void animSetIndicator(const char* name, const char* value)
{	
	if(animSys)
		animSys->setValue(name, value);
}

const char* animGetIndicator(const char* name)
{
	if(!animSys)
		return NULL;
	return animSys->getValue(name);
}

bool animSetText(const char* layer, int id, int x, int y, const char* font, const char* text)
{
	if(!animSys || !animSys->CurrentScene)
		return false;
	return animSys->CurrentScene->setText(layer, id, x, y, font, text);
}

bool animSetPicture(const char* layer, int id, int x, int y, const char* anim_name, int idx)
{
	if(!animSys || !animSys->CurrentScene)
		return false;
	return animSys->CurrentScene->setPicture(layer, id, x, y, anim_name, idx);
}

void animRemove(const char* layer, int id)
{
	if(animSys && animSys->CurrentScene)
		animSys->CurrentScene->remove(layer, id);
}

void animClear(const char* layer)
{
	if(animSys && animSys->CurrentScene)
		animSys->CurrentScene->clear(layer);
}

void animMove(const char* layer, int id, int x, int y, float speed)
{
	if(animSys && animSys->CurrentScene)
		animSys->CurrentScene->move(layer, id, x, y, speed);	
}

//--------------------------------------------------------------------------------------------------------

static void animSelectSceneCmd(const AttrList& list)
{
    const char* s=list[0];
    if(!animSelectScene(s))
        conPrintf("Error: AnimSelectScene(\"%s\")",s);
}

static void animHideCmd(const AttrList& list)
{
    animHide(list[0]);
}

static void animShowCmd(const AttrList& list)
{
	int idx=0;
	switch(list.count())
	{
		case 0:
			conPrintf("Too few parameters, need Animation name\n");
			break;
		default:
		case 2:
			idx = list[1];
		case 1:
			if(!animShow(list[0], idx))
        		conPrintf("AnimShow \"%s\":%d failed\n", (const char*)list[0], idx);
   	}
}

static void animPlayCmd(const AttrList& list)
{
    const char* s=list[0];
    if(!animPlay(s))
        conPrintf("AnimPlay \"%s\" failed",s);
}

static void animResetSceneCmd(const AttrList&)
{
    animReset();
}

static void animLoadCmd(const AttrList& list)
{
    const char* s=list[0];
    if(!animLoad(s))
        conPrintf("Error: AnimLoad(\"%s\")\n",s);
}

static void animSetIndicator(const AttrList& list)
{
	const char* value="";
	switch(list.count())
	{
		case 0:
			conPrintf("Too few parameters, need indicator name\n");
			break;
		default:
		case 2:
			value = list[1];
		case 1:
			animSetIndicator(list[0], value);
   	}
}

static void animMergeCmd(const AttrList& list)
{
	switch(list.count())
	{		
		case 0:
			conPrintf("AnimMerge required 2 parameters\n");
			break;
		case 1:
			animPlayCmd(list);
			break;
		default:
			animMerge(list[0], list[1]);
			break;
	}
}

static void animSetTextCmd(const AttrList& list)
{
	if(list.count()!=6)
		conPrintf("Error. required 6 parameters\n");
	else
		animSetText(list[0], list[1], list[2], list[3], list[4], list[5]);
}

static void animSetPictureCmd(const AttrList& list)
{
	int idx=0;
	switch(list.count())
	{
		case 6:
			idx = list[5];
		case 5:
			break;
		default:
			conPrintf("Error. required 5 or 6 parameters\n");
			return;
	}
	animSetPicture(list[0], list[1], list[2], list[3], list[4], idx);
}

static void animRemoveCmd(const AttrList& list)
{
	if(list.count()!=2)
		conPrintf("Error. required 2 parameters\n");
	else
		animRemove(list[0], list[1]);
}

static void animClearCmd(const AttrList& list)
{
	if(list.count()!=1)
		conPrintf("Error. required 1 parameter\n");
	else
		animClear(list[0]);
}

static void animMoveCmd(const AttrList& list)
{
	if(list.count()!=5)
		conPrintf("Error, needs 5 parameters\n");
	else
		animMove(list[0], list[1], list[2], list[3], list[4]);
}
bool animInit()
{
    if(!animSys)
    {
        try
        {
            animSys=new AnimSys;
            cmdAddCmd("AnimSelectScene",animSelectSceneCmd);
            cmdAddCmd("AnimHide",animHideCmd);
            cmdAddCmd("AnimShow",animShowCmd);
            cmdAddCmd("AnimPlay",animPlayCmd);
            cmdAddCmd("AnimMerge",animMergeCmd);
            cmdAddCmd("AnimResetScene",animResetSceneCmd);
            cmdAddCmd("AnimLoad",animLoadCmd);
            cmdAddCmd("AnimSetIndicator", animSetIndicator);
            cmdAddCmd("AnimSetText", animSetTextCmd);
            cmdAddCmd("AnimSetPicture", animSetPictureCmd);
            cmdAddCmd("AnimRemove", animRemoveCmd);
            cmdAddCmd("AnimClear", animClearCmd);
            cmdAddCmd("AnimMove", animMoveCmd);           

            // register cmds
            return true;
        }
        catch(std::bad_alloc& ba)
        {
            logError("animInit error: %s",ba.what());
        }
        catch(...)
        {
            logError("animInit error: shit happens");
        }
        animDone();
        return false;
    }
    return true;
}

void animDone()
{
    delete animSys;
    animSys=NULL;    
}
