// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright(C) 2009 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//    Reading of MIDI files.
//
//-----------------------------------------------------------------------------


#include "z_zone.h"

#ifdef _WIN32
#include "winsock2.h"
#else
#include <arpa/inet.h>
#endif

#include "midifile.h"

#define HEADER_CHUNK_ID "MThd"
#define TRACK_CHUNK_ID  "MTrk"
#define MAX_BUFFER_SIZE 0x10000

#ifdef _WIN32
#pragma pack(push, 1)
#endif

typedef struct {
  unsigned char chunk_id[4];
  unsigned int chunk_size;
} PACKEDATTR chunk_header_t;

typedef struct {
  chunk_header_t chunk_header;
  unsigned short format_type;
  unsigned short num_tracks;
  unsigned short time_division;
} PACKEDATTR midi_header_t;

#ifdef _WIN32
#pragma pack(pop)
#endif

typedef struct {
  unsigned int data_len;      // Length in bytes
  midi_event_t *events;       // Events in this track
  unsigned int num_events;
  unsigned int num_event_mem; // NSM track size of structure
} midi_track_t;

struct midi_track_iter_s {
  midi_track_t *track;
  unsigned int position;
};

struct midi_file_s {
  midi_header_t  header;
  midi_track_t  *tracks;      // All tracks in this file:
  unsigned int   num_tracks;
  unsigned char *buffer;      // Data buffer used to store data read for SysEx
  unsigned int   buffer_size; // or meta events
};

// Check the header of a chunk:
static bool CheckChunkHeader(chunk_header_t *chunk,
                             const char *expected_id) {
  bool result;

  result = (memcmp((char *) chunk->chunk_id, expected_id, 4) == 0);

  if (!result) {
    D_MsgLocalWarn(
      "CheckChunkHeader: Expected '%s' chunk header, got '%c%c%c%c'\n",
      expected_id,
      chunk->chunk_id[0],
      chunk->chunk_id[1],
      chunk->chunk_id[2],
      chunk->chunk_id[3]
    );
  }

  return result;
}

// Read a single byte.  Returns false on error.
static bool ReadByte(unsigned char *result, midimem_t *mf) {
  if (mf->pos >= mf->len) {
    D_MsgLocalWarn("ReadByte: Unexpected end of file\n");
    return false;
  }

  *result = mf->data[mf->pos];
  mf->pos++;
  return true;
}

static bool ReadMultipleBytes(void *dest, size_t len, midimem_t *mf) {
  unsigned char *cdest = (unsigned char *) dest;

  for (unsigned int i = 0; i < len; i++) {
    if (!ReadByte (cdest + i, mf)) {
      D_MsgLocalWarn("ReadMultipleBytes: Unexpected end of file\n");

      return false;
    }
  }

  return true;
}

// Read a variable-length value.
static bool ReadVariableLength(unsigned int *result, midimem_t *mf) {
  int i;
  unsigned char b;

  *result = 0;

  for (i = 0; i < 4; i++) {
    if (!ReadByte(&b, mf)) {
      D_MsgLocalWarn(
        "ReadVariableLength: Error while reading variable-length value\n"
      );
      return false;
    }

    // Insert the bottom seven bits from this byte.

    *result <<= 7;
    *result |= b & 0x7f;

    // If the top bit is not set, this is the end.

    if ((b & 0x80) == 0)
      return true;
  }

  D_MsgLocalWarn(
    "ReadVariableLength: Variable-length value too long: maximum of four bytes\n"
  );

  return false;
}

// Read a byte sequence into the data buffer.
static void* ReadByteSequence(unsigned int num_bytes, midimem_t *mf) {
  unsigned int i;
  unsigned char *result;

  // events can be length 0.  malloc(0) is not portable (can return NULL)
  if (!num_bytes)
    return malloc (4);

  // Allocate a buffer:

  result = malloc(num_bytes);

  if (result == NULL) {
    D_MsgLocalWarn("ReadByteSequence: Failed to allocate buffer %u bytes\n", num_bytes);
    return NULL;
  }

  // Read the data:
  for (i = 0; i < num_bytes; i++) {
    if (!ReadByte(&result[i], mf)) {
      D_MsgLocalWarn("ReadByteSequence: Error while reading byte %u\n", i);
      free(result);
      return NULL;
    }
  }

  return result;
}

