//-----------------------------------------------------------------------------
/*!
   \file     midi.h
   \brief    Functions to open and parse midi files

   \author   M.Heiss
*/
//-----------------------------------------------------------------------------

#ifndef _MIDI_H
#define _MIDI_H

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */
typedef void * T_MIDI_HANDLE;

typedef struct
{
   char acn_ChunkId[4+1];
   uint32_t u32_ChunkSize;
   uint16_t u16_FormatType;
   uint16_t u16_NumOfTracks;
   uint16_t u16_TimeDivision;
} T_midi_header_chunk;

typedef struct
{
   char acn_ChunkId[4+1];
   uint32_t u32_ChunkSize;
   const uint8_t * pu8_Chunk;
} T_midi_track_chunk;




/* -- Global Variables ---------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */
extern T_MIDI_HANDLE midi_open(const char * const opc_File);
extern void midi_close(T_MIDI_HANDLE opv_Handle);

//header
extern int32_t midi_get_header_chunk(T_MIDI_HANDLE opv_Handle, T_midi_header_chunk * const opt_HeaderChunk);
extern void midi_print_header_chunk(const T_midi_header_chunk * const opt_HeaderChunk);
//tracks
extern int32_t midi_get_track_chunk(T_MIDI_HANDLE opv_Handle, const uint32_t ou32_Track, T_midi_track_chunk * const opt_TrackChunk);
extern void midi_print_track_chunk(const T_midi_track_chunk * const opt_TrackChunk);





/* -- Implementation ------------------------------------------------------ */


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif



