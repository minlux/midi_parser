//-----------------------------------------------------------------------------
/*!
   \file     sound.c
   \brief    Functions to generate sound "files" based on midi events

   \author   M.Heiss
*/
//-----------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "sound.h"

/* -- Defines ------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------- */
typedef struct
{
   const T_midi_event_note * pt_NoteEvent;
   int32_t s32_Length;
   T_sound_signal * pat_SignalSequence;
   double f64_ScaleTicksToMs;
} T_sound_instance;


/* -- Global Variables ---------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------- */

/* -- Implementation ------------------------------------------------------ */


T_SOUND_HANDLE sound_open(const int32_t os32_Length, const T_midi_event_note * opt_NoteEvents, const uint16_t ou16_TimeDivision)
{
   T_sound_instance * pt_SoundInstance;
   double f64_ScaleTo1ms;

   //preconditional check
   if (os32_Length <= 0)
   {
      return 0;
   }

   //------------------------------------------------------------//
   // allocate soundinstance                                     //
   //------------------------------------------------------------//
   pt_SoundInstance = malloc(sizeof(T_sound_instance));
   pt_SoundInstance->pt_NoteEvent = opt_NoteEvents;
   pt_SoundInstance->s32_Length = os32_Length;

   //------------------------------------------------------------//
   // calculate scale factor for time delay values               //
   //------------------------------------------------------------//
   if (ou16_TimeDivision <= 0x7FFFu)
   {
      //ticks per beat -> assum 500ms per beat
      f64_ScaleTo1ms = 500; //500ms
      f64_ScaleTo1ms = f64_ScaleTo1ms / ou16_TimeDivision;
   }
   else
   {
      uint32_t u32_TicksPerFrame;
      uint32_t u32_FramesPerSec;

      u32_TicksPerFrame = ou16_TimeDivision & 0x00FFu;
      u32_FramesPerSec = ((ou16_TimeDivision & 0x7F00u) >> 8);

      f64_ScaleTo1ms = 1000; //1000ms
      f64_ScaleTo1ms = (f64_ScaleTo1ms / u32_FramesPerSec) / u32_TicksPerFrame;
   }
   pt_SoundInstance->f64_ScaleTicksToMs = f64_ScaleTo1ms;


   //------------------------------------------------------------//
   // allocate buffer for signal sequence                        //
   //------------------------------------------------------------//
   pt_SoundInstance->pat_SignalSequence = malloc(os32_Length * sizeof(T_sound_instance));

   //------------------------------------------------------------//
   // finalize                                                   //
   //------------------------------------------------------------//
   //return sound instance handle
   return pt_SoundInstance;
}


void sound_close(T_SOUND_HANDLE opv_Handle)
{
   T_sound_instance * const pt_SoundInstance = (T_sound_instance *)opv_Handle;

   //release signal sequence buffer and instance itself
   free(pt_SoundInstance->pat_SignalSequence);
   free(pt_SoundInstance);
}


int32_t sound_get_signal_sequence(T_SOUND_HANDLE opv_Handle, T_sound_signal ** oppt_SignalSequence)
{
   T_sound_instance * const pt_SoundInstance = (T_sound_instance *)opv_Handle;
   const T_midi_event_note * pt_NoteEvent;
   T_sound_signal * pt_SignalSequence;
   const double f64_C = 8.175798916;
   double f64_Exp;
   double f64_2Exp;
   double f64_Frequency1Hz;
   uint16_t u16_Frequency1Hz;
   int32_t s32_Count;
   double f64_Duration1ms;
   uint16_t u16_Duration1ms;
   int32_t s32_Signal;

   //
   *oppt_SignalSequence = pt_SoundInstance->pat_SignalSequence;
   //for each note
   pt_SignalSequence = pt_SoundInstance->pat_SignalSequence;
   pt_NoteEvent = pt_SoundInstance->pt_NoteEvent;
   s32_Signal = 0;
   for (s32_Count = 0; s32_Count < pt_SoundInstance->s32_Length; ++s32_Count)
   {
      uint32_t u32_DeltaTime;

      u32_DeltaTime = pt_NoteEvent[1].u32_DeltaTime;
      if (u32_DeltaTime != 0)
      {
         //-----------------------------------------------------//
         // calculate frequency                                 //
         //-----------------------------------------------------//
         u16_Frequency1Hz = 0;
         if (pt_NoteEvent->u8_OnOff != 0)
         {
            //calculate exponent (note/12)
            f64_Exp = pt_NoteEvent->u8_Note;
            f64_Exp = f64_Exp / 12;
            //power of 2
            f64_2Exp = pow(2.0, f64_Exp);
            //frequency
            f64_Frequency1Hz = f64_C * f64_2Exp;
            u16_Frequency1Hz = (uint16_t)f64_Frequency1Hz;
         }
         pt_SignalSequence->u16_Frequency1Hz = u16_Frequency1Hz;

         //-----------------------------------------------------//
         // calculate duration                                  //
         //-----------------------------------------------------//
         f64_Duration1ms = u32_DeltaTime;
         f64_Duration1ms = f64_Duration1ms * pt_SoundInstance->f64_ScaleTicksToMs;
         u16_Duration1ms = (uint16_t)f64_Duration1ms;
         pt_SignalSequence->u16_Duration1ms = u16_Duration1ms;

         //next
         if ((s32_Signal > 0) && (pt_SignalSequence->u16_Frequency1Hz == pt_SignalSequence[-1].u16_Frequency1Hz))
         {
            pt_SignalSequence[-1].u16_Duration1ms += pt_SignalSequence->u16_Duration1ms;
         }
         else
         {
            ++s32_Signal;
            ++pt_SignalSequence;
         }
      }

      //next
      ++pt_NoteEvent;
   }

   //the last signal shoudl switch off
   pt_SignalSequence->u16_Frequency1Hz = 0;
   pt_SignalSequence->u16_Duration1ms = 0;
   return s32_Signal + 1;
}

void sound_print_signal_sequence(const int32_t os32_Length, const T_sound_signal * opt_SignalSequence)
{
   int32_t s32_Count;

   printf("Duration [1ms], Frequeny [1Hz]\n");
   for (s32_Count = 0; s32_Count < os32_Length; ++s32_Count)
   {
      printf("%d : %d, %d\n", s32_Count + 1, opt_SignalSequence->u16_Duration1ms, opt_SignalSequence->u16_Frequency1Hz);
   }
}

void sound_write_signal_sequence(const char * const opc_File, const int32_t os32_Length, const T_sound_signal * opt_SignalSequence)
{
   FILE * pv_File;
   int32_t s32_Count;

   //------------------------------------------------------------//
   // open file to write                                         //
   //------------------------------------------------------------//
   pv_File = fopen(opc_File, "w");

   //------------------------------------------------------------//
   // write to file                                              //
   //------------------------------------------------------------//
   fprintf(pv_File, "const uint16_t gau16_SoundSequence[] = { //2x16-bit value pair : Duration [1ms], Frequeny [1Hz]\n");
   for (s32_Count = 0; s32_Count < os32_Length; ++s32_Count)
   {
      fprintf(pv_File, "  %d, %d, ", opt_SignalSequence->u16_Duration1ms, opt_SignalSequence->u16_Frequency1Hz);
      if ((s32_Count % 8) == 7)
      {
         fprintf(pv_File, "\n");
      }
      ++opt_SignalSequence;
   }
   fprintf(pv_File, " 0, 0\n};\n\n");

   //------------------------------------------------------------//
   // close file                                                 //
   //------------------------------------------------------------//
   fclose(pv_File);
}
