//-----------------------------------------------------------------------------
/*!
   \file     midi_event.c
   \brief    Functions to decode midi events

   \author   M.Heiss
*/
//-----------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "midi_event.h"



/* -- Defines ------------------------------------------------------------- */


/* -- Types --------------------------------------------------------------- */
typedef struct
{
   const T_midi_track_chunk * pt_TrackChunk;
   T_midi_event_note * pat_NoteEvents;
} T_midi_event_instance;

/* -- Global Variables ---------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------- */
static const char * const mapcn_MidiEvent[16] =
{
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   0,
   "Note off (key is released)",
   "Note on (key is pressed)",
   "Key after-touch",
   "Control Change",
   "Program (patch) change",
   "Channel after-touch",
   "Pitch wheel change (2000H is normal or no change)",
   "Meta"
};

/* -- Module Global Function Prototypes ----------------------------------- */
static const uint8_t * get_variable_length_value(const uint8_t * opu8_Data, uint32_t * opu32_Vlv);


/* -- Implementation ------------------------------------------------------ */


T_MIDI_EVENT_HANDLE midi_event_open(const T_midi_track_chunk * const opt_TrackChunk)
{
   T_midi_event_instance * pt_MidiEventInstance;
   uint32_t u32_MaxNoteEvents;

   //------------------------------------------------------------//
   // allocate midi event instance                               //
   //------------------------------------------------------------//
   pt_MidiEventInstance = malloc(sizeof(T_midi_event_instance));
   pt_MidiEventInstance->pt_TrackChunk = opt_TrackChunk;

   //------------------------------------------------------------//
   // allocate buffer for note events                            //
   //------------------------------------------------------------//
   //calculate theoretical maximal number of note event:
   //each note event takes at least 4bytes (1byte delta-time + 3byte event chunk)
   u32_MaxNoteEvents = (opt_TrackChunk->u32_ChunkSize / 4) + 1; //round up in each case
   pt_MidiEventInstance->pat_NoteEvents = malloc(u32_MaxNoteEvents * sizeof(T_midi_event_note));


   //------------------------------------------------------------//
   // finalize                                                   //
   //------------------------------------------------------------//
   //return midi event instance handle
   return pt_MidiEventInstance;
}


void midi_event_close(T_MIDI_EVENT_HANDLE opv_Handle)
{
   T_midi_event_instance * const pt_MidiEventInstance = (T_midi_event_instance *)opv_Handle;

   //release note event buffer and instance itself
   free(pt_MidiEventInstance->pat_NoteEvents);
   free(pt_MidiEventInstance);
}




void midi_event_hex_dump(T_MIDI_EVENT_HANDLE opv_Handle)
{
   T_midi_event_instance * const pt_MidiEventInstance = (T_midi_event_instance *)opv_Handle;
   const T_midi_track_chunk * const pt_TrackChunk = pt_MidiEventInstance->pt_TrackChunk;
   uint32_t u32_Count;

   for (u32_Count = 0; u32_Count < pt_TrackChunk->u32_ChunkSize; ++u32_Count)
   {
       //print every byte's decimal equiv to command line
      printf("%02X\t",pt_TrackChunk->pu8_Chunk[u32_Count]);
      if ((u32_Count % 8) == 7) //print 8 numbers to a line
      {
         printf("\n");
      }
   }
   printf("\n");
}


void midi_event_print_events(T_MIDI_EVENT_HANDLE opv_Handle)
{
   T_midi_event_instance * const pt_MidiEventInstance = (T_midi_event_instance *)opv_Handle;
   const T_midi_track_chunk * const pt_TrackChunk = pt_MidiEventInstance->pt_TrackChunk;
   const uint8_t * pu8_Chunk = pt_TrackChunk->pu8_Chunk;
   const uint8_t * const pu8_ChunkEnd = &pu8_Chunk[pt_TrackChunk->u32_ChunkSize];


   //for each event
   while(pu8_Chunk < pu8_ChunkEnd)
   {
      uint32_t u32_DeltaTime;
      uint8_t u8_Command;
      uint8_t u8_MidiChannel;
      uint8_t u8_MidiCommand;

      //get delta time from variable length value and address of subsequent event
      pu8_Chunk = get_variable_length_value(pu8_Chunk, &u32_DeltaTime);

      //get command byte and switch by midi command
      u8_Command = *pu8_Chunk;
      u8_MidiChannel = u8_Command & 0x0F;
      u8_MidiCommand = u8_Command & 0xF0;
      switch (u8_MidiCommand)
      {
      //2 byte command
      case 0xC0: //Program (patch) change
      case 0xD0: //Channel after-touch
         pu8_Chunk = &pu8_Chunk[2];
         break;


      //3 byte command
      case 0x80: //Note off
      case 0x90: //Note on
      case 0xA0: //Key after-touch
      case 0xB0: //Control Change
      case 0xE0: //Pitch wheel change (2000H is normal or no change)
         pu8_Chunk = &pu8_Chunk[3];
         break;

      case 0xF0: //Meta Event
         if (u8_Command == 0xFF)
         {
            uint32_t u32_Length;

            u32_Length = pu8_Chunk[2]; //get length of payload data
            pu8_Chunk += (2 + u32_Length); //skip payload
         }
         else
         {
            printf("[E] Invalid META Command 0x%02X!\n", u8_MidiCommand);
         }
         pu8_Chunk = &pu8_Chunk[1];
         break;

      default:
         printf("[E] Invalid Command 0x%02X!\n", u8_Command);
         return;
      }

      //print event
      printf("Event: %s\n", mapcn_MidiEvent[(u8_MidiCommand >> 4)]);
      printf("\tChannel: %d\n", u8_MidiChannel);
      printf("\tDelta-Time: %d\n", u32_DeltaTime);
   }
}





