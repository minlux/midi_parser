//-----------------------------------------------------------------------------
/*!
   \file     sound.h
   \brief    Functions to generate sound "files" based on midi events

   Sound files are C constant tables, containing frequency and duration of a "note".

   \author   M.Heiss
*/
//-----------------------------------------------------------------------------

#ifndef _SOUND_H
#define _SOUND_H

/* -- Includes ------------------------------------------------------------ */
#include <stdint.h>
#include "midi_event.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */
typedef void * T_SOUND_HANDLE;


typedef struct
{
   uint16_t u16_Duration1ms;
   uint16_t u16_Frequency1Hz;
} T_sound_signal;


/* -- Global Variables ---------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------- */
extern T_SOUND_HANDLE sound_open(const int32_t os32_Length, const T_midi_event_note * opt_NoteEvents, const uint16_t ou16_TimeDivision);
extern void sound_close(T_SOUND_HANDLE opv_Handle);

extern int32_t sound_get_signal_sequence(T_SOUND_HANDLE opv_Handle, T_sound_signal ** oppt_SignalSequence);
extern void sound_print_signal_sequence(const int32_t os32_Length, const T_sound_signal * opt_SignalSequence);
extern void sound_write_signal_sequence(const char * const opc_File, const int32_t os32_Length, const T_sound_signal * opt_SignalSequence);

/* -- Implementation ------------------------------------------------------ */


#ifdef __cplusplus
} /* end of extern "C" */
#endif

#endif



