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
/*=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*=============================================================================*/
#include "ELF.h"
#include "Simulator.h"
#include "LoadELFFile.h"
#include "Errors.h"
/*=============================================================================*/
typedef struct
	{
	char				*Name;
	unsigned			Index;
	elf_sectionheader_t	Data;
	} sectionitem_t;
/*=============================================================================*/
typedef struct
	{
	elf_elfheader_t		ELFHeader;
	char				*SectionsNames;
	sectionitem_t		*Sections;
	elf_segmentheader_t	Segments[];
	} allheaders_t;
/*=============================================================================*/
typedef struct
	{
	int				AreaIndex;
	uint32_t		BaseAddress;
	uint32_t		LastAddress;
	uint32_t		Size;
	} areainfo_t;
/*=======================================================================s======*/
typedef struct
	{
	unsigned			NumSections;
	uint32_t			BaseAddress;
	uint32_t			LastAddress;
	const sectionitem_t	*Sections[];
	} sectionlist_t;
/*=============================================================================*/
static int ReadSectionHeader( FILE *File, const elf_elfheader_t *ELFHeader, unsigned SectionNumber, elf_sectionheader_t *SectionHeader )
	{
	if( File == NULL || ELFHeader == NULL || SectionHeader == NULL )
		return ERROR_INVALID_ARGUMENT;

	if( SectionNumber >= ELFHeader->SectionHeaderEntries )
		return ERROR_READING_FILE;
	if( fseek( File, ELFHeader->SectionHeaderOffset + SectionNumber * ELFHeader->SectionHeaderSize, SEEK_SET ) < 0 ||
		  fread( SectionHeader, 1, sizeof *SectionHeader, File ) != sizeof *SectionHeader )
		return ERROR_READING_FILE;

	return RESULT_OK;
	}
/*=============================================================================*/
static int ReadSection( FILE *File, size_t Offset, size_t Size, void *Buffer )
	{
	if( File == NULL || Buffer == NULL )
		return ERROR_INVALID_ARGUMENT;

	if( fseek( File, Offset, SEEK_SET ) < 0 || fread( Buffer, 1, Size, File ) != Size )
		return ERROR_READING_FILE;

	return RESULT_OK;
	}
/*=============================================================================*/
static int ReadProgramHeader( FILE *File, const elf_elfheader_t *ELFHeader, unsigned AreaIndex, elf_segmentheader_t *ProgramHeader )
	{
	if( File == NULL || ELFHeader == NULL || ProgramHeader == NULL )
		return ERROR_INVALID_ARGUMENT;

	if( AreaIndex >= ELFHeader->ProgramHeaderEntries )
		return ERROR_READING_FILE;
	if( fseek( File, ELFHeader->ProgramHeaderOffset + AreaIndex * ELFHeader->ProgramHeaderSize, SEEK_SET ) < 0 ||
		  fread( ProgramHeader, 1, sizeof *ProgramHeader, File ) != sizeof *ProgramHeader )
		return ERROR_READING_FILE;

	return RESULT_OK;
	}