// Read a MIDI channel event.
// two_param indicates that the event type takes two parameters
// (three byte) otherwise it is single parameter (two byte)
static bool ReadChannelEvent(midi_event_t *event, unsigned char event_type,
                                                  bool two_param,
                                                  midimem_t *mf) {
  unsigned char b;

  // Set basics:
  event->event_type = event_type & 0xf0;
  event->data.channel.channel = event_type & 0x0f;

  // Read parameters:
  if (!ReadByte(&b, mf)) {
    D_MsgLocalWarn(
      "ReadChannelEvent: Error while reading channel event parameters\n"
    );

    return false;
  }

  event->data.channel.param1 = b;

  if (!two_param)
    return true;

  // Second parameter:
  if (!ReadByte(&b, mf)) {
    D_MsgLocalWarn(
      "ReadChannelEvent: Error while reading channel event parameters\n"
    );

    return false;
  }

  event->data.channel.param2 = b;

  return true;
}

// Read sysex event:
static bool ReadSysExEvent(midi_event_t *event, int event_type,
                                                midimem_t *mf) {
  event->event_type = event_type;

  if (!ReadVariableLength(&event->data.sysex.length, mf)) {
    D_MsgLocalWarn("ReadSysExEvent: Failed to read length of SysEx block\n");

    return false;
  }

  // Read the byte sequence:

  event->data.sysex.data = ReadByteSequence(event->data.sysex.length, mf);

  if (!event->data.sysex.data) {
    D_MsgLocalWarn("ReadSysExEvent: Failed while reading SysEx event\n");

    return false;
  }

  return true;
}

// Read meta event:
static bool ReadMetaEvent(midi_event_t *event, midimem_t *mf) {
  unsigned char b;

  event->event_type = MIDI_EVENT_META;

  // Read meta event type:
  if (!ReadByte(&b, mf)) {
    D_MsgLocalWarn("ReadMetaEvent: Failed to read meta event type\n");
    return false;
  }

  event->data.meta.type = b;

  // Read length of meta event data:
  if (!ReadVariableLength(&event->data.meta.length, mf)) {
    D_MsgLocalWarn(
      "ReadMetaEvent: Failed to read length of MetaEvent block\n"
    );

    return false;
  }

  // Read the byte sequence:
  event->data.meta.data = ReadByteSequence(event->data.meta.length, mf);

  if (!event->data.meta.data) {
    D_MsgLocalWarn("ReadMetaEvent: Failed while reading MetaEvent\n");

    return false;
  }

  return true;
}

static bool ReadEvent(midi_event_t *event, unsigned int *last_event_type,
                                           midimem_t *mf) {
  unsigned char event_type;

  if (!ReadVariableLength(&event->delta_time, mf)) {
    D_MsgLocalWarn("ReadEvent: Failed to read event timestamp\n");

    return false;
  }

  if (!ReadByte(&event_type, mf)) {
    D_MsgLocalWarn("ReadEvent: Failed to read event type\n");

    return false;
  }

  // All event types have their top bit set.  Therefore, if the top bit is
  // not set, it is because we are using the "same as previous event type"
  // shortcut to save a byte.  Skip back a byte so that we read this byte
  // again.

  if ((event_type & 0x80) == 0) {
    event_type = *last_event_type;
    mf->pos--;
  }
  else {
    *last_event_type = event_type;
  }

  // Check event type:
  switch (event_type & 0xf0) {
    // Two parameter channel events:
    case MIDI_EVENT_NOTE_OFF:
    case MIDI_EVENT_NOTE_ON:
    case MIDI_EVENT_AFTERTOUCH:
    case MIDI_EVENT_CONTROLLER:
    case MIDI_EVENT_PITCH_BEND:
      return ReadChannelEvent(event, event_type, true, mf);
    // Single parameter channel events:
    case MIDI_EVENT_PROGRAM_CHANGE:
    case MIDI_EVENT_CHAN_AFTERTOUCH:
      return ReadChannelEvent(event, event_type, false, mf);
    default:
    break;
  }

  // Specific value?

  switch (event_type) {
    case MIDI_EVENT_SYSEX:
    case MIDI_EVENT_SYSEX_SPLIT:
      return ReadSysExEvent(event, event_type, mf);
    case MIDI_EVENT_META:
      return ReadMetaEvent(event, mf);
    default:
      break;
  }

  D_MsgLocalWarn("ReadEvent: Unknown MIDI event type: 0x%x\n", event_type);

  return false;
}

