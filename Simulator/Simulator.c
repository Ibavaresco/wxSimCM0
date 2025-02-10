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
//==============================================================================
#include <stdlib.h>
#include <string.h>
#include "Simulator.h"
#include "Thumb.h"
#include "Errors.h"
#include "ELF.h"
/*============================================================================*/
static int CalculateStringsAreaSize( sim_address_t BaseAddress, sim_address_t Length, symboltable_t *SymbolTable )
	{
	sim_address_t	LastAddress;
	unsigned		i;
	size_t			NumSymbols, Size;

	if( SymbolTable == NULL )
		return 0;

	LastAddress	= BaseAddress + Length - 1;

	for( Size = 0, NumSymbols = 0, i = 0; i < SymbolTable->NumEntries; i++ )
		{
		elf_symboltableentry_t	*s	= &SymbolTable->Symbols[i];
		unsigned				Length;

		if( s->VirtualAddress >= BaseAddress && s->VirtualAddress <= LastAddress
				&& ( s->Type == ELF_SYMBOL_TYPE_DATA || s->Type == ELF_SYMBOL_TYPE_THUMB
				  || s->Type == ELF_SYMBOL_TYPE_CONSTANT || s->Type == ELF_SYMBOL_TYPE_LABEL_DATA
				  || s->Type == ELF_SYMBOL_TYPE_LABEL_THUMB || s->Type == 0x0211
				  || s->Type == ELF_SYMBOL_TYPE_LABEL_ARM || s->Type == 0x0221 || s->Type == ELF_SYMBOL_TYPE_ALIAS_ARM )
				&& ( Length = strlen( &SymbolTable->NamesTable[s->NameOffset] )) > 0 )
			{
			NumSymbols++;
			Size   += Length + 1;
			}
		}

	return Size;
	}
/*============================================================================*/
static int CompareAddresses( int IsCode, size_t A, size_t B )
	{
	return (int)( IsCode ? ( A & ~(size_t)1 ) - ( B & ~(size_t)1 ) : A - B );
	}
/*============================================================================*/
typedef enum
	{
	ADDRESS_TYPE_UNKNOWN,
	ADDRESS_TYPE_DATA,
	ADDRESS_TYPE_THUMB,
	ADDRESS_TYPE_ARM
	} addresstypes_t;
/*============================================================================*/
#define	INVALID_NAME_OFFSET	((sim_nameoffset_t)-1)
/*============================================================================*/
static int PopulateSymbolTable( sim_address_t BaseAddress, sim_address_t Size,
								symboltable_t *SymbolTable, size_t StringsAreaSize, sim_memoryarea_t *Area )
	{
	size_t			Index, Address, NumInputSymbols, OutputStringsIndex;
	int				AddressType;
	sim_mapentry_t	*OutputSymbolTable;
	char			*OutputStringsArea;

	if( Area == NULL )
		return ERROR_INVALID_ARGUMENT;

	OutputSymbolTable	= Area->Map;
	OutputStringsArea	= Area->SymbolsNames;

	for( Index = 0; Index < Area->Size; Index++ )
		{
		OutputSymbolTable[Index].Type			= ADDRESS_TYPE_UNKNOWN;
		OutputSymbolTable[Index].TransientInfo	= 0;
		OutputSymbolTable[Index].NameOffset		= INVALID_NAME_OFFSET;
		}

	if( SymbolTable == NULL )
		return ERROR_INVALID_ARGUMENT;

	for( AddressType = ADDRESS_TYPE_UNKNOWN, OutputStringsIndex = 0, Index = 0, Address = BaseAddress, NumInputSymbols = 0;
		 Address <= Area->LastAddress && NumInputSymbols < SymbolTable->NumEntries; )
		{
		elf_symboltableentry_t	*s		= &SymbolTable->Symbols[NumInputSymbols];
		int						Result	= CompareAddresses(( s->Type & ( ELF_SYMBOL_TYPE_DATA | ELF_SYMBOL_TYPE_THUMB )) == ELF_SYMBOL_TYPE_THUMB
															 || ( s->Type == ELF_SYMBOL_TYPE_CONSTANT && AddressType == ADDRESS_TYPE_THUMB ),
															 s->VirtualAddress, Address );

		/* This symbol's address is less than the address we are processing... */
		if( Result < 0 )
			/* ...just skip the symbol. */
			NumInputSymbols++;
		/* The symbol is ahead of the address we are processing... */
		else if( Result > 0 )
			{
			OutputSymbolTable[Index].Type				=  AddressType;
			Address++;
			Index++;
			}
		else
			{
			if( s->Type == ELF_SYMBOL_TYPE_TYPE_MARKER )
				{
				if( strcmp( &SymbolTable->NamesTable[s->NameOffset], "$a" ) == 0 )
					AddressType							=  ADDRESS_TYPE_ARM;
				else if( strcmp( &SymbolTable->NamesTable[s->NameOffset], "$t" ) == 0 )
					AddressType							=  ADDRESS_TYPE_THUMB;
				else if( strcmp( &SymbolTable->NamesTable[s->NameOffset], "$d" ) == 0 )
					AddressType							=  ADDRESS_TYPE_DATA;
				}
			/* Valid symbol... */
			else if( s->Type == ELF_SYMBOL_TYPE_CONSTANT || ( s->Type & ~0x0230 ) == ELF_SYMBOL_TYPE_DATA || ( s->Type & ~0x0230 ) == ELF_SYMBOL_TYPE_THUMB )
				{
				static const int	Sizes[4]	= { [ADDRESS_TYPE_UNKNOWN] = 1, [ADDRESS_TYPE_DATA] = 1, [ADDRESS_TYPE_THUMB] = 2, [ADDRESS_TYPE_ARM] = 4 };
				int					i;

				/* This address does not have a symbol yet or the symbol type is one of 0x0011, 0x0012, 0x0211 or 0x0212, that have precedence... */
				if( OutputSymbolTable[Index].NameOffset == INVALID_NAME_OFFSET || ( s->Type != ELF_SYMBOL_TYPE_CONSTANT && ( s->Type & ~0x0203 ) == ELF_SYMBOL_TYPE_CONSTANT ))
					{
					int	Length	= strlen( &SymbolTable->NamesTable[s->NameOffset] );
					/* The string area still has room for this symbol's name... */
					if( OutputStringsIndex + Length < StringsAreaSize )
						{
						strcpy( &OutputStringsArea[OutputStringsIndex], &SymbolTable->NamesTable[s->NameOffset] );
						OutputSymbolTable[Index].NameOffset			= OutputStringsIndex;
						OutputStringsIndex						   += Length + 1;
						}
					}
				for( i = Sizes[AddressType&3]; i > 0; i--, Address++, Index++ )
					OutputSymbolTable[Index].Type				=  AddressType;
				}

			NumInputSymbols++;
			}
		}

	/* Fill the remaining of the table with empty entries. */
	for( ; Address <= Area->LastAddress; Address++, Index++ )
		{
		OutputSymbolTable[Index].Type					= AddressType;
		OutputSymbolTable[Index].TransientInfo			=  0;
		OutputSymbolTable[Index].NameOffset				= INVALID_NAME_OFFSET;
		}

	return RESULT_OK;
	}
