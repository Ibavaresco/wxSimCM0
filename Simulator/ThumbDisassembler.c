/*===============================================================================
wxSimCM0

Simulator for a generic ARM Cortex-M0 processor with a few onboard peripherals
based on a previous simulator for a fictitious processor also written by me
(<https://github.com/Ibavaresco/wxCesar>).

Written using Code::Blocks 20.03 and
wxFormBuilder v4.2.1 using the library wxWidgets v3.2.5
=================================================================================
Copyright (c) 2024-2025, Isaac Marino Bavaresco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Neither the name of the author nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.;;

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
==============================================================================*/
/*============================================================================*/
#include <setjmp.h>
#include <stdio.h>
#include <stdint.h>
#include "Thumb.h"
#include "ThumbDisassembler.h"
/*============================================================================*/
#if 1
#define INLINE static inline __attribute(( always_inline ))
#else
#define INLINE static
#endif
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10 09 08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 0| 0| 0| 0|-----Off5-----|---Rm---|---Rd---|	LSL1
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 0| 0| 0| 1|-----Off5-----|---Rs---|---Rd---|	LSR1
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 0| 0| 1| 0|-----Off5-----|---Rs---|---Rd---|  ASR1
 +--+--+--+--+--+--------------+--------+--------+
*/
INLINE int Shifts1( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "LSLS", "LSRS", "ASRS", "?1x" };
	unsigned int		Rd, Rm, Shift, Operation;

	Rd					= ( OpCode >>  0 ) & 0x07;
	Rm					= ( OpCode >>  3 ) & 0x07;
	Operation			= ( OpCode >> 11 ) & 0x03;
	/* Shift can be 0 to 31 */
	if( Operation == 0 )
		Shift			= ( OpCode >>  6 ) & 0x1f;
	/* Shift can be 1 to 32 */
	else
		Shift			= ((( OpCode >>  6 ) - 1 ) & 0x1f ) + 1;

	if( Operation == 0 && Shift == 0 )
		return sprintf( Text, "%08X:  %04X       MOVS   R%u,R%u", Address - 2, OpCode, Rd, Rm );

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,R%u,#%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Rm, Shift );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rn---|---Rd---|	ADD3
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 0| 1|---Rm---|---Rn---|---Rd---|  SUB3
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
INLINE int AddSub3( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "ADDS", "SUBS" };
	unsigned int		Rd, Rn, Rm, Operation;

	Rd					= ( OpCode >>  0 ) & 0x07;
	Rn					= ( OpCode >>  3 ) & 0x07;
	Rm					= ( OpCode >>  6 ) & 0x07;
	Operation			= ( OpCode >>  9 ) & 0x01;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,R%u,R%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Rn, Rm );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 1| 0|--Imm3--|---Rn---|---Rd---|	ADD1
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 0| 0| 1| 1| 1| 1|--Imm3--|---Rn---|---Rd---|	SUB1
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
INLINE int AddSub1( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "ADDS", "SUBS" };
	unsigned int		Rd, Rn, Immediate, Operation;

	Rd					= ( OpCode >>  0 ) & 0x07;
	Rn					= ( OpCode >>  3 ) & 0x07;
	Immediate			= ( OpCode >>  6 ) & 0x07;
	Operation			= ( OpCode >>  9 ) & 0x01;

	return sprintf( Text, "%08X:  %02X       %-5s  R%u,R%u,#%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Rn, Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 0| 0|---Rd---|---------Imm8----------|	MOV1
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 0| 1|---Rn---|---------Imm8----------|	CMP1
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 1| 0|---Rd---|---------Imm8----------|	ADD2
 +--+--+--+--+--+--------+-----------------------+
 | 0| 0| 1| 1| 1|---Rd---|---------Imm8----------|	SUB2
 +--+--+--+--+--+--------+-----------------------+
*/
INLINE int MovCmp1AddSub2( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "MOVS", "CMP", "ADDS", "SUBS" };
	unsigned int		Rd, Immediate, Operation;

	Rd					= ( OpCode >>  8 ) & 0x07;
	Immediate			= ( OpCode >>  0 ) & 0xff;
	Operation			= ( OpCode >> 11 ) & 0x03;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,#%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 0| 0|---Rm---|---Rd---|	AND
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 0| 1|---Rm---|---Rd---|	EOR
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 1| 0|---Rs---|---Rd---|	LSL2
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 0| 1| 1|---Rm---|---Rd---|	LSR2
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 0| 0|---Rs---|---Rd---|	ASR2
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 0| 1|---Rm---|---Rd---|	ADC
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 1| 0|---Rm---|---Rd---|	SBC
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 0| 1| 1| 1|---Rs---|---Rd---|	ROR
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 0| 0|---Rm---|---Rn---|	TST
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 0| 1|---Rm---|---Rd---|	NEG
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 1| 0|---Rm---|---Rn---|	CMP2
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 0| 1| 1|---Rm---|---Rd---|	CMN
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rd---|	ORR
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 0| 0|---Rm---|---Rd---|	MUL
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 1| 0|---Rm---|---Rd---|	BIC
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 0| 1| 1| 1| 1|---Rm---|---Rd---|	MVN
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
INLINE int ALU( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "ANDS", "EORS", "LSLS", "LSRS", "ASRS", "ADCS", "SBCS", "RORS", "TST", "NEGS", "CMP", "CMN", "ORRS", "MULS", "BIC", "MVN" };
	unsigned int		Rd, Rm, Operation;

	Rd					= ( OpCode >> 0 ) & 0x07;
	Rm					= ( OpCode >> 3 ) & 0x07;
	Operation			= ( OpCode >> 6 ) & 0x0f;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,R%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Rm );
	}/*============================================================================*/
