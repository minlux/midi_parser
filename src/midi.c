//-----------------------------------------------------------------------------
/*!
   \file     midi.c
   \brief    Functions to open and parse midi files

   \author   M.Heiss
*/
//-----------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "midi.h"



/* -- Defines ------------------------------------------------------------- */


/* -- Types --------------------------------------------------------------- */
typedef struct
{
   uint8_t * pu8_FileBuffer;
   T_midi_header_chunk t_HeaderChunk;
} T_midi_instance;

/* -- Global Variables ---------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------- */
static const uint8_t * decode_track_chunk(const uint8_t * opu8_Data, T_midi_track_chunk * const opt_TrackChunk);


/* -- Implementation ------------------------------------------------------ */

T_MIDI_HANDLE midi_open(const char * const opc_File)
{
   FILE * pv_File;
   uint32_t u32_FileSize1By;
   T_midi_instance * pt_MidiInstance;
   uint8_t u8_Char;
   uint32_t u32_Count;



   //------------------------------------------------------------//
   // open file to read                                          //
   //------------------------------------------------------------//
   pv_File = fopen(opc_File, "rb"); //open for read in binary mode
   if (pv_File == NULL)
   {
      printf("cannot open file %s\n", opc_File);
      return NULL;
   }

   //get size of file
   fseek(pv_File, 0, SEEK_END); // seek to end of file
   u32_FileSize1By = ftell(pv_File); // get current file pointer
   fseek(pv_File, 0, SEEK_SET); // seek back to beginning of file
   //------------------------------------------------------------//
   // allocate midi instance and file buffer                     //
   //------------------------------------------------------------//
   pt_MidiInstance = malloc(sizeof(T_midi_instance));
   pt_MidiInstance->pu8_FileBuffer = malloc(u32_FileSize1By);

   //------------------------------------------------------------//
   // load file to buffer                                        //
   //------------------------------------------------------------//
   u32_Count = 0;
   u8_Char = fgetc(pv_File);
   while (!feof(pv_File)) //loop for whole file
   {
      pt_MidiInstance->pu8_FileBuffer[u32_Count++] = u8_Char; //add to buffer
      u8_Char = fgetc(pv_File); //next char
   }


   u32_Count = 0;
   //------------------------------------------------------------//
   // decode MIDI header                                         //
   //------------------------------------------------------------//
   //chunck id
   pt_MidiInstance->t_HeaderChunk.acn_ChunkId[0] = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.acn_ChunkId[1] = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.acn_ChunkId[2] = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.acn_ChunkId[3] = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.acn_ChunkId[4] = 0;
   //chunk size
   pt_MidiInstance->t_HeaderChunk.u32_ChunkSize = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.u32_ChunkSize = (pt_MidiInstance->t_HeaderChunk.u32_ChunkSize << 8) | pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.u32_ChunkSize = (pt_MidiInstance->t_HeaderChunk.u32_ChunkSize << 8) | pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.u32_ChunkSize = (pt_MidiInstance->t_HeaderChunk.u32_ChunkSize << 8) | pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   //format type
   pt_MidiInstance->t_HeaderChunk.u16_FormatType = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.u16_FormatType = (pt_MidiInstance->t_HeaderChunk.u16_FormatType << 8) | pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   //number of tracks
   pt_MidiInstance->t_HeaderChunk.u16_NumOfTracks = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.u16_NumOfTracks = (pt_MidiInstance->t_HeaderChunk.u16_NumOfTracks << 8) | pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   //number of tracks
   pt_MidiInstance->t_HeaderChunk.u16_TimeDivision = pt_MidiInstance->pu8_FileBuffer[u32_Count++];
   pt_MidiInstance->t_HeaderChunk.u16_TimeDivision = (pt_MidiInstance->t_HeaderChunk.u16_TimeDivision << 8) | pt_MidiInstance->pu8_FileBuffer[u32_Count++];


   //------------------------------------------------------------//
   // finalize                                                   //
   //------------------------------------------------------------//
   //close file
   fclose(pv_File);
   //return midi instance handle
   return pt_MidiInstance;
}


