#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "midi.h"
#include "midi_event.h"
#include "sound.h"




int main(int argc, char ** argv)
{
   const char * pcn_Filename = "../result/demo.midi";
   T_midi_header_chunk t_HeaderChunk;
   T_midi_track_chunk t_TrackChunk;
   T_MIDI_HANDLE pv_Midi;
   T_MIDI_EVENT_HANDLE pv_MidiEvent;
   uint32_t u32_Track;
   T_midi_event_note * pt_NoteEvents;
   int32_t s32_NoteEvents;
   T_SOUND_HANDLE pv_Sound;
   T_sound_signal * pt_SignalSequence;
   int32_t s32_SignalSequence;

//   printf("Current directory: %s\n", get_current_dir_name()); //print binary
   //overwrite default input with the one specified at command line
   if (argc >= 2)
   {
      pcn_Filename = argv[1]; //1st argument assumed as input filename
   }

   //open midi
   pv_Midi = midi_open(pcn_Filename);
   if (pv_Midi != 0)
   {
      //midi header
      midi_get_header_chunk(pv_Midi, &t_HeaderChunk);
      midi_print_header_chunk(&t_HeaderChunk);

      //for each track
      for (u32_Track = 0; u32_Track < t_HeaderChunk.u16_NumOfTracks; ++u32_Track)
      {
         //track header
         midi_get_track_chunk(pv_Midi, u32_Track, &t_TrackChunk);
         midi_print_track_chunk(&t_TrackChunk);

         //midi events of track
         pv_MidiEvent = midi_event_open(&t_TrackChunk);
//         midi_event_hex_dump(pv_MidiEvent);
//         midi_event_print_events(pv_MidiEvent);

         //get note events
         s32_NoteEvents = midi_event_get_note_events(pv_MidiEvent, &pt_NoteEvents);
         if (s32_NoteEvents > 0)
         {
            midi_event_print_note_events(s32_NoteEvents, pt_NoteEvents);

//            //remove redundant events (e.g. note off + immediate note on event -> the note off event will be removed)
//            s32_NoteEvents = midi_event_strip_redundant_note_events(s32_NoteEvents, pt_NoteEvents);
//            if (s32_NoteEvents > 0)
            {
               pv_Sound = sound_open(s32_NoteEvents, pt_NoteEvents, t_HeaderChunk.u16_TimeDivision);
               //now the events can be converted to duration and frequency
               s32_SignalSequence = sound_get_signal_sequence(pv_Sound, &pt_SignalSequence);
               if (s32_SignalSequence > 0)
               {
                  sound_print_signal_sequence(s32_SignalSequence, pt_SignalSequence);
                  sound_write_signal_sequence("../result/out.c", s32_SignalSequence, pt_SignalSequence);
               }
               sound_close(pv_Sound);
            }
         }

         midi_event_close(pv_MidiEvent);
      }

      //close midi
      midi_close(pv_Midi);
   }
   return 0;
}