/*============================================================================*/
int CreateSimulatorMemoryArea( sim_memoryarea_t **Area, sim_address_t BaseAddress,
							   sim_address_t Size, int Allocate, int ReadOnly, int Viewable, void *Cookie,
							   int (*ReadFunc)(sim_memoryarea_t*,sim_address_t,sim_address_t,void*),
							   int (*WriteFunc)(sim_memoryarea_t*,sim_address_t,sim_address_t,const void*),
							   symboltable_t *SymbolTable )
	{
	size_t					TotalSize, BufferSize, StringsAreaSize;
	sim_memoryarea_t		*AreaTemp;

	if( Area == NULL )
		return ERROR_INVALID_ARGUMENT;

	*Area					= NULL;

	BufferSize				= ( Size + sizeof( sim_address_t ) - 1 ) & ~( sizeof( sim_address_t ) - 1 );

	StringsAreaSize			= CalculateStringsAreaSize( BaseAddress, Size, SymbolTable );

	if( StringsAreaSize <= 0 )
		StringsAreaSize		= 0;
	else if( StringsAreaSize > INVALID_NAME_OFFSET )
		StringsAreaSize		= INVALID_NAME_OFFSET;

	TotalSize				= sizeof( sim_memoryarea_t ) + 3 * BufferSize + Size * sizeof( sim_mapentry_t ) + StringsAreaSize;

	if(( AreaTemp = malloc( TotalSize )) == NULL )
		return WXTHUMB_ERROR_NOT_ENOUGH_MEMORY;

	memset( AreaTemp, 0x00, TotalSize );

	AreaTemp->BaseAddress	= BaseAddress;
	AreaTemp->LastAddress	= BaseAddress + Size - 1;
	AreaTemp->Size			= Size;
	AreaTemp->Flags.ReadOnly= ReadOnly;
	AreaTemp->Flags.Viewable= Viewable;
	AreaTemp->Flags.Allocate= Allocate;
	AreaTemp->Cookie		= Cookie;
	AreaTemp->ReadFunc		= ReadFunc;
	AreaTemp->WriteFunc		= WriteFunc;
	AreaTemp->Pristine		= (uint8_t*)&AreaTemp->Buffer[BufferSize];
	AreaTemp->Previous		= (uint8_t*)&AreaTemp->Buffer[2*BufferSize];
	AreaTemp->Map			= (sim_mapentry_t*)&AreaTemp->Buffer[3*BufferSize];
	AreaTemp->SymbolsNames	= (char*)( &AreaTemp->Buffer[3*BufferSize] + Size * sizeof( sim_mapentry_t ));

	PopulateSymbolTable( BaseAddress, Size, SymbolTable, StringsAreaSize, AreaTemp );

	*Area					= AreaTemp;

	return RESULT_OK;
	}
//==============================================================================
int DeleteSimulatorMemoryArea( sim_memoryarea_t **Area )
	{
	if( Area == NULL || *Area == NULL )
		return ERROR_INVALID_ARGUMENT;

	free( *Area );
	*Area	= NULL;

	return RESULT_OK;
	}
//==============================================================================