void midi_close(T_MIDI_HANDLE opv_Handle)
{
   T_midi_instance * const pt_MidiInstance = (T_midi_instance *)opv_Handle;

   //release file buffer and instance itself
   free(pt_MidiInstance->pu8_FileBuffer);
   free(pt_MidiInstance);
}



int32_t midi_get_header_chunk(T_MIDI_HANDLE opv_Handle, T_midi_header_chunk * const opt_HeaderChunk)
{
   T_midi_instance * const pt_MidiInstance = (T_midi_instance *)opv_Handle;

   //copy header chunk from instance
   *opt_HeaderChunk = pt_MidiInstance->t_HeaderChunk;
   return 0;
}


void midi_print_header_chunk(const T_midi_header_chunk * const opt_HeaderChunk)
{
   //print header chunk
   printf("Header Chunk\n");
   printf("\tChunk ID: %s\n", opt_HeaderChunk->acn_ChunkId);
   printf("\tChunk Size: %d\n", opt_HeaderChunk->u32_ChunkSize);
   printf("\tFormat Type: %d\n", opt_HeaderChunk->u16_FormatType);
   printf("\tNumber of Tracks: %d\n", opt_HeaderChunk->u16_NumOfTracks);
   printf("\tTime Division: 0x%04X\n\n", opt_HeaderChunk->u16_TimeDivision);
}



static const uint8_t * decode_track_chunk(const uint8_t * opu8_Data, T_midi_track_chunk * const opt_TrackChunk)
{
   uint32_t u32_ChunkSize1By;

   //decode track header
   //chunck id
   opt_TrackChunk->acn_ChunkId[0] = *opu8_Data++;
   opt_TrackChunk->acn_ChunkId[1] = *opu8_Data++;
   opt_TrackChunk->acn_ChunkId[2] = *opu8_Data++;
   opt_TrackChunk->acn_ChunkId[3] = *opu8_Data++;
   opt_TrackChunk->acn_ChunkId[4] = 0;
   //chunk size
   u32_ChunkSize1By = *opu8_Data++;
   u32_ChunkSize1By = (u32_ChunkSize1By << 8) | *opu8_Data++;
   u32_ChunkSize1By = (u32_ChunkSize1By << 8) | *opu8_Data++;
   u32_ChunkSize1By = (u32_ChunkSize1By << 8) | *opu8_Data++;
   opt_TrackChunk->u32_ChunkSize = u32_ChunkSize1By;
   //set chunk address
   opt_TrackChunk->pu8_Chunk = opu8_Data;

   //return address of next track
   return &opu8_Data[u32_ChunkSize1By];
}


int32_t midi_get_track_chunk(T_MIDI_HANDLE opv_Handle, const uint32_t ou32_Track, T_midi_track_chunk * const opt_TrackChunk)
{
   T_midi_instance * const pt_MidiInstance = (T_midi_instance *)opv_Handle;
   const uint8_t * pu8_Track;
   uint32_t u32_Track;

   //preconditional check
   if (ou32_Track >= pt_MidiInstance->t_HeaderChunk.u16_NumOfTracks)
   {
      return -1;
   }


   //otherwise
   u32_Track = 0;
   //get start of selected track
   pu8_Track = &pt_MidiInstance->pu8_FileBuffer[14]; //reference to first track (skip 14 bytes header chunk)
   for (u32_Track = 0; u32_Track <= ou32_Track; ++u32_Track) //get track chunk of selected track
   {
      pu8_Track = decode_track_chunk(pu8_Track, opt_TrackChunk);
   }
   return 0;
}


void midi_print_track_chunk(const T_midi_track_chunk * const opt_TrackChunk)
{
   //print track header
   printf("Track Chunk\n");
   printf("\tChunk ID: %s\n", opt_TrackChunk->acn_ChunkId);
   printf("\tChunk Size: %d\n", opt_TrackChunk->u32_ChunkSize);
}



