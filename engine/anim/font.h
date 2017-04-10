#ifndef __ANIM_FONT_H__
#define __ANIM_FONT_H__

#include <mem.h>
#include <stuff/anim.h>

#include <unordered_map>

class Font: public NamedObj
{
protected:
	int	_default_width;
public:
	Font(ParseNode* node);
	virtual ~Font();

	virtual void draw(int x, int y, const char* text) const = 0;
	virtual int  width(const char* text) const = 0;
};

//---------------------------------------------------------

class FontS : public Font
{
    struct CharInfo
    {
    	int offset;
    	int w;	
    };
    typedef std::unordered_map<int, CharInfo> SymbolMap;

	IMAGE 		_image;
	SymbolMap	_symbols;

public:
	FontS(ParseNode* node);

	~FontS() { imgFree(_image); }

	void draw(int x, int y, const char* text) const;
	int  width(const char* text) const;
};

//---------------------------------------------------------

class FontM : public Font
{
    typedef std::unordered_map<int, IMAGE> SymbolMap;

	SymbolMap	_symbols;
public:
	FontM(ParseNode* node);
	~FontM();

	void draw(int x, int y, const char* text) const;
	int  width(const char* text) const;
};

#endif