// Free an event:
static void FreeEvent(midi_event_t *event) {
  // Some event types have dynamically allocated buffers assigned to them that
  // must be freed.
  switch (event->event_type) {
    case MIDI_EVENT_SYSEX:
    case MIDI_EVENT_SYSEX_SPLIT:
      free(event->data.sysex.data);
    break;

    case MIDI_EVENT_META:
      free(event->data.meta.data);
    break;

    default:
      // Nothing to do.
    break;
  }
}

// Read and check the track chunk header

static bool ReadTrackHeader(midi_track_t *track, midimem_t *mf) {
  size_t records_read;
  chunk_header_t chunk_header;

  records_read = ReadMultipleBytes(&chunk_header, sizeof(chunk_header_t), mf);

  if (records_read < 1)
    return false;

  if (!CheckChunkHeader(&chunk_header, TRACK_CHUNK_ID))
    return false;

  track->data_len = ntohl(chunk_header.chunk_size);

  return true;
}

static bool ReadTrack(midi_track_t *track, midimem_t *mf) {
  midi_event_t *new_events = NULL;
  midi_event_t *event;
  unsigned int last_event_type = 0;

  track->num_events = 0;
  track->events = NULL;
  track->num_event_mem = 0; // NSM

  // Read the header:
  if (!ReadTrackHeader(track, mf))
    return false;

  // Then the events:
  while (true) {
    // Resize the track slightly larger to hold another event:
    // depending on the state of the heap and the malloc implementation, realloc()
    // one more event at a time can be VERY slow.  10sec+ in MSVC
    if (track->num_events == track->num_event_mem) {
      track->num_event_mem += 100; 
      new_events = realloc (track->events, sizeof (midi_event_t) * track->num_event_mem);
    }

    if (new_events == NULL)
      return false;

    track->events = new_events;

    // Read the next event:
    event = &track->events[track->num_events];
    if (!ReadEvent(event, &last_event_type, mf))
      return false;

    track->num_events++;

    // End of track?
    if (event->event_type == MIDI_EVENT_META &&
        event->data.meta.type == MIDI_META_END_OF_TRACK) {
      break;
    }
  }

  return true;
}

// Free a track:
static void FreeTrack(midi_track_t *track) {
  for (unsigned int i = 0; i < track->num_events; i++)
    FreeEvent(&track->events[i]);

  free(track->events);
}

static bool ReadAllTracks(midi_file_t *file, midimem_t *mf) {
  // Allocate list of tracks and read each track:
  file->tracks = malloc(sizeof(midi_track_t) * file->num_tracks);

  if (!file->tracks)
    return false;

  memset(file->tracks, 0, sizeof(midi_track_t) * file->num_tracks);

  // Read each track:
  for (unsigned int i = 0; i < file->num_tracks; i++) {
    if (!ReadTrack(&file->tracks[i], mf))
      return false;
  }

  return true;
}

// Read and check the header chunk.