/*=============================================================================*/
static int ReadAllELFHeaders( FILE *File, allheaders_t **ELFHeaders )
	{
	elf_elfheader_t		ELFHeader;
	elf_sectionheader_t	SHSTRTABSectionHeader;
	allheaders_t		*AllHeaders;
	size_t				TotalSize;
	unsigned			i;
	int					Result;

	if( ELFHeaders == NULL )
		return ERROR_INVALID_ARGUMENT;

	*ELFHeaders	= NULL;

	if( File == NULL )
		return ERROR_INVALID_ARGUMENT;

	if( fseek( File, 0, SEEK_SET ) < 0 || fread( &ELFHeader, 1, sizeof ELFHeader, File ) != sizeof ELFHeader )
		return ERROR_READING_FILE;

	if( ELFHeader.MagicNumber != ELF_SIGNATURE )
		return LOAD_ELF_FILE_ERROR_NOT_AN_ELF_FILE;

	if( ELFHeader.WordLength != 1 || ELFHeader.Endianness != 1 || ELFHeader.HeaderVersion != 1 || ELFHeader.FileType != ELF_FILE_TYPE_EXECUTABLE ||
		  ELFHeader.MachineId != ELF_MACHINE_ARM )
		return LOAD_ELF_FILE_ERROR_INCOMPATIBLE_ELF_FILE;

	if( ELFHeader.ProgramHeaderSize != sizeof( elf_segmentheader_t ) || ELFHeader.SectionHeaderSize != sizeof ( elf_sectionheader_t ))
		return LOAD_ELF_FILE_ERROR_CORRUPT_ELF_FILE;

	if(( Result = ReadSectionHeader( File, &ELFHeader, ELFHeader.SectionHeaderNamesIndex, &SHSTRTABSectionHeader )) < RESULT_OK )
		return Result;

	TotalSize	= sizeof( allheaders_t ) +
				  ELFHeader.ProgramHeaderEntries * ELFHeader.ProgramHeaderSize +
				  ELFHeader.SectionHeaderEntries * sizeof AllHeaders->Sections[0] +
				  SHSTRTABSectionHeader.SectionSize;

	if(( AllHeaders = malloc( TotalSize )) == NULL )
		return WXTHUMB_ERROR_NOT_ENOUGH_MEMORY;

	/* Copy the already read ELF header to its place in the result structure. */
	memcpy( &AllHeaders->ELFHeader, &ELFHeader, sizeof AllHeaders->ELFHeader );

	AllHeaders->SectionsNames	= (char*)&AllHeaders->Segments[ELFHeader.ProgramHeaderEntries] +
								  ELFHeader.SectionHeaderEntries * sizeof AllHeaders->Sections[0];

	if(( Result = ReadSection( File, SHSTRTABSectionHeader.OffsetInFile, SHSTRTABSectionHeader.SectionSize, AllHeaders->SectionsNames )) < RESULT_OK )
		{
		free( AllHeaders );
		return Result;
		}

	for( i = 0; i < ELFHeader.ProgramHeaderEntries; i++ )
		if(( Result = ReadProgramHeader( File, &ELFHeader, i, &AllHeaders->Segments[i] )) < RESULT_OK )
			{
			free( AllHeaders );
			return Result;
			}

	AllHeaders->Sections	=   (sectionitem_t*)&AllHeaders->Segments[ELFHeader.ProgramHeaderEntries];

	for( i = 0; i < ELFHeader.SectionHeaderEntries; i++ )
		{
		if(( Result = ReadSectionHeader( File, &ELFHeader, i, &AllHeaders->Sections[i].Data )) < RESULT_OK )
			{
			free( AllHeaders );
			return Result;
			}
		AllHeaders->Sections[i].Name	= &AllHeaders->SectionsNames[AllHeaders->Sections[i].Data.NameOffset];
		AllHeaders->Sections[i].Index	= i;
		}

	*ELFHeaders	= AllHeaders;

	return RESULT_OK;
	}
/*=============================================================================*/
static int FindMemoryArea( allheaders_t *AllHeaders, areainfo_t *AreaInfo, int IsRAM )
	{
	unsigned		i;
	int				PrimaryArea;
	sim_address_t	LowestAddress	= 0, HighestAddress	= 0;

	if( AllHeaders == NULL || AreaInfo == NULL )
		return ERROR_INVALID_ARGUMENT;

	for( PrimaryArea = ERROR_NO_MEMORY_AREA, i = 0; i < AllHeaders->ELFHeader.ProgramHeaderEntries; i++ )
		{
		elf_segmentheader_t	*p;

		p	= &AllHeaders->Segments[i];

		if( p->Flags.Readable && (( IsRAM && p->Flags.Writeable ) ||
			  ( !IsRAM && !p->Flags.Writeable && p->Flags.Executable )))
			{
			if( PrimaryArea < RESULT_OK )
				{
				PrimaryArea			= i;
				LowestAddress		= p->VirtualAddress;
				HighestAddress		= p->VirtualAddress + p->SizeInMemory - 1;
				}
			else
				{
				if( p->VirtualAddress + p->SizeInMemory - 1 < LowestAddress )
					{
					if( LowestAddress - p->VirtualAddress - p->SizeInMemory > p->Alignment )
						return ERROR_SPARSE_MEMORY_AREA;
					PrimaryArea		= i;
					LowestAddress	= p->VirtualAddress;
					}
				else if( p->VirtualAddress > HighestAddress )
					{
					if( p->VirtualAddress - HighestAddress > p->Alignment )
						return ERROR_SPARSE_MEMORY_AREA;
					HighestAddress	= p->VirtualAddress + p->SizeInMemory - 1;
					}
				else
					return ERROR_OVERLAPPED_MEMORY_AREAS;
				}
			}
		}

	AreaInfo->AreaIndex		= PrimaryArea;
	AreaInfo->BaseAddress	= LowestAddress;
	AreaInfo->LastAddress	= HighestAddress;
	AreaInfo->Size			= HighestAddress - LowestAddress + 1;

	return RESULT_OK;
	}
