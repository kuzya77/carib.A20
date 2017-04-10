#ifndef __INDICATOR_H__
#define __INDICATOR_H__

#include "font.h"
#include <string>

class Indicator : public AnimLayer
{
protected:
  	int   _align;
    Font*   _font;
  	std::string message;
public:
	Indicator(ParseNode* node, Font* font);
  	virtual void set(const char*);
    const char* get() const;
  	void draw();
};

// here is a hack:
// I chenge animation in draw procedure
// why: just to simplify this junk code
class DigitalIndicator: public Indicator
{
	int _mode;	// -1 dynamic descedent, 1 dynamic increment
				// 0 - both (inc+dec)
	int _current_value;
	int _final_value;
public:
	DigitalIndicator(ParseNode* node, Font* font, int mode);
  	void set(const char*);
  	void draw();
};

#endif
