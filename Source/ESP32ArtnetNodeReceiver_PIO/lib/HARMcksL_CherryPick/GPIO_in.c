
/*!\file GPIO_in.c
** \author SMFSW
** \copyright MIT (c) 2017-2026, SMFSW
** \brief GPIO input handling
**/
/****************************************************************/
//#include "sarmfsw.h"
#include "GPIO_in.h"

/****************************************************************/


/*!\brief Get GPIO port value
** \param[in,out] in - GPIO_in instance
** \return GPIO port value
**/
static inline __attribute__((always_inline)) GPIO_PinState __attribute__((nonnull, always_inline)) GPIO_getter(GPIO_in * const in)
{
	return digitalRead(in->cfg.LOG_Pos);
}


void __attribute__((nonnull(1))) GPIO_in_init(	GPIO_in * const in,
									const uint16_t GPIO_Pin,
									const GPIO_PinState polarity,
									const uint16_t filter,
									void (*onSet)(const GPIO_in * const),
									void (*onReset)(const GPIO_in * const),
									const bool repeat)
{
	Logic_in_init(	in,
					(GPIO_PinState (*)(const Logic_in * const)) GPIO_getter,
					NULL,
					GPIO_Pin,
					polarity,
					filter,
					(void (*)(const Logic_in * const)) onSet,
					(void (*)(const Logic_in * const)) onReset,
					repeat);
}


/********************************************/