/*=============================================================================*/
static int FindProgramSections( const allheaders_t *AllHeaders, uint32_t BaseAddress, uint32_t LimitAddress, int IsRAM, sectionlist_t **SectionList )
	{
	unsigned		NumSections, i;
	uint32_t		TotalLength;
	sectionlist_t	*SctnListTemp;

	if( SectionList == NULL )
		return ERROR_INVALID_ARGUMENT;

	*SectionList	= NULL;

	if( AllHeaders == NULL )
		return ERROR_INVALID_ARGUMENT;

	for( TotalLength = 0, NumSections = 0, i = 0; i < AllHeaders->ELFHeader.SectionHeaderEntries; i++ )
		{
		sectionitem_t *s	= &AllHeaders->Sections[i];

		if((( IsRAM && s->Data.Flags == 3 ) || ( !IsRAM && s->Data.Flags == 6 )) &&
				s->Data.VirtualAddress >= BaseAddress && ( LimitAddress <= BaseAddress ||
				s->Data.VirtualAddress + s->Data.SectionSize <= LimitAddress ))
			{
			if( s->Data.VirtualAddress + s->Data.SectionSize - BaseAddress > TotalLength )
				TotalLength	= s->Data.VirtualAddress + s->Data.SectionSize - BaseAddress;
			NumSections++;
			}
		}

	if( NumSections == 0 )
		return LOAD_ELF_FILE_SECTION_NOT_FOUND;

	if(( SctnListTemp = malloc( sizeof( sectionlist_t ) + NumSections * sizeof( elf_sectionheader_t	* ))) == NULL )
		return WXTHUMB_ERROR_NOT_ENOUGH_MEMORY;

	SctnListTemp->NumSections	= NumSections;
	SctnListTemp->BaseAddress	= BaseAddress;
	SctnListTemp->LastAddress	= BaseAddress + TotalLength - 1;

	for( NumSections = 0, i = 0; NumSections < SctnListTemp->NumSections && i < AllHeaders->ELFHeader.SectionHeaderEntries; i++ )
		{
		sectionitem_t *s	= &AllHeaders->Sections[i];

		if((( IsRAM && s->Data.Flags == 3 ) || ( !IsRAM && s->Data.Flags == 6 )) &&
				s->Data.VirtualAddress >= BaseAddress && ( LimitAddress <= BaseAddress ||
				s->Data.VirtualAddress + s->Data.SectionSize <= LimitAddress ))
			SctnListTemp->Sections[NumSections++]	= s;
		}

	*SectionList	= SctnListTemp;

	return RESULT_OK;
	}
/*=============================================================================*/
static int FindSection( const allheaders_t *AllHeaders, const char *SectionName, int SectionType )
	{
	unsigned	i;

	if( AllHeaders == NULL || ( SectionName == NULL && SectionType < 0 ))
		return ERROR_INVALID_ARGUMENT;

	for( i = 0; i < AllHeaders->ELFHeader.SectionHeaderEntries; i++ )
		{
		sectionitem_t *s	= &AllHeaders->Sections[i];

		if(( SectionName == NULL || strcmp( SectionName, s->Name ) == 0 ) && ( SectionType < 0 || SectionType == s->Data.Type ))
			return i;
		}

	return LOAD_ELF_FILE_SECTION_NOT_FOUND;
	}