static bool ReadFileHeader(midi_file_t *file, midimem_t *mf) {
  size_t records_read;
  unsigned int format_type;

  records_read = ReadMultipleBytes(&file->header, sizeof(midi_header_t), mf);

  if (records_read < 1)
    return false;

  if (!CheckChunkHeader(&file->header.chunk_header, HEADER_CHUNK_ID) ||
      ntohl(file->header.chunk_header.chunk_size) != 6) {
#ifdef _WIN32
    D_MsgLocalWarn(
      "ReadFileHeader: Invalid MIDI chunk header! chunk_size=%ld\n",
      ntohl(file->header.chunk_header.chunk_size)
    );
#else
    D_MsgLocalWarn(
      "ReadFileHeader: Invalid MIDI chunk header! chunk_size=%d\n",
      ntohl(file->header.chunk_header.chunk_size)
    );
#endif
    return false;
  }

  format_type = ntohs(file->header.format_type);
  file->num_tracks = ntohs(file->header.num_tracks);

  if ((format_type != 0 && format_type != 1) || file->num_tracks < 1) {
    D_MsgLocalWarn(
      "ReadFileHeader: Only type 0/1 MIDI files supported!\n"
    );
    return false;
  }

  // NSM
  file->header.time_division = ntohs(file->header.time_division);

  return true;
}

void MIDI_FreeFile(midi_file_t *file) {
  unsigned int i;

  if (file->tracks != NULL) {
    for (i = 0; i < file->num_tracks; i++)
      FreeTrack(&file->tracks[i]);

    free(file->tracks);
  }

  free(file);
}

midi_file_t* MIDI_LoadFile(midimem_t *mf) {
  midi_file_t *file;

  file = malloc(sizeof(midi_file_t));

  if (file == NULL)
    return NULL;

  file->tracks = NULL;
  file->num_tracks = 0;
  file->buffer = NULL;
  file->buffer_size = 0;

  // Read MIDI file header

  if (!ReadFileHeader(file, mf)) {
    MIDI_FreeFile(file);
    return NULL;
  }

  // Read all tracks:

  if (!ReadAllTracks(file, mf)) {
    MIDI_FreeFile(file);
    return NULL;
  }

  return file;
}

// Get the number of tracks in a MIDI file.

unsigned int MIDI_NumTracks(const midi_file_t *file) {
  return file->num_tracks;
}

// Start iterating over the events in a track.

midi_track_iter_t* MIDI_IterateTrack(const midi_file_t *file, unsigned int track) {
  midi_track_iter_t *iter;

  assert(track < file->num_tracks);

  iter = malloc(sizeof(*iter));
  iter->track = &file->tracks[track];
  iter->position = 0;

  return iter;
}

void MIDI_FreeIterator(midi_track_iter_t *iter) {
  free(iter);
}

// Get the time until the next MIDI event in a track.
unsigned int MIDI_GetDeltaTime(midi_track_iter_t *iter) {
  if (iter->position >= iter->track->num_events)
    return 0;

  return iter->track->events[iter->position].delta_time;
}

// Get a pointer to the next MIDI event.

int MIDI_GetNextEvent(midi_track_iter_t *iter, midi_event_t **event) {
  if (iter->position >= iter->track->num_events)
    return 0;

  *event = &iter->track->events[iter->position];
  ++iter->position;

  return 1;
}

unsigned int MIDI_GetFileTimeDivision(const midi_file_t *file) {
  return file->header.time_division;
}

void MIDI_RestartIterator(midi_track_iter_t *iter) {
  iter->position = 0;
}

