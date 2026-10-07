
/*!\file Logic_in.h
** \author SMFSW
** \copyright MIT (c) 2017-2026, SMFSW
** \brief Logic input handling
**/
/****************************************************************/
#ifndef LOGIC_IN_H__
	#define LOGIC_IN_H__

#include "stdint.h"
#include "Arduino.h"


#ifdef __cplusplus
	extern "C" {
#endif

/****************************************************************/
//#include "sarmfsw.h"


#define LSHIFT(v, n)			((v) << (n))					//!< Shift \p v \p n bits left
#define RSHIFT(v, n)			((v) >> (n))					//!< Shift \p v \p n bits right

#define LSHIFT8(v, n)			((BYTE) ((BYTE) (v) << (n)))	//!< Shift \p v \p n bits left (up to 7b)
#define RSHIFT8(v, n)			((BYTE) ((BYTE) (v) >> (n)))	//!< Shift \p v \p n bits right (up to 7b)

#define LSHIFT16(v, n)			((WORD) ((WORD) (v) << (n)))	//!< Shift \p v \p n bits left (up to 15b)
#define RSHIFT16(v, n)			((WORD) ((WORD) (v) >> (n)))	//!< Shift \p v \p n bits right (up to 15b)

#define LSHIFT32(v, n)			((DWORD) ((DWORD) (v) << (n)))	//!< Shift \p v \p n bits left (up to 31b)
#define RSHIFT32(v, n)			((DWORD) ((DWORD) (v) >> (n)))	//!< Shift \p v \p n bits right (up to 31b)

#define LSHIFT64(v, n)			((LWORD) ((LWORD) (v) << (n)))	//!< Shift \p v \p n bits left (up to 63b)
#define RSHIFT64(v, n)			((LWORD) ((LWORD) (v) >> (n)))	//!< Shift \p v \p n bits right (up to 63b)

/*!\enum eEdge
** \brief Signal Edges
**/
typedef enum {
	NoEdge = 0,	//!< No change
	Rising,		//!< Rising edge
	Falling		//!< Falling edge
} eEdge;

typedef enum
{
	GPIO_PIN_RESET  = LOW,
	GPIO_PIN_SET  = HIGH,
}GPIO_PinState;




// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\struct Logic_in
** \brief Logic input structure
**/
typedef struct logic_in {
	bool			in;											//!< Input value
	eEdge			edge;										//!< Input edge
	/*pvt*/
	bool			mem;										//!< Memo value
	uint32_t		hIn;										//!< Filter time
	struct {
	void			(*onSet)(const struct logic_in * const);	//!< Push callback ON function pointer
	void			(*onReset)(const struct logic_in * const);	//!< Push callback OFF function pointer
	GPIO_PinState	(*get)(const struct logic_in * const);		//!< Getter function
	void *			LOGx;
	uint16_t		LOG_Pos;									//!< Monitored bit position in variable
	uint16_t		filt;										//!< Filter time (ms)
	GPIO_PinState	polarity;									//!< Input polarity
	bool			repeat;										//!< Callback ON repeat
	} cfg;
} Logic_in;





static inline __attribute__((always_inline)) bool __attribute__((nonnull, always_inline)) TPSSUP_MS(const uint32_t start_tick, const uint32_t lapse)
{
	const uint32_t scaled_time = lapse * 1U;
	const uint32_t time_diff = millis() - start_tick;	// Underflow computation will give the same result for unsigned type

	return (bool)(time_diff >= scaled_time);
}


/*!\brief Tests if stored time value has not reached time lapse in ms
** \warning For SAM families, no ms base time counter is implemented in HAL,
**			please refer to arm_chip_sam.h for an implementation example.
** \note	Define custom \c HAL_MS_TICKS_FACTOR at project level if tick period is not 1ms
** \param[in] start_tick - previously stored time value
** \param[in] lapse - time lapse (in ms)
** \return true if time not elapsed
**/
static inline __attribute__((always_inline)) bool __attribute__((nonnull, always_inline)) TPSINF_MS(const uint32_t start_tick, const uint32_t lapse)
{
	const uint32_t scaled_time = lapse * 1U;
	const uint32_t time_diff = millis() - start_tick;	// Underflow computation will give the same result for unsigned type

	return (bool)(time_diff < scaled_time);
}



// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief Get Logic_in input value
** \param[in] in - input instance
** \return Input value
**/
static inline __attribute__((always_inline)) bool __attribute__((nonnull, always_inline)) get_Logic_in(const Logic_in * const in) {
	return in->in; }


/*!\brief Get Logic_in input edge
** \param[in] in - input instance
** \return Input edge
**/
static inline __attribute__((always_inline)) eEdge __attribute__((nonnull, always_inline)) get_Logic_in_edge(const Logic_in * const in) {
	return in->edge; }


/*!\brief Initialize Logic_in instance
** \param[in,out] in - input instance to initialize
** \param[in] getter - Pointer to variable getter function (may be NULL: default behavior for handling RAM variable at address \b addr)
** \param[in] addr - Variable address to read from (pointer to unsigned 32b, may be NULL if getter handles everything)
** \param[in] pos - monitored bit position (may be unused if getter function is used)
** \param[in] polarity - active state
** \param[in] filter - input filtering time
** \param[in] onSet - Pointer to callback ON function
** \param[in] onReset - Pointer to callback OFF function
** \param[in] repeat - To repeat callback ON as long as input is set
**/
void __attribute__((nonnull(1))) Logic_in_init(	Logic_in * const in,
									GPIO_PinState (*getter)(const Logic_in * const),
									uint32_t * const addr,
									const uint32_t pos,
									const GPIO_PinState polarity,
									const uint16_t filter,
									void (*onSet)(const Logic_in * const),
									void (*onReset)(const Logic_in * const),
									const bool repeat);


/*!\brief Handles Logic_in read and treatment
** \param[in,out] in - input instance to handle
**/
void __attribute__((nonnull)) Logic_in_handler(Logic_in * const in);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* LOGIC_IN_H__ */
/****************************************************************/