#ifndef __STATIC_LAYER_H__
#define __STATIC_LAYER_H__

#include "font.h"

#include <vector> // must be hash_map, but it doesnt work yet

class slElement
{
	int _id;
protected:
	int _x, _y;
public:
	slElement(int id, int x, int y);
	virtual ~slElement();
	int getid() const { return _id; }
	tPoint getpos() const { tPoint t; t.x=_x; t.y=_y; return t; }
	void setpos(int x, int y) { _x=x; _y=y; }

	virtual void draw() = 0;
};

class StaticLayer: public AnimLayer
{
  typedef std::vector<slElement*> Collection;
  Collection _collection;

  Collection::iterator find(int id);
  void insert_or_replace(int id, slElement*);
public:
  StaticLayer(ParseNode* node);
  ~StaticLayer();
  void draw();

  bool create(int id, int x, int y, Font* font, const char* text);
  bool create(int id, int x, int y, IMAGE img, bool transparent);
  slElement* get(int id);
  void remove(int id);
  void clear();
};

#endif