// NSM: an alternate iterator tool.
midi_event_t **MIDI_GenerateFlatList(midi_file_t *file) {
  int totalevents = 0;
  unsigned int i, delta;
  int nextrk;

  int totaldelta = 0;

  int *trackpos = calloc(file->num_tracks, sizeof(int));
  int *tracktime = calloc(file->num_tracks, sizeof(int));
  int trackactive = file->num_tracks;

  midi_event_t **ret;
  midi_event_t **epos;

  for (i = 0; i < file->num_tracks; i++)
    totalevents += file->tracks[i].num_events;

  ret = malloc (totalevents * sizeof (midi_event_t **));

  epos = ret;

  while (trackactive) {
    unsigned int event_count;

    delta = 0x10000000;
    nextrk = -1;

    for (i = 0; i < file->num_tracks; i++) {
      int track_delta_time = file->tracks[i].events[trackpos[i]].delta_time;
      int track_delta = track_delta_time - tracktime[i];

      if (trackpos[i] != -1 && track_delta < delta) {
        delta = track_delta;
        nextrk = i;
      }
    }

    if (nextrk == -1) // unexpected EOF (not every track got end track)
      break;

    event_count = file->tracks[nextrk].num_events;

    *epos = file->tracks[nextrk].events + trackpos[nextrk];

    for (i = 0; i < file->num_tracks; i++) {
      if (i == (unsigned int)nextrk) {
        tracktime[i] = 0;
        trackpos[i]++;
      }
      else {
        tracktime[i] += delta;
      }
    }

    // yes, this clobbers the original timecodes
    epos[0]->delta_time = delta;
    totaldelta += delta;

    if (epos[0]->event_type == MIDI_EVENT_META &&
        epos[0]->data.meta.type == MIDI_META_END_OF_TRACK) {
      // change end of track into no op
      trackactive--;
      trackpos[nextrk] = -1;
      epos[0]->data.meta.type = MIDI_META_TEXT;
    }
    else if ((unsigned int)trackpos[nextrk] == event_count) {
      D_MsgLocalWarn("MIDI_GenerateFlatList: Unexpected end of track\n");

      free(trackpos);
      free(tracktime);
      free(ret);

      return NULL;
    }
    epos++;
  }
  
  if (trackactive) {
    // unexpected EOF
    D_MsgLocalWarn("MIDI_GenerateFlatList: Unexpected end of midi file\n");

    free(trackpos);
    free(tracktime);
    free(ret);

    return NULL;
  }
  
  // last end of track event is preserved though
  epos[-1]->data.meta.type = MIDI_META_END_OF_TRACK;

  free(trackpos);
  free(tracktime);
  
  if (totaldelta < 100) {
    D_MsgLocalWarn("MIDI_GeneratFlatList: very short file %i\n", totaldelta);

    free(ret);

    return NULL;
  }

  //MIDI_PrintFlatListDBG (ret);

  return ret;
}



void MIDI_DestroyFlatList(midi_event_t **evs) {
  free (evs);
}

// NSM: timing assist functions
// they compute samples per midi clock, where midi clock is the units
// that the time deltas are in, and samples is an arbitrary unit of which
// "sndrate" of them occur per second

// returns samples per midi clock
//
// inputs: mpq (midi clocks per quarternote, from header)
// tempo (from tempo event, in microseconds per quarternote)
// sndrate (sound sample rate in hz)
//
// samples   quarternote     microsec    samples    second
// ------- = ----------- * ----------- * ------- * --------
// midiclk     midiclk     quarternote   second    microsec
//
// return  =  (1 / mpq)  *    tempo    * sndrate * (1 / 1000000)
static double compute_spmc_normal(unsigned int mpq, unsigned int tempo,
                                                    unsigned int sndrate) {
  return (double)tempo / 1000000 * sndrate / mpq;
}

// returns samples per midi clock
//
// inputs: smpte_fps (24, 25, 29, 30)
// mpf (midi clocks per frame, 0-255)
// sndrate (sound sample rate in hz)
//
// tempo is ignored here
//
// samples     frame      seconds    samples
// ------- = --------- * --------- * -------
// midiclk    midiclk      frame     second
//
// return  = (1 / mpf) * (1 / fps) * sndrate
static double compute_spmc_smpte (unsigned int smpte_fps,
                                  unsigned int mpf,
                                  unsigned int sndrate) {
  double fps; // actual frames per second

  switch (smpte_fps) {
    case 24:
    case 25:
    case 30:
      fps = smpte_fps;
    break;
    case 29:
      // i hate NTSC, i really do
      fps = smpte_fps * 1000.0 / 1001.0;
    break;
    default:
      D_MsgLocalWarn("MIDI_spmc: Unexpected SMPTE timestamp %i\n", smpte_fps);
      // assume
      fps = 30.0;
    break;
  }

  return (double)sndrate / fps / mpf;
}

// if event is NULL, compute with default starting tempo (120BPM)