/*=============================================================================*/
static int ReadSymbolTable( FILE *File, const allheaders_t *AllHeaders, symboltable_t **SymbolTable )
	{
	elf_sectionheader_t	*SymTblSctn, *StrTblSctn;
	symboltable_t		*TmpSymTbl;
	int					SymTblIdx, StrTblIdx;
	size_t				TotalSize;
	int					Result;

	if( SymbolTable == NULL )
		return ERROR_INVALID_ARGUMENT;

	*SymbolTable	= NULL;

	if( File == NULL || AllHeaders == NULL )
		return ERROR_INVALID_ARGUMENT;

	if(( SymTblIdx = FindSection( AllHeaders, ".symtab", 2 )) < RESULT_OK )
		return SymTblIdx;

	if(( StrTblIdx = FindSection( AllHeaders, ".strtab", 3 )) < RESULT_OK )
		return StrTblIdx;

	SymTblSctn	= &AllHeaders->Sections[SymTblIdx].Data;
	StrTblSctn	= &AllHeaders->Sections[StrTblIdx].Data;

	TotalSize	= sizeof( symboltable_t ) + SymTblSctn->SectionSize + StrTblSctn->SectionSize;

	if(( TmpSymTbl = malloc( TotalSize )) == NULL )
		return WXTHUMB_ERROR_NOT_ENOUGH_MEMORY;

	TmpSymTbl->NumEntries	= SymTblSctn->SectionSize / SymTblSctn->EntriesSize;
	TmpSymTbl->NamesTable	= (char*)&TmpSymTbl->Symbols[TmpSymTbl->NumEntries];

	if(( Result = ReadSection( File, SymTblSctn->OffsetInFile, SymTblSctn->SectionSize, TmpSymTbl->Symbols )) < RESULT_OK )
		{
		free( TmpSymTbl );
		return Result;
		}

	if(( Result = ReadSection( File, StrTblSctn->OffsetInFile, StrTblSctn->SectionSize, TmpSymTbl->NamesTable )) < RESULT_OK )
		{
		free( TmpSymTbl );
		return Result;
		}

	*SymbolTable	= TmpSymTbl;

	return RESULT_OK;
	}
/*=============================================================================*/
static int IsSymbolInSections( const sectionlist_t *SectionList, const elf_symboltableentry_t *Symbol )
	{
	unsigned	i;

	if( SectionList == NULL || Symbol == NULL )
		return 0;

	for( i = 0; i < SectionList->NumSections; i++ )
		if( Symbol->Section == SectionList->Sections[i]->Index )
			return 1;

	return 0;
	}
/*=============================================================================*/
static int EstimateAreaSize( areainfo_t *Area, const sectionlist_t *SectionList, const symboltable_t *SymbolTable )
	{
	unsigned	i;

	if( SymbolTable == NULL || SectionList == NULL || Area == NULL )
		return ERROR_INVALID_ARGUMENT;

	for( i = 0; i < SymbolTable->NumEntries; i++ )
		{
		const elf_symboltableentry_t *s	= &SymbolTable->Symbols[i];
		if( IsSymbolInSections( SectionList, s ) && s->VirtualAddress >= Area->LastAddress )
			Area->LastAddress	= (( s->VirtualAddress + 1023 ) & ~1023 ) - 1;
		}

	Area->Size	= Area->LastAddress - Area->BaseAddress + 1;

	return RESULT_OK;
	}