int32_t midi_event_get_note_events(T_MIDI_EVENT_HANDLE opv_Handle, T_midi_event_note ** oppt_NoteEvents)
{
   T_midi_event_instance * const pt_MidiEventInstance = (T_midi_event_instance *)opv_Handle;
   const T_midi_track_chunk * const pt_TrackChunk = pt_MidiEventInstance->pt_TrackChunk;
   const uint8_t * pu8_Chunk = pt_TrackChunk->pu8_Chunk;
   const uint8_t * const pu8_ChunkEnd = &pu8_Chunk[pt_TrackChunk->u32_ChunkSize];
   T_midi_event_note * pt_NoteEvents = pt_MidiEventInstance->pat_NoteEvents;
   int32_t s32_Count;


   //for each event
   s32_Count = 0;
   *oppt_NoteEvents = pt_NoteEvents;
   while(pu8_Chunk < pu8_ChunkEnd)
   {
      uint8_t u8_Command;
      uint8_t u8_MidiChannel;
      uint8_t u8_MidiCommand;
      uint32_t u32_DeltaTime;

      //get delta time from variable length value and address of subsequent event
      pu8_Chunk = get_variable_length_value(pu8_Chunk, &u32_DeltaTime);

      //get command byte and switch by midi command
      u8_Command = *pu8_Chunk;
      u8_MidiChannel = u8_Command & 0x0F;
      u8_MidiCommand = u8_Command & 0xF0;
      switch (u8_MidiCommand)
      {
      //3 byte command
      case 0x80: //Note off
      case 0x90: //Note on
         pt_NoteEvents->u32_DeltaTime = u32_DeltaTime;
         pt_NoteEvents->u8_OnOff = ((u8_MidiCommand == 0x80) ? 0 : 1);
         pt_NoteEvents->u8_Channel = u8_MidiChannel;
         pt_NoteEvents->u8_Note = pu8_Chunk[1];
         pt_NoteEvents->u8_Velocity = pu8_Chunk[2];
         if (pt_NoteEvents->u8_Velocity == 0) //velocity == 0 is synoym for note off
         {
            pt_NoteEvents->u8_OnOff = 0; //note off is intended
         }
         //continue
         ++s32_Count;
         ++pt_NoteEvents;
         pu8_Chunk = &pu8_Chunk[3];
         break;


      //2 byte command
      case 0xC0: //Program (patch) change
      case 0xD0: //Channel after-touch
         pu8_Chunk = &pu8_Chunk[2];
         break;


      //3 byte command
      case 0xA0: //Key after-touch
      case 0xB0: //Control Change
      case 0xE0: //Pitch wheel change (2000H is normal or no change)
         pu8_Chunk = &pu8_Chunk[3];
         break;

      case 0xF0: //Meta Event
         if (u8_Command == 0xFF)
         {
            uint32_t u32_Length;

            u32_Length = pu8_Chunk[2]; //get length of payload data
            pu8_Chunk += (2 + u32_Length); //skip payload
         }
         else
         {
            printf("[E] Invalid META Command 0x%02X!\n", u8_MidiCommand);
         }
         pu8_Chunk = &pu8_Chunk[1];
         break;

      default:
         printf("[E] Invalid Command 0x%02X!\n", u8_Command);
         return - 1;
      }
   }

   //return number of note events
   return s32_Count;
}



int32_t midi_event_strip_redundant_note_events(const int32_t os32_Length, T_midi_event_note * opt_NoteEvents)
{
   return -1;
}



void midi_event_print_note_events(const int32_t os32_Length, const T_midi_event_note * opt_NoteEvents)
{
   int32_t s32_Count;


   //for each note event
   for (s32_Count = 0; s32_Count < os32_Length; ++s32_Count)
   {
      printf("%d. %s\n", s32_Count, mapcn_MidiEvent[8 + opt_NoteEvents->u8_OnOff]);
      printf("\tChannel %d", opt_NoteEvents->u8_Channel);
      printf("\tNote %d", opt_NoteEvents->u8_Note);
      printf("\tVelocity %d", opt_NoteEvents->u8_Velocity);
      printf("\tDelta Time %d\n", opt_NoteEvents->u32_DeltaTime);
      //
      ++opt_NoteEvents;
   }
}









/*
   If byte is greater or equal to 80h (128 decimal) then the next byte
        is also part of the VLV,
   else byte is the last byte in a VLV.
*/
static const uint8_t * get_variable_length_value(const uint8_t * opu8_Data, uint32_t * opu32_Vlv)
{
   uint32_t u32_7Bit;
   uint32_t u32_Vlv;

   u32_Vlv = 0;
   u32_7Bit = *opu8_Data++;
   while ((u32_7Bit & 0x80uL) != 0) //if bit 8 is set
   {
      u32_Vlv = ((u32_Vlv << 7) | (u32_7Bit & 0x7FuL));

      //get next part of variable length value
      u32_7Bit = *opu8_Data++;
   }

   //append last 7bit
   u32_Vlv = ((u32_Vlv << 7) | u32_7Bit);
   *opu32_Vlv = u32_Vlv;

   //return consecutive address
   return opu8_Data;
}