double MIDI_spmc(const midi_file_t *file, const midi_event_t *ev,
                                          unsigned int sndrate) {
  int smpte_fps;
  unsigned int tempo;
  unsigned int headerval = MIDI_GetFileTimeDivision (file);

  // SMPTE
  if (headerval & 0x8000) {
    // i don't really have any files to test this on...
    smpte_fps = -(short) headerval >> 8;

    return compute_spmc_smpte(smpte_fps, headerval & 0xff, sndrate);
  }

  // normal timing
  tempo = 500000; // default 120BPM

  if (ev) {
    if (ev->event_type == MIDI_EVENT_META) {
      if (ev->data.meta.length == 3) {
        tempo = (unsigned int) ev->data.meta.data[0] << 16 |
                (unsigned int) ev->data.meta.data[1] << 8  |
                (unsigned int) ev->data.meta.data[2];
      }
      else {
        D_MsgLocalWarn(
          "MIDI_spmc: wrong length tempo meta message in midi file\n"
        );
      }
    }
    else {
      D_MsgLocalWarn("MIDI_spmc: passed non-meta event\n");
    }
  }

  return compute_spmc_normal(headerval, tempo, sndrate);
}

/*
 * The timing system used by the OPL driver is very interesting. But there are
 * too many edge cases in multitrack (type 1) midi tempo changes that it simply
 * can't handle without a major rework.  The alternative is that we recook the
 * file into a single track file with no tempo changes at load time.
 */

midi_file_t* MIDI_LoadFileSpecial(midimem_t *mf) {
  midi_event_t **flatlist;
  midi_file_t *base = MIDI_LoadFile(mf);
  midi_file_t *ret;
  
  double opi;

  int epos = 0;

  if (!base)
    return NULL;

  flatlist = MIDI_GenerateFlatList (base);
  if (!flatlist) {
    MIDI_FreeFile (base);
    return NULL;
  }

  ret = malloc(sizeof(midi_file_t));

  ret->header.format_type = 0;
  ret->header.num_tracks = 1;
  ret->header.time_division = 10000;
  ret->num_tracks = 1;
  ret->buffer_size = 0;
  ret->buffer = NULL;
  ret->tracks = malloc (sizeof (midi_track_t));

  ret->tracks->num_events = 0;
  ret->tracks->num_event_mem = 0;
  ret->tracks->events = NULL;

  opi = MIDI_spmc (base, NULL, 20000);


  while (true) {
    midi_event_t *oldev;
    midi_event_t *nextev;

    if (ret->tracks->num_events == ret->tracks->num_event_mem) { 
      ret->tracks->num_event_mem += 100; 
      ret->tracks->events = realloc (ret->tracks->events, sizeof (midi_event_t) * ret->tracks->num_event_mem);
    }

    oldev = flatlist[epos];
    nextev = ret->tracks->events + ret->tracks->num_events;


    // figure delta time
    nextev->delta_time = (unsigned int)(opi * oldev->delta_time);

    // opl player can't process any sysex...
    if (oldev->event_type == MIDI_EVENT_SYSEX ||
        oldev->event_type == MIDI_EVENT_SYSEX_SPLIT) {
      epos++;
      continue;
    }

    if (oldev->event_type == MIDI_EVENT_META) {
      if (oldev->data.meta.type == MIDI_META_SET_TEMPO) {
        // adjust future tempo scaling
        opi = MIDI_spmc (base, oldev, 20000);
        // insert event as dummy
        nextev->event_type = MIDI_EVENT_META;
        nextev->data.meta.type = MIDI_META_TEXT;
        nextev->data.meta.length = 0;
        nextev->data.meta.data = malloc (4);
        epos++;
        ret->tracks->num_events++;

        continue;
      }

      if (oldev->data.meta.type == MIDI_META_END_OF_TRACK) {
        // reproduce event and break
        nextev->event_type = MIDI_EVENT_META;
        nextev->data.meta.type = MIDI_META_END_OF_TRACK;
        nextev->data.meta.length = 0;
        nextev->data.meta.data = malloc (4);
        epos++;
        ret->tracks->num_events++;

        break;
      }

      // other meta events not needed
      epos++;
      continue;
    }

    // non meta events can simply be copied (excluding delta time)
    memcpy(&nextev->event_type,
      &oldev->event_type,
      sizeof (midi_event_t) - sizeof (unsigned int)
    );
    epos++;
    ret->tracks->num_events++;
  }

  MIDI_DestroyFlatList(flatlist);
  MIDI_FreeFile(base);
  return ret;
}

