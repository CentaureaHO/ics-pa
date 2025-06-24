#ifndef NATIVE_MIDI_H
#define NATIVE_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

// Dummy implementation for compatibility
typedef struct NativeMidiSong NativeMidiSong;

static inline NativeMidiSong* native_midi_loadsong(const char* file) { return NULL; }
static inline void            native_midi_freesong(NativeMidiSong* song) {}
static inline void            native_midi_start(NativeMidiSong* song) {}
static inline void            native_midi_stop(void) {}
static inline int             native_midi_active(void) { return 0; }
static inline void            native_midi_setvolume(int volume) {}
static inline const char*     native_midi_error(void) { return "Not implemented"; }

#ifdef __cplusplus
}
#endif

#endif