/*=============================================================================*/
static int CompareVAT( const void *a, const void *b )
	{
	const elf_symboltableentry_t	*A		= (elf_symboltableentry_t*)a;
	const elf_symboltableentry_t	*B		= (elf_symboltableentry_t*)b;
	/* Thumb addresses must be rounded to the even value below. */
	sim_address_t				AddrA	= ( A->Type & ~0x0230 ) == 0x0002 ? A->VirtualAddress & ~(sim_address_t)1 : A->VirtualAddress;
	sim_address_t				AddrB	= ( B->Type & ~0x0230 ) == 0x0002 ? B->VirtualAddress & ~(sim_address_t)1 : B->VirtualAddress;

	return AddrA != AddrB ? AddrA - AddrB :
		   /* 0x0000 (ELF_SYMBOL_TYPE_TYPE_MARKER) always comes first, but we must resolve it before the tests on the next two lines. */
		   (  A->Type == ELF_SYMBOL_TYPE_TYPE_MARKER || B->Type == ELF_SYMBOL_TYPE_TYPE_MARKER ) ?    A->Type - B->Type :
		   /* 0x0011 (ELF_SYMBOL_TYPE_LABEL_DATA) and 0x0012 (ELF_SYMBOL_TYPE_LABEL_THUMB) come before all other values, except 0x0000. */
		   ( A->Type == ELF_SYMBOL_TYPE_LABEL_DATA || A->Type == ELF_SYMBOL_TYPE_LABEL_THUMB || A->Type == 0x0212 ) &&   B->Type != ELF_SYMBOL_TYPE_LABEL_DATA && B->Type != ELF_SYMBOL_TYPE_LABEL_THUMB && B->Type != 0x0212   ? -1 :
		     A->Type != ELF_SYMBOL_TYPE_LABEL_DATA && A->Type != ELF_SYMBOL_TYPE_LABEL_THUMB && A->Type != 0x0212   && ( B->Type == ELF_SYMBOL_TYPE_LABEL_DATA || B->Type == ELF_SYMBOL_TYPE_LABEL_THUMB || B->Type == 0x0212 ) ?  1 :
		   /* 0x0010 (ELF_SYMBOL_TYPE_CONSTANT) comes next. */
		      A->Type == ELF_SYMBOL_TYPE_CONSTANT      &&   B->Type != ELF_SYMBOL_TYPE_CONSTANT       ? -1 :
		      A->Type != ELF_SYMBOL_TYPE_CONSTANT      &&   B->Type == ELF_SYMBOL_TYPE_CONSTANT       ?  1 :
		   /* All the other combinations are just a normal compare. */
		      A->Type - B->Type;
	}
/*=============================================================================*/
static int LoadSegment( FILE *File, allheaders_t *AllHeaders, unsigned AreaIndex, sim_memoryarea_t *Area )
	{
	int	Result;

	if( File == NULL || AllHeaders == NULL || Area == NULL )
		return ERROR_INVALID_ARGUMENT;

	memset( Area->Buffer, 0x00, Area->Size );
	if( Area->Pristine != NULL )
		memset( Area->Pristine, 0x00, Area->Size );
	if( Area->Previous != NULL )
		memset( Area->Previous, 0x00, Area->Size );

	const elf_segmentheader_t	*Segment	= &AllHeaders->Segments[AreaIndex];

	if(( Result = ReadSection( File, Segment->Offset, Segment->SizeInFile, Area->Buffer )) < RESULT_OK )
		return Result;

	if( Area->Pristine != NULL )
		memcpy( Area->Pristine, Area->Buffer, Area->Size );

	return RESULT_OK;
	}