#define	SKIP_NORMAL		0
#define	SKIP_LABEL		1
#define	SKIP_EMPTYLINE	2
#define	SKIP_SEPARATOR	3
#define	SKIP_SEPARATOR2	4
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 1| 0| 0|H1|H2|---Rm---|---Rd---|	ADD4
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 1| 0| 1|H1|H2|---Rm---|---Rn---|	CMP3
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 0| 1| 0| 0| 0| 1| 1| 0|H1|H2|---Rm---|---Rd---|	MOV
 +--+--+--+--+--+--+--+--+--+--+--------+--------+

*/
INLINE int Add4Cmp3Cpy( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	static const char	*Mnemonics[]	= { "ADD", "CMP", "MOV", "?47" };
	static const char	*Registers[]	= { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "SL", "FP", "IP", "SP", "LR", "PC" };
	unsigned int		Rd, Rm, Operation;

	Rd					= (( OpCode >> 4 ) & 0x08 ) | (( OpCode >> 0 ) & 0x07 );
	Rm					=  ( OpCode >> 3 ) & 0x0f;
	Operation			=  ( OpCode >> 8 ) & 0x03;

	if( Rd == 15 && ( Operation == 0 || Operation == 2 ))
		{
		if( Skip != NULL )
			*Skip		= SKIP_SEPARATOR;
		if( Operation == 2 )
			return sprintf( Text, "%08X:  %04X       B      %s", Address - 2, OpCode, Registers[Rm] );
		}

	if( Operation == 2 && Rd == Rm )
		return sprintf( Text, "%08X:  %04X       NOP               ; MOV    %s,%s", Address - 2, OpCode, Registers[Rd], Registers[Rm] );

	return sprintf( Text, "%08X:  %04X       %-5s  %s,%s", Address - 2, OpCode, Mnemonics[Operation], Registers[Rd], Registers[Rm] );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+-----------+--+--+--+
 |15|14|13|12|11|10|09|08|07|06 05 04 03|02|01|00|
 +--+--+--+--+--+--+--+--+--+-----------+--+--+--+
 | 0| 1| 0| 0| 0| 1| 1| 1| L|----Rm-----| 0| 0| 0|	BX/BLX
 +--+--+--+--+--+--+--+--+--+-----------+--+--+--+
*/
INLINE int BxBlx2( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	static const char	*Mnemonics[]	= { "BX", "BLX" };
	static const char	*Registers[]	= { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "SL", "FP", "IP", "SP", "LR", "PC" };
	unsigned int		Rm;
	unsigned int		Operation;

	Rm				=  ( OpCode >>  3 ) & 0x0f;
	Operation		=  ( OpCode >>  7 ) & 0x01;

	if( Skip != NULL )
		*Skip		= Operation == 0 ? SKIP_SEPARATOR : SKIP_EMPTYLINE;

	return sprintf( Text, "%08X:  %04X       %-5s  %s", Address - 2, OpCode, Mnemonics[Operation], Registers[Rm] );
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 0| 1| 0| 0| 1|---Rd---|---------Imm8----------|	LDR3
 +--+--+--+--+--+--------+-----------------------+
*/
INLINE execstatus_t Ldr3( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	unsigned int	Rd, Immediate;
	uint32_t		Target, Value;
	int				Index;
	const char		*SymbolName;

	Rd				= ( OpCode >>  8 ) & 0x07;
	Immediate		= ( OpCode <<  2 ) & 0x3fc;

	Target			= (( Address + 2 ) & 0xfffffffc ) + Immediate;

	if( ThumbLoadWord( State, Target, &Value ) < EXEC_STATUS_OK )
		Index	= sprintf( Text, "%08X:  %04X       LDR    R%u,[PC,#%u] ; (0x%X) = INACCESSIBLE", Address - 2, OpCode, Rd, Immediate, Target );
	else
		Index	= sprintf( Text, "%08X:  %04X       LDR    R%u,[PC,#%u] ; (0x%X) = 0x%X", Address - 2, OpCode, Rd, Immediate, Target, Value );

	if(( SymbolName = GetSymbolName( State, Value )) != NULL || (( Value & 1 ) != 0 && ( SymbolName = GetSymbolName( State, Value & ~(uint32_t)1 )) != NULL ))
		Index  += sprintf( &Text[Index], " = %s", SymbolName );

	return Index;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--------+--------+--------+
 |15|14|13|12|11|10|09|08 07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 0| 0|---Rm---|---Rn---|---Rd---|	STR2
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 0| 1|---Rm---|---Rn---|---Rd---|	STRH2
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 1| 0|---Rm---|---Rn---|---Rd---|	STRB2
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 0| 1| 1|---Rm---|---Rn---|---Rd---|	LDRSB
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 0| 0|---Rm---|---Rn---|---Rd---|	LDR2
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 0| 1|---Rm---|---Rn---|---Rd---|	LDRH2
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 1| 0|---Rm---|---Rn---|---Rd---|	LDRB2
 +--+--+--+--+--+--+--+--------+--------+--------+
 | 0| 1| 0| 1| 1| 1| 1|---Rm---|---Rn---|---Rd---|	LDRSH
 +--+--+--+--+--+--+--+--------+--------+--------+
*/
INLINE int LdrStr2( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "STR", "STRH", "STRB", "LDRSB", "LDR", "LDRH", "LDRB", "LDRSH" };
	unsigned int	Rd, Rn, Rm, Operation;

	Rd				= ( OpCode >>  0 ) & 0x07;
	Rn				= ( OpCode >>  3 ) & 0x07;
	Rm				= ( OpCode >>  6 ) & 0x07;
	Operation		= ( OpCode >>  9 ) & 0x07;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,[R%u,R%u]", Address - 2, OpCode, Mnemonics[Operation], Rd, Rn, Rm );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------+--------+--------+
 |15|14|13|12|11|10|09|08|07|06|05 04 03|02 01 00|
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 0| 0|-----Imm5-----|---Rn---|---Rd---|	STR1
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 0| 1|-----Imm5-----|---Rn---|---Rd---|	LDR1
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 1| 0|-----Imm5-----|---Rn---|---Rd---|	STRB1
 +--+--+--+--+--+--------------+--------+--------+
 | 0| 1| 1| 1| 1|-----Imm5-----|---Rn---|---Rd---|	LDRB1
 +--+--+--+--+--+--------------+--------+--------+
 | 1| 0| 0| 0| 0|-----Imm5-----|---Rn---|---Rd---|	STRH1
 +--+--+--+--+--+--------------+--------+--------+
 | 1| 0| 0| 0| 1|-----Imm5-----|---Rn---|---Rd---|	LDRH1
 +--+--+--+--+--+--------------+--------+--------+
*/
INLINE int LdrStr1( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "STRH", "LDRH", "?80", "?88", "STR", "LDR", "STRB", "LDRB" };
	unsigned int		Rd, Rn, Immediate, Operation;

	Rd					= ( OpCode >>  0 ) & 0x07;
	Rn					= ( OpCode >>  3 ) & 0x07;
	Operation			= ( OpCode >> 11 ) & 0x07;

	switch( Operation )
		{
		default:
			Immediate	= ( OpCode >>  4 ) & 0x7c;
			break;
		case 0: case 1:
			Immediate	= ( OpCode >>  5 ) & 0x3e;
			break;
		case 6: case 7:
			Immediate	= ( OpCode >>  6 ) & 0x1f;
			break;
		}

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,[R%u,#%u]", Address - 2, OpCode, Mnemonics[Operation], Rd, Rn, Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06b05 04 03-02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 0| 1| 0|---Rd---|---------Imm8----------|	STR3
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 0| 1| 1|---Rd---|---------Imm8----------|	LDR4
 +--+--+--+--+--+--------+-----------------------+
*/
INLINE int Str3Ldr4( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "STR", "LDR" };
	unsigned int		Rd, Immediate, Operation;

	Rd					= ( OpCode >>  8 ) & 0x07;
	Immediate			= ( OpCode <<  2 ) & 0x3fc;
	Operation			= ( OpCode >> 11 ) & 0x01;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,[SP,#%u]", Address - 2, OpCode, Mnemonics[Operation], Rd, Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------+-----------------------+
 |15|14|13|12|11|10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 1| 0| 0|---Rd---|---------Imm8----------|	ADD5
 +--+--+--+--+--+--------+-----------------------+
 | 1| 0| 1| 0| 1|---Rd---|---------Imm8----------|	ADD6
 +--+--+--+--+--+--------+-----------------------+
*/
INLINE int Add5Add6( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Registers[]	= { "PC", "SP" };
	unsigned int		Rd, Immediate, Rx;

	Rd					= ( OpCode >>  8 ) & 0x07;
	Immediate			= ( OpCode <<  2 ) & 0x3fc;
	Rx					= ( OpCode >> 11 ) & 0x01;

	return sprintf( Text, "%08X:  %04X       ADD    R%u,%s,#%u", Address - 2, OpCode, Rd, Registers[Rx], Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+--+--------------------+
 |15|14|13|12|11|10|09|08|07|06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+--+--------------------+
 | 1| 0| 1| 1| 0| 0| 0| 0| 0|--------Imm7--------|	ADD7
 +--+--+--+--+--+--+--+--+--+--------------------+
 | 1| 0| 1| 1| 0| 0| 0| 0| 1|--------Imm7--------|	SUB4
 +--+--+--+--+--+--+--+--+--+--------------------+
*/
INLINE int Add7Sub4( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "ADD", "SUB" };
	unsigned int		Operation, Immediate;

	Operation			= ( OpCode >>  7 ) & 0x01;
	Immediate			= ( OpCode <<  2 ) & 0x1fc;

	return sprintf( Text, "%08X:  %04X       %-5s  SP,#%u", Address - 2, OpCode, Mnemonics[Operation], Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 |15|14|13|12|11|10|09|08|07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0|--Op-|---Rm---|---Rd---|
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 0| 0|---Rm---|---Rd---|	SXTH
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 0| 1|---Rm---|---Rd---|	SXTB
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 1| 0|---Rm---|---Rd---|	UXTH
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 0| 0| 1| 0| 1| 1|---Rm---|---Rd---|	UXTB
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
INLINE int XXTX( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "SXTH", "SXTB", "UXTH", "UXTB" };
	unsigned int		Rd, Rm, Operation;

	Rd					= ( OpCode >>  0 ) & 0x07;
	Rm					= ( OpCode >>  3 ) & 0x07;
	Operation			= ( OpCode >>  6 ) & 0x03;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,R%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Rm );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 0| 1| 0|Lr|---------List----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE int Push( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	uint8_t			List, Lr;
	unsigned int	Rx;
	int				Index;

	List		= ( OpCode >>  0 ) & 0xff;
	Lr			= ( OpCode >>  8 ) & 0x01;

	Index		= sprintf( Text, "%08X:  %04X       PUSH   {", Address - 2, OpCode );

	for( Rx = 0; List != 0; Rx++, List >>= 1 )
		if( List & 0x01 )
			Index  += sprintf( &Text[Index], "R%u,", Rx );

	if( Lr )
		Index	   += sprintf( &Text[Index], "LR" );
	else
		Index--;

	Index		   += sprintf( &Text[Index], "}" );

	return Index;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 |15|14|13|12|11|10|09|08|07 06|05 04 03|02 01 00|
 +--+--+--+--+--+--+--+--+-----+--------+--------+
 | 1| 0| 1| 1| 1| 0| 1| 0| Op. |   Rn   |   Rd   |
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 1| 0| 1| 0| 0| 0|   Rn   |   Rd   |	REV
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 1| 0| 1| 0| 0| 1|   Rn   |   Rd   |	REV16
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 1| 0| 1| 0| 1| 0|   Rn   |   Rd   |	UNDEFINED
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
 | 1| 0| 1| 1| 1| 0| 1| 0| 1| 1|   Rn   |   Rd   |	REVSH
 +--+--+--+--+--+--+--+--+--+--+--------+--------+
*/
INLINE int RevXX( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	static const char	*Mnemonics[]	= { "REV", "REV16", "?ba8", "REVSH" };
	unsigned int		Rd, Rn, Operation;

	Rd			= ( OpCode >> 0 ) & 0x07;
	Rn			= ( OpCode >> 3 ) & 0x07;
	Operation	= ( OpCode >> 6 ) & 0x03;

	return sprintf( Text, "%08X:  %04X       %-5s  R%u,R%u", Address - 2, OpCode, Mnemonics[Operation], Rd, Rn );
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 0| 1| 0|Pc|---------List----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE int Pop( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	uint8_t			List, Pc;
	unsigned int	Rx;
	int				Index;

	List		= ( OpCode >>  0 ) & 0xff;
	Pc			= ( OpCode >>  8 ) & 0x01;

	Index		= sprintf( Text, "%08X:  %04X       POP    {", Address - 2, OpCode );

	for( Rx = 0; List != 0; Rx++, List >>= 1 )
		if( List & 0x01 )
			Index  += sprintf( &Text[Index], "R%u,", Rx );

	if( Pc )
		Index	   += sprintf( &Text[Index], "PC" );
	else
		Index--;

	Index		   += sprintf( &Text[Index], "}" );

	return Index;
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 1| 1| 1| 0|---------Imm8----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE int Bkpt( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	unsigned int	Immediate;

	if( Skip != NULL )
		*Skip		= SKIP_SEPARATOR2;

	Immediate		= ( OpCode >> 0 ) && 0xff;

	return sprintf( Text, "%08X:  %04X       BKPT   #%u", Address - 2, OpCode, Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 1| 1| 1| 1|---------Imm8----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE int Nop( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	return sprintf( Text, "%08X:  %04X       NOP", Address - 2, OpCode );
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 1| 1| 1| 1|---------Imm8----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE int Undefined( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	return sprintf( Text, "%08X:  %04X       UNDEFINED", Address - 2, OpCode );
	}
/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 0| 1| 1| 1| 1| 1| 1|---------Imm8----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE int Invalid( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	return sprintf( Text, "%08X:  %04X       INVALID", Address - 2, OpCode );
	}
/*============================================================================*/
/*
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; |15|14|13|12|11|10 09 08|07|06|05|04|03|02|01|00|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; | 1| 1| 0| 0| 0|---Rn---|r7|r6|r5|r4|r3|r2|r1|r0|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
*/
INLINE int Stmia( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	uint8_t			List;
	unsigned int	Rn, Rx;
	int				Index;

	List		= ( OpCode >>  0 ) & 0xff;
	Rn			= ( OpCode >>  8 ) & 0x03;

	if( List == 0 )
		return sprintf( Text, "%08X:  %04X       STMIA  R%u!,{} ; INVALID", Address - 2, OpCode, Rn );

	Index		= sprintf( Text, "%08X:  %04X       STMIA  R%u!,{", Address - 2, OpCode, Rn );

	for( Rx = 0; List != 0; Rx++, List >>= 1 )
		if( List & 0x01 )
			Index  += sprintf( &Text[Index], "R%u,", Rx );

	Index--;

	Index		   += sprintf( &Text[Index], "}" );

	return Index;
	}
/*============================================================================*/
/*
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; |15|14|13|12|11|10 09 08|07|06|05|04|03|02|01|00|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
; | 1| 1| 0| 0| 1|---Rn---|r7|r6|r5|r4|r3|r2|r1|r0|
; +--+--+--+--+--+--------+--+--+--+--+--+--+--+--+
*/
INLINE int Ldmia( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text )
	{
	unsigned int	Rn, List, Wb, Rx;
	int				Index;

	Rn				= ( OpCode >>  8 ) & 0x07;
	List			= ( OpCode >>  0 ) & 0xff;
	Wb				= ( ~List >> Rn ) & 0x01;

	if( List == 0 )
		return sprintf( Text, "%08X:  %04X       LDMIA  R%u!,{} ; INVALID", Address - 2, OpCode, Rn );

	Index		= sprintf( Text, "%08X:  %04X       LDMIA  R%u%s,{", Address - 2, OpCode, Rn, Wb ? "!" : "" );

	for( Rx = 0; List != 0; Rx++, List >>= 1 )
		if( List & 0x01 )
			Index  += sprintf( &Text[Index], "R%u,", Rx );

	Index--;

	Index		   += sprintf( &Text[Index], "}" );

	return Index;

	}
/*============================================================================*/
/*
 +--+--+--+--+-----------+-----------------------+
 |15|14|13|12|11 10 09 08|07 06 05 04 03 02 01 00|
 +--+--+--+--+-----------+-----------------------+
 | 1| 1| 0| 1|---Cond----|----------Imm8---------|
 +--+--+--+--+-----------+-----------------------+
*/
INLINE int Bcc( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	static const char	*Mnemonics[]	= { "BEQ", "BNE", "BCS", "BCC", "BMI", "BPL", "BVS", "BVC", "BHI", "BLS", "BGE", "BLT", "BGT", "BLE", "?de", "?df" };
	unsigned int		Condition;
	uint32_t			Offset;
	const char			*SymbolName;

	if( Skip != NULL )
		*Skip			= SKIP_EMPTYLINE;

	Condition			= ( OpCode >> 8 ) & 0x0f;
	Offset				= ( (int32_t)(int8_t)OpCode ) << 1;

	if(( SymbolName = GetSymbolName( State, Address + 2 + Offset )) != NULL || ((( Address + 2 + Offset ) & 1 ) != 0 && ( SymbolName = GetSymbolName( State, ( Address + 2 + Offset ) & ~(uint32_t)1 )) != NULL ))
		return sprintf( Text, "%08X:  %04X       %s.N  0x%X     ; $%+d = %s", Address - 2, OpCode, Mnemonics[Condition], Address + 2 + Offset, Offset + 4, SymbolName );
	else
		return sprintf( Text, "%08X:  %04X       %s.N  0x%X     ; $%+d", Address - 2, OpCode, Mnemonics[Condition], Address + 2 + Offset, Offset + 4 );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--+--+--+-----------------------+
 |15|14|13|12|11|10|09|08|07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--+--+--+-----------------------+
 | 1| 1| 0| 1| 1| 1| 1| 1|---------Imm8----------|
 +--+--+--+--+--+--+--+--+-----------------------+
*/
INLINE execstatus_t Swi( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	unsigned int	Immediate;

	if( Skip != NULL )
		*Skip		= SKIP_EMPTYLINE;

	Immediate		= ( OpCode >> 0 ) & 0xff;

	return sprintf( Text, "%08X:  %04X       SVC    #%u", Address - 2, OpCode, Immediate );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 0| 0|--------------Imm11-------------|
 +--+--+--+--+--+--------------------------------+
*/
INLINE int B2( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	int32_t		Offset;
	const char	*SymbolName;

	if( Skip != NULL )
		*Skip			= SKIP_SEPARATOR;

	Offset		 		= (( (int32_t)(int8_t)( OpCode >> 3 )) << 4 ) | ( (uint8_t)OpCode << 1 );

	if(( SymbolName = GetSymbolName( State, Address + 2 + Offset )) != NULL || ((( Address + 2 + Offset ) & 1 ) != 0 && ( SymbolName = GetSymbolName( State, ( Address + 2 + Offset ) & ~(uint32_t)1 )) != NULL ))
		return sprintf( Text, "%08X:  %04X       B.N    0x%X     ; $%+d = %s", Address - 2, OpCode, Address + 2 + Offset, Offset + 4, SymbolName );
	else
		return sprintf( Text, "%08X:  %04X       B.N    0x%X     ; $%+d", Address - 2, OpCode, Address + 2 + Offset, Offset + 4 );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 1| 0|--------------Imm11-------------|
 +--+--+--+--+--+--------------------------------+
*/
INLINE int BlBlx( thumbstate_t *State, uint16_t OpCode, uint32_t *Address, char *Text, int *Skip )
	{
	static const char	*Mnemonics[]	= { "BLX", "BL" };
	unsigned int		Operation;
	uint16_t			OpCode2;
	int32_t				Offset;
	uint32_t			Target;
	const char			*SymbolName;

	if( Skip != NULL )
		*Skip			= SKIP_EMPTYLINE;

	Operation			= ( OpCode >> 12 ) & 0x01;
	Offset				= (( (int32_t)(int8_t)( OpCode >> 3 )) << 15 ) | (( OpCode & 7 ) << 12 );
	Target				= *Address + 2 + Offset;

	if( ThumbLoadHalf( State, *Address, &OpCode2 ) < EXEC_STATUS_OK )
		return sprintf( Text, "%08X:  %04X ????  MOV    LR,#0x%08X ; INCOMPLETE BL/BLX 0x%05X???", *Address - 2, OpCode, Target, Target >> 12 );

	if(( OpCode2 & 0xe800 ) != 0xe800 )
		return sprintf( Text, "%08X:  %04X       MOV    LR,#0x%08X ; INCOMPLETE BL/BLX 0x%05X000+0x???", *Address - 2, OpCode, Target, Target >> 12 );

	*Address		   += 2;

	Target			   += (( OpCode2 << 1 ) & 0x0ffe );

	if(( SymbolName = GetSymbolName( State, Target )) != NULL || (( Target & 1 ) != 0 && ( SymbolName = GetSymbolName( State, Target & ~(uint32_t)1 )) != NULL ))
		return sprintf( Text, "%08X:  %04X %04X  %-5s  0x%X     ; $%+d = %s", *Address - 4, OpCode, OpCode2, Mnemonics[Operation], Target, Target + 4 - *Address, SymbolName );
	else
		return sprintf( Text, "%08X:  %04X %04X  %-5s  0x%X     ; $%+d", *Address - 4, OpCode, OpCode2, Mnemonics[Operation], Target, Target + 4 - *Address );
	}/*============================================================================*/
/*
 +--+--+--+--+--+--------------------------------+
 |15|14|13|12|11|10 09 08 07 06 05 04 03 02 01 00|
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 0| 1|--------------Imm11-------------|	BL suffix
 +--+--+--+--+--+--------------------------------+
 | 1| 1| 1| 1| 1|--------------Imm11-------------|	BLX suffix
 +--+--+--+--+--+--------------------------------+
*/
INLINE int BlSuffix( thumbstate_t *State, uint16_t OpCode, uint32_t Address, char *Text, int *Skip )
	{
	static const char	*Mnemonics[]	= { "BLX", "BL" };
	unsigned int		Operation;
	int32_t				Offset;

	if( Skip != NULL )
		*Skip			= SKIP_EMPTYLINE;

	Operation			= ( OpCode >> 12 ) & 0x01;
	Offset				= (( OpCode << 1 ) & 0x0ffe );

	return sprintf( Text, "%08X:  %04X       %-5s  0x????????+0x%03X ; *INCOMPLETE OPCODE*", Address - 2, OpCode, Mnemonics[Operation], Offset );
	}/*============================================================================*/
int ThumbDisassembler( thumbstate_t *State, uint32_t *Address, int Forced, char *Text, int *Skip )
	{
	sim_mapentry_t	MapEntry[4];
	uint16_t		OpCode;
	execstatus_t	Result;

	if( Skip != NULL )
		{
		const char	*SymbolName;
		if( *Skip == SKIP_EMPTYLINE )
			{
			*Skip	= SKIP_NORMAL;
			sprintf( Text, ";" );
			return DISASM_SEPARATOR;
			}
		else if( *Skip == SKIP_SEPARATOR )
			{
			*Skip	= SKIP_NORMAL;
			sprintf( Text, ";--------------------------------------------------" );
			return DISASM_SEPARATOR;
			}
		else if( *Skip == SKIP_SEPARATOR2 )
			{
			*Skip	= SKIP_NORMAL;
			sprintf( Text, ";==================================================" );
			return DISASM_SEPARATOR;
			}

		if( *Skip != SKIP_LABEL && ( SymbolName = GetSymbolName( State, *Address )) != NULL )
			{
			*Skip	= SKIP_LABEL;
			sprintf( Text, "%s:", SymbolName );
			return DISASM_LABEL;
			}
		*Skip	= SKIP_NORMAL;
		}

	MapEntry[0]	= GetMapEntry( State, *Address + 0 );
	MapEntry[1]	= GetMapEntry( State, *Address + 1 );
	MapEntry[2]	= GetMapEntry( State, *Address + 2 );
	MapEntry[3]	= GetMapEntry( State, *Address + 3 );

	/* Data or ARM Instruction. */
	if( !Forced && ( /*MapEntry[0].Type == 0 ||*/ MapEntry[0].Type == 1 || MapEntry[0].Type == 3 ))
		{
		/* The address is odd or the next address is not of the same type or it has a label of its own... */
		if(( *Address & 1 ) != 0 || MapEntry[1].Type != MapEntry[0].Type || MapEntry[1].NameOffset != (sim_nameoffset_t)-1 )
			{
			uint8_t	Value;
			/* ...we must output this address alone. */
			Result		= ThumbLoadByte( State, *Address, &Value );
			if( MapEntry[0].Type == 3 )
				{
				if( Result < EXEC_STATUS_OK )
					sprintf( Text, "%08X:  ??         ?? ; UNALIGNED INVALID ARM INSTRUCTION *INACCESSIBLE*", *Address );
				else
					sprintf( Text, "%08X:  %02hX         ?? ; UNALIGNED INVALID ARM INSTRUCTION", *Address, Value );
				*Address	   += 1;
				return DISASM_INSTRUCTION;
				}
			else
				{
				if( Result < EXEC_STATUS_OK )
					sprintf( Text, "%08X:  ??         .byte  0x?? ; DATA *INACCESSIBLE*", *Address );
				else
					sprintf( Text, "%08X:  %02hX         .byte  0x%02X", *Address, Value, Value );
				*Address	   += 1;
				return DISASM_DATA;
				}
			}
		/* The address is not a multiple of four or the second or third following addresses are not of the same type as this one or have a label of its own... */
		else if(( *Address & 2 ) != 0 || MapEntry[2].Type != MapEntry[0].Type || MapEntry[3].Type != MapEntry[0].Type || MapEntry[2].NameOffset != (sim_nameoffset_t)-1 || MapEntry[3].NameOffset != (sim_nameoffset_t)-1 )
			{
			uint16_t	Value;
			/* ...we must output this address alone. */
			Result		= ThumbLoadHalf( State, *Address, &Value );
			if( MapEntry[0].Type == 3 )
				{
				if( Result < EXEC_STATUS_OK )
					sprintf( Text, "%08X:  ????       ?? ; UNALIGNED INVALID ARM INSTRUCTION *INACCESSIBLE*", *Address );
				else
					sprintf( Text, "%08X:  %04hX       ?? ; UNALIGNED INVALID ARM INSTRUCTION", *Address, Value );
				*Address	   += 2;
				return DISASM_INSTRUCTION;
				}
			else
				{
				if( Result < EXEC_STATUS_OK )
					sprintf( Text, "%08X:  ????       .half  0x???? ; DATA *INACCESSIBLE*", *Address );
				else
					sprintf( Text, "%08X:  %04hX       .half  0x%04X", *Address, Value, Value );
				*Address	   += 2;
				return DISASM_DATA;
				}
			}
		/* The address is a multiple of four and the three following addresses are of the same type as this one and don't have a label of their own...*/
		else
			{
			uint32_t	Value;
			Result		= ThumbLoadWord( State, *Address, &Value );
			if( MapEntry[0].Type == 3 )
				{
				if( Result < EXEC_STATUS_OK )
					sprintf( Text, "%08X:  ????????   ; ARM INSTRUCTION *INACCESSIBLE*", *Address );
				else
					sprintf( Text, "%08X:  %08X   ?? ; ARM INSTRUCTION", *Address, Value );
				*Address	   += 4;
				return DISASM_INSTRUCTION;
				}
			else
				{
				const char	*SymbolName;

				if( Result < EXEC_STATUS_OK )
					sprintf( Text, "%08X:  ????????   .word  0x???????? ; DATA *INACCESSIBLE*", *Address );
				else if(( SymbolName = GetSymbolName( State, Value )) != NULL )
					sprintf( Text, "%08X:  %08X   .word  0x%08X ; DATA = %s", *Address, Value, Value, SymbolName );
				else if( (int32_t)Value < 0 )
					sprintf( Text, "%08X:  %08X   .word  0x%08X ; DATA = %u = %d", *Address, Value, Value, Value, (int32_t)Value );
				else
					sprintf( Text, "%08X:  %08X   .word  0x%08X ; DATA = %u", *Address, Value, Value, Value );

				*Address	   += 4;

				return DISASM_DATA;
				}
			}
		}

	/* Bit zero of the PC is not zero... */
	if(( *Address & 1 ) != 0 )
		/* ...the address is unaligned. */
		return DISASM_UNKNOWN;

	/* To fetch the opcode we must zero bit zero of the PC. */
	if( ThumbLoadHalf( State, *Address, &OpCode ) < EXEC_STATUS_OK )
		return DISASM_UNKNOWN;
	*Address		   += 2;

	switch( (uint8_t)( OpCode >> 8 ))
		{
		/* LSL */
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
		/* LSR */
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		/* ASR */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			Shifts1( State, OpCode, *Address, Text );
			break;

		/* ADD(3) */
		case 0x18: case 0x19:
		/* SUB(3) */
		case 0x1a: case 0x1b:
			AddSub3( State, OpCode, *Address, Text );
			break;

		/* ADD(1) */
		case 0x1c: case 0x1d:
		/* SUB(1) */
		case 0x1e: case 0x1f:
			AddSub1( State, OpCode, *Address, Text );
			break;

		/* MOV(1) */
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		/* CMP(1) */
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		/* ADD(2) */
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		/* SUB(2) */
		case 0x38: case 0x39: case 0x3a: case 0x3b:
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			MovCmp1AddSub2( State, OpCode, *Address, Text );
			break;

		/* AND,EOR,LSL(2),LSR(2) */
		case 0x40:
		/* ASR(2),ADC,SBC,ROR */
		case 0x41:
		/* TST,NEG,CMP(2),CMN */
		case 0x42:
		/* ORR,MUL,BIC,MVN */
		case 0x43:
			ALU( State, OpCode, *Address, Text );
			break;

		/* ADD(4) */
		case 0x44:
		/* CMP(3) */
		case 0x45:
		/* CPY */
		case 0x46:
			Add4Cmp3Cpy( State, OpCode, *Address, Text, Skip );
			break;

		/* BX,BLX(2) */
		case 0x47:
			BxBlx2( State, OpCode, *Address, Text, Skip );
			break;

		/* LDR(3) */
		case 0x48: case 0x49: case 0x4a: case 0x4b:
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			Ldr3( State, OpCode, *Address, Text );
			break;

		/* STR(2) */
		case 0x50: case 0x51:
		/* STRH(2) */
		case 0x52: case 0x53:
		/* STRB(2) */
		case 0x54: case 0x55:
		/* LDRSB */
		case 0x56: case 0x57:
		/* LDR(2) */
		case 0x58: case 0x59:
		/* LDRH(2) */
		case 0x5a: case 0x5b:
		/* LDRB(2) */
		case 0x5c: case 0x5d:
		/* LDRSH */
		case 0x5e: case 0x5f:
			LdrStr2( State, OpCode, *Address, Text );
			break;

		/* STR(1) */
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		/* LDR(1) */
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		/* STRB(1) */
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		/* LDRB(1) */
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		/* STRH(1) */
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		/* LDRH(1) */
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			LdrStr1( State, OpCode, *Address, Text );
			break;

		/* STR(3) */
		case 0x90: case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		/* LDR(4) */
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			Str3Ldr4( State, OpCode, *Address, Text );
			break;

		/* ADD(5) */
		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		/* ADD(6) */
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			Add5Add6( State, OpCode, *Address, Text );
			break;

		/* ADD(7),SUB(4) */
		case 0xb0:
			Add7Sub4( State, OpCode, *Address, Text );
			break;

		/* <undefined> */
		case 0xb1:
			Undefined( State, OpCode, *Address, Text );
			break;

		/* SXTH,SXTB,UXTH,UXTB */
		case 0xb2:
			XXTX( State, OpCode, *Address, Text );
			break;

		/* <undefined> */
		case 0xb3:
			Undefined( State, OpCode, *Address, Text );
			break;

		/* PUSH */
		case 0xb4: case 0xb5:
			Push( State, OpCode, *Address, Text );
			break;

		/* SETEND,CPS */
		case 0xb6:
			Invalid( State, OpCode, *Address, Text );
			break;

		/* <undefined> */
		case 0xb7:
		case 0xb8: case 0xb9:
			Undefined( State, OpCode, *Address, Text );
			break;

		case 0xba:
			RevXX( State, OpCode, *Address, Text );
			break;

		case 0xbb:
			Undefined( State, OpCode, *Address, Text );
			break;

		/* POP */
		case 0xbc: case 0xbd:
			Pop( State, OpCode, *Address, Text );
			break;

		/* BKPT */
		case 0xbe:
			Bkpt( State, OpCode, *Address, Text, Skip );
			break;

		/* <undefined> */
		case 0xbf:
			Nop( State, OpCode, *Address, Text );
			break;

		/* STMIA */
		case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		case 0xc4: case 0xc5: case 0xc6: case 0xc7:
			Stmia( State, OpCode, *Address, Text );
			break;

		/* LDMIA */
		case 0xc8: case 0xc9: case 0xca: case 0xcb:
		case 0xcc: case 0xcd: case 0xce: case 0xcf:
			Ldmia( State, OpCode, *Address, Text );
			break;

		/* BEQ */
		case 0xd0:
		/* BNE */
		case 0xd1:
		/* BCS */
		case 0xd2:
		/* BCC */
		case 0xd3:
		/* BMI */
		case 0xd4:
		/* BPL */
		case 0xd5:
		/* BVS */
		case 0xd6:
		/* BVC */
		case 0xd7:
		/* BHI */
		case 0xd8:
		/* BLS */
		case 0xd9:
		/* BGE */
		case 0xda:
		/* BLT */
		case 0xdb:
		/* BGT */
		case 0xdc:
		/* BLE */
		case 0xdd:
			Bcc( State, OpCode, *Address, Text, Skip );
			break;

		/* <undefined> */
		case 0xde:
			Undefined( State, OpCode, *Address, Text );
			break;

		/* SWI */
		case 0xdf:
			Swi( State, OpCode, *Address, Text, Skip );
			break;

		/* B(2) */
		case 0xe0: case 0xe1: case 0xe2: case 0xe3:
		case 0xe4: case 0xe5: case 0xe6: case 0xe7:
			B2( State, OpCode, *Address, Text, Skip );
			break;

		/* BLX suffix [4] */
		case 0xe8: case 0xe9: case 0xea: case 0xeb:
		case 0xec: case 0xed: case 0xee: case 0xef:
			BlSuffix( State, OpCode, *Address, Text, Skip );
			break;

		/* BL,BLX */
		case 0xf0: case 0xf1: case 0xf2: case 0xf3:
		case 0xf4: case 0xf5: case 0xf6: case 0xf7:
			BlBlx( State, OpCode, Address, Text, Skip );
			break;

		/* BLX suffix [4] */
		case 0xf8: case 0xf9: case 0xfa: case 0xfb:
		case 0xfc: case 0xfd: case 0xfe: case 0xff:
			BlSuffix( State, OpCode, *Address, Text, Skip );
			break;
		}

	return DISASM_INSTRUCTION;
	}
/*============================================================================*/
