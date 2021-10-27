#include <nds32_intrinsic.h>
#include "irqn.h"
#include "type.h"

#ifndef NDS32_SR_PFT_CTL
#define NDS32_SR_PFT_CTL 0x210
#endif

inline void GIE_ENABLE(void)
{
	__nds32__setgie_en();
}

inline void GIE_DISABLE(void)
{
	__nds32__setgie_dis();
	__nds32__dsb();
}

inline bool GIE_STATE_GET(void)
{
	return ((__nds32__mfsr(NDS32_SR_PSW) & 0x01) == 0x01) ? TRUE : FALSE;
}

void InterruptLevelSet(uint8_t Level)
{
	uint32_t psw;
	psw = __nds32__mfsr(NDS32_SR_PSW)&(~(7<<16));
	__nds32__mtsr(psw|(Level<<16), NDS32_SR_PSW);
}

void InterruptLevelRestore(void)
{
	uint32_t psw;
	psw = __nds32__mfsr(NDS32_SR_PSW);
	__nds32__mtsr(psw|(7<<16), NDS32_SR_PSW);
}
void Throttle_Enable(void)
{
	uint32_t psw;
	psw = __nds32__mfsr(NDS32_SR_PSW);
	psw |= (1<<21);
	__nds32__mtsr(psw, NDS32_SR_PSW);
}

void Throttle_Disable(void)
{
	uint32_t psw;
	psw = __nds32__mfsr(NDS32_SR_PSW);
	psw &= ~(1<<21);
	__nds32__mtsr(psw, NDS32_SR_PSW);
}

void Throttle_Setup(uint32_t setting)
{
	uint32_t pft_ctl;
	pft_ctl = __nds32__mfsr(NDS32_SR_PFT_CTL);
	pft_ctl = (pft_ctl&~0x1f0)|setting;
	__nds32__mtsr(pft_ctl, NDS32_SR_PFT_CTL);
}


void NVIC_EnableIRQ(IRQn_Type IRQn)
{
	uint32_t int_mask;

	if((IRQn>=0)&&(IRQn<32))
	 {
	  int_mask = __nds32__mfsr(NDS32_SR_INT_MASK2);
	  int_mask |= 1<<IRQn;
	  __nds32__mtsr(int_mask, NDS32_SR_INT_MASK2);
	 }
}

void NVIC_DisableIRQ(IRQn_Type IRQn)
{
	uint32_t int_mask;
	if((IRQn>=0)&&(IRQn<32))
	 {
	  int_mask = __nds32__mfsr(NDS32_SR_INT_MASK2);
	  int_mask &= ~(1<<IRQn);
	  __nds32__mtsr(int_mask, NDS32_SR_INT_MASK2);
	 }
}

//priority=0,1,2,3 0=highest 3=lowest
void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority)
{
	uint32_t prio_mask;

	if((IRQn >= 0) && (IRQn < 16)){
      prio_mask = __nds32__mfsr(NDS32_SR_INT_PRI);
      prio_mask &= ~(3<<(IRQn*2));
      prio_mask |= (priority&3)<<(IRQn*2);
      __nds32__mtsr(prio_mask, NDS32_SR_INT_PRI);
	}
	else if((IRQn >= 16) && (IRQn < 32)){
	  prio_mask = __nds32__mfsr(NDS32_SR_INT_PRI2);
	  prio_mask &= ~(3<<((IRQn-16)*2));
	  prio_mask |= (priority&3)<<((IRQn-16)*2);
	  __nds32__mtsr(prio_mask, NDS32_SR_INT_PRI2);
	}
}

uint32_t NVIC_GetPriority(IRQn_Type IRQn)
{
	uint32_t prio_mask;

	if((IRQn >= 0) && (IRQn < 16)){
      prio_mask = __nds32__mfsr(NDS32_SR_INT_PRI);
      prio_mask >>= (IRQn*2);
      return (prio_mask&3);
	}
	else if((IRQn >= 16) && (IRQn < 32)){
	  prio_mask = __nds32__mfsr(NDS32_SR_INT_PRI2);
	  prio_mask >>= ((IRQn-16)*2);
      return (prio_mask&3);
	}

	return -1;
}
