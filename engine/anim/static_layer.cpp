#include "static_layer.h"

#include "font.h"

#include <string>

slElement::slElement(int id, int x, int y): _id(id), _x(x), _y(y) {}
slElement::~slElement() {}

//-----------------------------------------------------------------------------

class slText: public slElement
{
	Font* 	_fnt;
	std::string  _str;
public:
	slText(int id, int x, int y, Font* font, const char* text);
	void draw();
};

slText::slText(int id, int x, int y, Font* font, const char* text)
		:slElement(id, x, y), _fnt(font), _str(text)
{
}

void slText::draw()
{
	_fnt->draw(_x, _y, _str.c_str());
}

//-----------------------------------------------------------------------------

class slImage: public slElement
{
protected:
	IMAGE 	_img;
public:
	slImage(int id, int x, int y, IMAGE img);
	void draw();
};

slImage::slImage(int id, int x, int y, IMAGE img)
		:slElement(id, x, y), _img(img)
{
}

void slImage::draw()
{
	gfxPutImage(_x, _y, _img);
}


class slAImage: public slImage
{
public:
	slAImage(int id, int x, int y, IMAGE img);
	void draw();
};

slAImage::slAImage(int id, int x, int y, IMAGE img)
		:slImage(id, x, y, img)
{
}

void slAImage::draw()
{
	gfxPutImageA(_x, _y, _img);
}

//-----------------------------------------------------------------------------

StaticLayer::StaticLayer(ParseNode* node):AnimLayer(node) {}

StaticLayer::~StaticLayer()
{
	clear();
}

void StaticLayer::draw()
{
	for(auto it:_collection)
		it->draw();
}

void StaticLayer::clear()
{
	for(auto it:_collection)
		delete it;
	_collection.clear();
}

StaticLayer::Collection::iterator StaticLayer::find(int id)
{
	for(Collection::iterator it=_collection.begin(); it!=_collection.end(); it++)
		if((*it)->getid()==id)
			return it;
	return _collection.end();
}

void StaticLayer::insert_or_replace(int id, slElement* p)
{
	auto it = find(id);
	if(it==_collection.end())
		_collection.push_back(p);
	else
		*it = p;
}

bool StaticLayer::create(int id, int x, int y, Font* font, const char* text)
{
	slElement* p = new slText(id, x, y, font, text);	
	insert_or_replace(id, p);
	return true;
}

bool StaticLayer::create(int id, int x, int y, IMAGE img, bool transparent)
{
	slElement* p = transparent?new slAImage(id, x, y, img):new slImage(id, x, y, img);
	insert_or_replace(id, p);
	return true;
}

void StaticLayer::remove(int id)
{
	Collection::iterator it = find(id);
	if(it!=_collection.end())
		_collection.erase(it);
}

slElement* StaticLayer::get(int id)
{
	auto it = find(id);
	if(it!=_collection.end())
		return *it;
	return NULL;
}