#ifdef MIDITEST

static char* MIDI_EventTypeToString(midi_event_type_t event_type) {
  switch (event_type) {
    case MIDI_EVENT_NOTE_OFF:
      return "MIDI_EVENT_NOTE_OFF";
    case MIDI_EVENT_NOTE_ON:
      return "MIDI_EVENT_NOTE_ON";
    case MIDI_EVENT_AFTERTOUCH:
      return "MIDI_EVENT_AFTERTOUCH";
    case MIDI_EVENT_CONTROLLER:
      return "MIDI_EVENT_CONTROLLER";
    case MIDI_EVENT_PROGRAM_CHANGE:
      return "MIDI_EVENT_PROGRAM_CHANGE";
    case MIDI_EVENT_CHAN_AFTERTOUCH:
      return "MIDI_EVENT_CHAN_AFTERTOUCH";
    case MIDI_EVENT_PITCH_BEND:
      return "MIDI_EVENT_PITCH_BEND";
    case MIDI_EVENT_SYSEX:
      return "MIDI_EVENT_SYSEX";
    case MIDI_EVENT_SYSEX_SPLIT:
      return "MIDI_EVENT_SYSEX_SPLIT";
    case MIDI_EVENT_META:
      return "MIDI_EVENT_META";
    default:
      return "(unknown)";
  }
}

void PrintTrack(midi_track_t *track) {
  midi_event_t *event;
  unsigned int i;

  for (i = 0; i < track->num_events; i++) {
    event = &track->events[i];

    if (event->delta_time > 0)
      printf("Delay: %i ticks\n", event->delta_time);

    printf("Event type: %s (%i)\n",
      MIDI_EventTypeToString(event->event_type),
      event->event_type
    );

    switch(event->event_type) {
      case MIDI_EVENT_NOTE_OFF:
      case MIDI_EVENT_NOTE_ON:
      case MIDI_EVENT_AFTERTOUCH:
      case MIDI_EVENT_CONTROLLER:
      case MIDI_EVENT_PROGRAM_CHANGE:
      case MIDI_EVENT_CHAN_AFTERTOUCH:
      case MIDI_EVENT_PITCH_BEND:
        printf("\tChannel: %i\n", event->data.channel.channel);
        printf("\tParameter 1: %i\n", event->data.channel.param1);
        printf("\tParameter 2: %i\n", event->data.channel.param2);
      break;
      case MIDI_EVENT_SYSEX:
      case MIDI_EVENT_SYSEX_SPLIT:
        printf("\tLength: %i\n", event->data.sysex.length);
      break;
      case MIDI_EVENT_META:
        printf("\tMeta type: %i\n", event->data.meta.type);
        printf("\tLength: %i\n", event->data.meta.length);
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  FILE *f;
  midimem_t mf;
  midi_file_t *file;
  unsigned int i;

  if (argc < 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  f = fopen(argv[1], "rb");

  if (!f) {
    fprintf(stderr, "Failed to open %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  fseek(f, 0, SEEK_END);
  mf.len = ftell(f);
  mf.pos = 0;
  rewind(f);
  mf.data = malloc(mf.len);
  fread(mf.data, 1, mf.len, f);
  fclose(f);

  file = MIDI_LoadFile(&mf);

  if (file == NULL) {
    fprintf(stderr, "Failed to open %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < file->num_tracks; i++) {
    printf("\n== Track %i ==\n\n", i);
    PrintTrack(&file->tracks[i]);
  }

  return EXIT_SUCCESS;
}

#endif

/* vi: set et ts=2 sw=2: */

