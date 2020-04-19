//-----------------------------------------------------------------------------
/*!
   \file     midi_event.h
   \brief    Functions to decode midi events

   \author   M.Heiss
*/
//-----------------------------------------------------------------------------

#ifndef _MIDI_EVENT_H
#define _MIDI_EVENT_H

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>
#include "midi.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */
typedef void * T_MIDI_EVENT_HANDLE;


typedef struct
{
   uint32_t u32_DeltaTime;
   uint8_t u8_OnOff;
   uint8_t u8_Channel;
   uint8_t u8_Note;
   uint8_t u8_Velocity;
} T_midi_event_note;


/* -- Global Variables ---------------------------------------------------- */


/* -- Function Prototypes ------------------------------------------------- */
extern T_MIDI_EVENT_HANDLE midi_event_open(const T_midi_track_chunk * const opt_TrackChunk);
extern void midi_event_close(T_MIDI_EVENT_HANDLE opv_Handle);
//all events
extern void midi_event_hex_dump(T_MIDI_EVENT_HANDLE opv_Handle); //raw hex dump
extern void midi_event_print_events(T_MIDI_EVENT_HANDLE opv_Handle); //decode events and print to stdout
//note events
extern int32_t midi_event_get_note_events(T_MIDI_EVENT_HANDLE opv_Handle, T_midi_event_note ** oppt_NoteEvents);
extern int32_t midi_event_strip_redundant_note_events(const int32_t os32_Length, T_midi_event_note * opt_NoteEvents);
extern void midi_event_print_note_events(const int32_t os32_Length, const T_midi_event_note * opt_NoteEvents);






/* -- Implementation ------------------------------------------------------ */


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif



