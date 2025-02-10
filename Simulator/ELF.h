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
#if			!defined __ELF_H__
#define __ELF_H__
/*============================================================================*/
#include <stdint.h>
/*============================================================================*/
#if			defined __cplusplus
extern "C"
	{
#endif	/*	defined __cplusplus */
/*============================================================================*/
#pragma pack( 1 )
/*============================================================================*/
#define	ELF_SIGNATURE					0x464c457f
#define	ELF_MACHINE_ARM					0x0028
#define	ELF_FILE_TYPE_EXECUTABLE		0x0002

#define	ELF_SYMBOL_TYPE_TYPE_MARKER		0x0000
#define	ELF_SYMBOL_TYPE_DATA			0x0001
#define	ELF_SYMBOL_TYPE_THUMB			0x0002
#define	ELF_SYMBOL_TYPE_LABEL_SECTION	0x0003
#define	ELF_SYMBOL_TYPE_FILENAME		0x0004
#define	ELF_SYMBOL_TYPE_CONSTANT		0x0010
#define	ELF_SYMBOL_TYPE_LABEL_DATA		0x0011
#define	ELF_SYMBOL_TYPE_LABEL_THUMB		0x0012
#define	ELF_SYMBOL_TYPE_ALIAS_THUMB		0x0022
#define	ELF_SYMBOL_TYPE_LABEL_ARM		0x0212
#define	ELF_SYMBOL_TYPE_ALIAS_ARM		0x0222
/*=============================================================================*/
typedef struct
	{
	uint32_t			MagicNumber;
	uint8_t				WordLength;
	uint8_t				Endianness;
	uint8_t				HeaderVersion;
	uint8_t				OperatingSystemId;
	uint8_t				ABIVersion;
	uint8_t				Padding[7];
	uint16_t			FileType;
	uint16_t			MachineId;
	uint32_t			ELFVersion;
	uint32_t			EntryPointAddress;
	uint32_t			ProgramHeaderOffset;
	uint32_t			SectionHeaderOffset;
	uint32_t			Flags;
	uint16_t			HeaderSize;
	uint16_t			ProgramHeaderSize;
	uint16_t			ProgramHeaderEntries;
	uint16_t			SectionHeaderSize;
	uint16_t			SectionHeaderEntries;
	uint16_t			SectionHeaderNamesIndex;
	} elf_elfheader_t;
/*============================================================================*/
typedef struct
	{
	uint32_t			Type;
	uint32_t			Offset;
	uint32_t			VirtualAddress;
	uint32_t			PhysicalAddress;
	uint32_t			SizeInFile;
	uint32_t			SizeInMemory;
	union
		{
		struct
			{
			uint32_t	Executable	: 1;
			uint32_t	Writeable	: 1;
			uint32_t	Readable	: 1;
			};
		uint32_t		Value;
		}				Flags;
	uint32_t			Alignment;
	} elf_segmentheader_t;
/*============================================================================*/
typedef struct
	{
	uint32_t			NameOffset;
	uint32_t			Type;
	uint32_t			Flags;
	uint32_t			VirtualAddress;
	uint32_t			OffsetInFile;
	uint32_t			SectionSize;
	uint32_t			Link;
	uint32_t			Info;
	uint32_t			Alignment;
	uint32_t			EntriesSize;
	} elf_sectionheader_t;
/*============================================================================*/
typedef struct
	{
	uint32_t			NameOffset;
	uint32_t			VirtualAddress;
	uint32_t			Size;
	uint16_t			Type;
	uint16_t			Section;
	} elf_symboltableentry_t;
/*============================================================================*/
#pragma pack()
/*============================================================================*/
#if			defined __cplusplus
	}
#endif	/*	defined __cplusplus */
/*============================================================================*/
#endif	/*	!defined __ELF_H__ */
/*============================================================================*/
