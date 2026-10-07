/*!\file Logic_in.c
** \author SMFSW
** \copyright MIT (c) 2017-2026, SMFSW
** \brief Logic input handling
**/
/****************************************************************/
#include "Logic_in.h"

/****************************************************************/


void __attribute__((nonnull(1))) Logic_in_init(	Logic_in * const in,
									GPIO_PinState (*getter)(const Logic_in * const),
									uint32_t * const addr,
									const uint32_t pos,
									const GPIO_PinState polarity,
									const uint16_t filter,
									void (*onSet)(const Logic_in * const),
									void (*onReset)(const Logic_in * const),
									const bool repeat)
{
	in->cfg.get = getter;
	in->cfg.LOGx = addr;
	in->cfg.LOG_Pos = pos;
	in->cfg.polarity = polarity & GPIO_PIN_SET;	// Ensuring polarity is boolean like value
	in->cfg.filt = filter;
	in->cfg.onSet = onSet;
	in->cfg.onReset = onReset;
	in->cfg.repeat = repeat;
	in->in = false;
	in->mem = false;
	in->edge = NoEdge;
	in->hIn = 0;
}


void __attribute__((nonnull)) Logic_in_handler(Logic_in * const in)
{
	GPIO_PinState val;

	if (in->cfg.get)				{ val = in->cfg.get(in); }
	else if (in->cfg.LOGx != NULL)	{ val = RSHIFT(*((uint32_t *) in->cfg.LOGx), in->cfg.LOG_Pos) & 1U; }
	else							{ val = in->cfg.polarity; }

	if (val == in->cfg.polarity)
	{
		if (TPSSUP_MS(in->hIn, in->cfg.filt))	{ in->in = true; }
	}
	else
	{
		in->in = false;
		in->hIn = millis();
	}

	if (in->in == in->mem)
	{
		in->edge = NoEdge;
		if ((in->in) && (in->cfg.repeat))
		{
			if (in->cfg.onSet != NULL)	{ in->cfg.onSet(in); }
		}
	}
	else if (in->in > in->mem)
	{
		in->edge = Rising;
		if (in->cfg.onSet != NULL)		{ in->cfg.onSet(in); }
	}
	else
	{
		in->edge = Falling;
		if (in->cfg.onReset != NULL)	{ in->cfg.onReset(in); }
	}

	in->mem = in->in;
}
