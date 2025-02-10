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
#include "Errors.h"
/*=============================================================================*/
#define	LENGTH(a)	( sizeof (a) / sizeof (a)[0] )
/*=============================================================================*/
const char *GetErrorMessage( int ErrorCode )
	{
	static const char *ErrorMessages[]	=
		{
		[-RESULT_OK]									= "Load Successful",
		[-ERROR_OPENING_FILE]							= "Error Opening File",
		[-ERROR_READING_FILE]							= "Error Reading File",
		[-LOAD_ELF_FILE_ERROR_NOT_AN_ELF_FILE]			= "Not An ELF File",
		[-LOAD_ELF_FILE_ERROR_INCOMPATIBLE_ELF_FILE]	= "Incompatible ELF File",
		[-LOAD_ELF_FILE_ERROR_CORRUPT_ELF_FILE]			= "Corrupted ELF File",
		[-WXTHUMB_ERROR_NOT_ENOUGH_MEMORY]					= "Not Enough Memory",
		[-ERROR_INVALID_ARGUMENT]						= "Internal Error",
		[-LOAD_ELF_FILE_SECTION_NOT_FOUND]				= "Memory Section Not Found",
		[-ERROR_NO_MEMORY_AREA]							= "No Memory Segment",
		[-ERROR_NO_RAM_SEGMENT]							= "No RAM Segment",
		[-ERROR_SPARSE_MEMORY_AREA]						= "Sparse Memory Area",
		[-ERROR_OVERLAPPED_MEMORY_AREAS]				= "Overlapping Memory Areas",
		[-ERROR_CREATING_MEMORY_AREA]					= ""
		};


	ErrorCode	= - ErrorCode;

	if( ErrorCode < RESULT_OK || ErrorCode >= LENGTH( ErrorMessages ))
		return "Unknown Error";

	return ErrorMessages[ErrorCode];
	}
/*=============================================================================*/
