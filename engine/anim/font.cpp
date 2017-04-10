#include <mem.h>
#include <log.h>

#include "font.h"

Font::Font(ParseNode* node):NamedObj(node)
{
	_default_width = attri(node, "default_width");
}

Font::~Font() {}

//--------------------------------------------------------

FontS::FontS(ParseNode* node):Font(node)
{
	const char* path = 	attrs(node,"image");
	_image = imgLoad(path, gfxBpp(), IMG_SHARED|IMG_NOPALETTE);
	if(!_image)
		throw std::runtime_error("Font load error: bad image");

	const char* symb_str = attrs(node, "symbols", "");

	const AttrList& plist=attrlist(node, "offsets");
   	
   	if(!plist || plist.count()!=strlen(symb_str))
   	{
   		logError("Font \"%s\" load error: absent offsets or wrong symbols amount", name());
   		throw std::runtime_error("Font load error");
	}
	for(int i=0; symb_str[i]; i++)
	{
		CharInfo ci;
		ci.offset = plist[i];
		if(i+1<plist.count())
			ci.w = plist[i+1];
		else
			ci.w = imgWidth(_image);
		ci.w -= ci.offset;
		_symbols.insert({symb_str[i], ci});
	}
}

void FontS::draw(int x, int y, const char* text) const
{
	const int ybottom = y+imgHeight(_image);
	for(;*text;text++)
	{
		SymbolMap::const_iterator it = _symbols.find(*text);
		if(it==_symbols.end())
			x += _default_width;
		else
		{
			const CharInfo& ci = it->second;
            cRect rect(x, y, x+ci.w-1, ybottom);
            gfxPutImageA(x-ci.offset, y, _image, &rect);

            x += ci.w;
		}
	}
}

int FontS::width(const char* text) const
{
	int w = 0;
	for(;*text;text++)
	{
		SymbolMap::const_iterator it = _symbols.find(*text);
		if(it==_symbols.end())
			w += _default_width;
		else
            w += it->second.w;
	}
	return w;
}

//--------------------------------------------------------

FontM::FontM(ParseNode* node):Font(node)
{
	const char* symb_str = attrs(node, "symbols", "");

	const AttrList& plist=attrlist(node, "images");

   	if(!plist || plist.count()!=strlen(symb_str))
   	{
   		logError("FontM \"%s\" load error: absent pictures or wrong symbols amount", name());
   		throw std::runtime_error("Font load error");
	}

	for(int i=0; symb_str[i]; i++)
	{
		IMAGE img=imgLoad(plist[i]);
		if(!img)
		{
			logError("FontM::FontM bad image \"%s\"", (const char*)plist[i]);
			throw std::runtime_error("Font load error: bad image");
		}	
		_symbols.insert({symb_str[i], img});
	}
}

FontM::~FontM() 
{ 
	for(SymbolMap::iterator it=_symbols.begin(); it!=_symbols.end(); it++)
		imgFree(it->second);
}

void FontM::draw(int x, int y, const char* text) const
{
	for(;*text;text++)
	{
		auto it = _symbols.find(*text);
		if(it==_symbols.end())
			x += _default_width;
		else
		{
			IMAGE img = it->second;
            gfxPutImageA(x, y, img);

            x += imgWidth(img);
		}
	}
}

int  FontM::width(const char* text) const
{
	int w=0;
	for(;*text;text++)
	{
		auto it = _symbols.find(*text);
		w += it==_symbols.end()?_default_width:imgWidth(it->second);
	}
	return w;
}
