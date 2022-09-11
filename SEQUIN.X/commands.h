// Single-byte commands
#define SYN_PLAYBACK_START 0x7F
#define SYN_PLAYBACK_STOP 0x80
#define SYN_REC_START 0x81
#define SYN_REC_END 0x82
#define SYN_OCTAVE_UP 0x83
#define SYN_OCTAVE_DOWN 0x84

// Two-byte commands, always followed by data byte
// The sequencer processor does not need to know the functionality of these commands,
// but the definition table is kept here for reference
#define SYN_SET_ATTACK 0x85
#define SYN_SET_DECAY 0x86
#define SYN_SET_SUSTAIN 0x87
#define SYN_SET_LFO1_FREQ 0x88
#define SYN_SET_LFO1_MAG 0x89
#define SYN_SET_LFO2_FREQ 0x8A
#define SYN_SET_LFO2_MAG 0x8B
#define SYN_SET_FLANG_FREQ 0x8C
#define SYN_SET_FLANG_MAG 0x8D
#define SYN_SET_DISTORT 0x8E
#define SYN_SET_REVERB_DELAY 0x8F
#define SYN_SET_REVERB_MAG 0x90
#define SYN_SET_GAIN 0x91
#define SYN_SET_KEY 0x92
#define SYN_CLEAR_KEY 0x93
#define SYN_SET_BASS 0x94 // Not implemented
#define SYN_SET_OCTAVE 0x95
#define SYN_STARTUP_DONE 0x96