/*=============================================================================*/
static int ReadELF_B( FILE *File, allheaders_t **AllHeaders, symboltable_t **SymbolTable,
					  sectionlist_t **ROMSections, sectionlist_t **RAMSections,
					  sim_memoryarea_t **MemoryAreas, unsigned *NumMemoryAreas, unsigned MaxMemoryAreas )
	{
	areainfo_t			RAMInfo, ROMInfo;
	int					Result;

	if(( Result = ReadAllELFHeaders( File, AllHeaders )) < RESULT_OK )
		return Result;

	if(( Result = FindMemoryArea( *AllHeaders, &RAMInfo, 1 )) < RESULT_OK )
		{
		if( Result == ERROR_NO_MEMORY_AREA )
			return ERROR_NO_RAM_SEGMENT;
		else
			return Result;
		}

	if(( Result = FindMemoryArea( *AllHeaders, &ROMInfo, 0 )) >= RESULT_OK
			&& ( Result = FindProgramSections( *AllHeaders, ROMInfo.BaseAddress, RAMInfo.BaseAddress, 0, ROMSections )) < RESULT_OK && Result != LOAD_ELF_FILE_SECTION_NOT_FOUND )
		return Result;

	if(( Result = FindProgramSections( *AllHeaders, RAMInfo.BaseAddress, ROMInfo.AreaIndex < 0 ? 0 : ROMInfo.BaseAddress, 1, RAMSections )) < RESULT_OK )
		return Result;

	if(( Result = ReadSymbolTable( File, *AllHeaders, SymbolTable )) < RESULT_OK && Result != LOAD_ELF_FILE_SECTION_NOT_FOUND )
		return Result;

	qsort( (*SymbolTable)->Symbols, (*SymbolTable)->NumEntries, sizeof (*SymbolTable)->Symbols[0], CompareVAT );


	EstimateAreaSize( &ROMInfo, *ROMSections, *SymbolTable );

	if(( Result = CreateSimulatorMemoryArea( &MemoryAreas[0], ROMInfo.BaseAddress, ROMInfo.Size, 1, 1, 1, NULL, NULL, NULL, *SymbolTable )) < RESULT_OK )
		return Result;

	if(( Result = LoadSegment( File, *AllHeaders, ROMInfo.AreaIndex, MemoryAreas[0] )) < RESULT_OK )
		return Result;

	(*NumMemoryAreas)++;

	EstimateAreaSize( &RAMInfo, *RAMSections, *SymbolTable);

	if(( Result = CreateSimulatorMemoryArea( &MemoryAreas[1], RAMInfo.BaseAddress, RAMInfo.Size, 1, 0, 1, NULL, NULL, NULL, *SymbolTable )) < RESULT_OK )
		return Result;

	if(( Result = LoadSegment( File, *AllHeaders, RAMInfo.AreaIndex, MemoryAreas[1] )) < RESULT_OK )
		return Result;

	(*NumMemoryAreas)++;

	return RESULT_OK;
	}
/*=============================================================================*/
int LoadELFFile( FILE *File, sim_memoryarea_t **MemoryAreas, unsigned *NumMemoryAreas, unsigned MaxMemoryAreas )
	{
	allheaders_t		*AllHeaders		= NULL;
	symboltable_t		*SymbolTable	= NULL;
	sectionlist_t		*ROMSections	= NULL, *RAMSections	= NULL;
	int					Result, i;

	if( File == NULL || MemoryAreas == NULL || MaxMemoryAreas == 0 )
		return ERROR_INVALID_ARGUMENT;

	*NumMemoryAreas		= 0;
	for( i = 0; i < MaxMemoryAreas; i++ )
		MemoryAreas[i]	= NULL;

	Result = ReadELF_B( File, &AllHeaders, &SymbolTable, &ROMSections, &RAMSections, MemoryAreas, NumMemoryAreas, MaxMemoryAreas );

	if( AllHeaders != NULL )
		free( AllHeaders );
	if( SymbolTable != NULL )
		free( SymbolTable );
	if( ROMSections != NULL )
		free( ROMSections );
	if( RAMSections != NULL )
		free( RAMSections );

	if( Result < RESULT_OK )
		{
		int i;
		for( i = 0; i < *NumMemoryAreas; i++ )
			{
			free( MemoryAreas[i] );
			MemoryAreas[i]	= NULL;
			}
		*NumMemoryAreas	= 0;
		}

	return Result;
	}
/*=============================================================================*/
int SimulatorLoadELFFile( thumbstate_t *CPUState, const char *Path )
	{
	FILE	*File;
	int		Result;

	if(( File = fopen( Path, "rb" )) == NULL )
		return ERROR_OPENING_FILE;

	Result = LoadELFFile( File, CPUState->MemoryAreas, &CPUState->NumMemoryAreas, 3 );

	fclose( File );

	return Result;
	}
/*=============================================================================*/
