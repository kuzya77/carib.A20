#include "indicator.h"

Indicator::Indicator(ParseNode* node, Font* font):AnimLayer(node),_align(1)
{
	_font = font;
	const AttrList& plist=attrlist(node, "pos");
	switch(plist.count())
	{
		default:
		case 2:
			pos.y = plist[1];
		case 1:
			pos.x = plist[0];
		case 0:;
	}
	const AttrList& align=attrlist(node, "align");
	if(align.count()>0)
		_align = align[0];
}

void Indicator::set(const char *s)
{
	message = s;
}

const char* Indicator::get() const
{
      return message.c_str();
}

void Indicator::draw()
{
	int x = pos.x;
	switch(_align)
	{
		case 0:
			x -= _font->width(message.c_str())/2;
			break;
		case -1:
			x -= _font->width(message.c_str());
			break;
	}
	_font->draw(x, pos.y, message.c_str());
}

DigitalIndicator::DigitalIndicator(ParseNode* node, Font* font, int mode)
	:Indicator(node, font), _mode(mode)
{
	_current_value = _final_value = 0;
	message = "0";
}

void DigitalIndicator::set(const char* value)
{
	_final_value = atoi(value);
	if(_final_value == _current_value)
		return;
    if((_mode==-1 && _final_value>_current_value) ||
    	(_mode==1 && _final_value<_current_value))
    {
		message = value;
		_current_value = _final_value;
	}
}

void DigitalIndicator::draw()
{
	const int values[] = {1000, 200, 20 };
	const int incs[] = {500, 100, 10};
	const int count = 3;
	
	if(_final_value!=_current_value)
	{
		int inc = 1;
		if(_final_value>_current_value)
		{
			for(int i=0; i<count; i++)
				if(_final_value-_current_value>values[i])
				{
					inc = incs[i];
					break;
				}
		}
		else 
		{
			for(int i=0; i<count; i++)
				if(_current_value-_final_value>values[i])
				{
					inc = incs[i];
					break;
				}
			inc = -inc;

		}
		_current_value += inc;
		char buff[50];
		sprintf(buff, "%d", _current_value);
		message = buff;
	}
	Indicator::draw();
}
