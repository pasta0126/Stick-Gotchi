#pragma once
#include <stdint.h>

// Per-creature animation ID constants.
// IDs are uint8_t indices into each creature's own sprite/anim table.
// Values here are placeholders — the renderer maps them to actual frame sequences
// when creature sprites are authored.

namespace ByteeAnim {
    constexpr uint8_t IDLE              =  0;  // default standing
    constexpr uint8_t LOOK_AROUND       =  1;  // glances left/right, curious
    constexpr uint8_t READ_BOOK         =  2;  // tiny book appears, reads
    constexpr uint8_t SLEEP             =  3;  // ZZZ, book falls
    constexpr uint8_t STARTLED_CURIOUS  =  4;  // jump → settle → curious head tilt
    constexpr uint8_t TUMBLE_DIZZY      =  5;  // tumbles, visor scrambles
    constexpr uint8_t PICKUP_JOY        =  6;  // floats up, eyes glow bright
    constexpr uint8_t SET_DOWN          =  7;  // lands, looks around
    constexpr uint8_t WONDER_WALK       =  8;  // bobs while being carried
    constexpr uint8_t TILT_ANALYZE      =  9;  // head tilts, analyzing sound
    constexpr uint8_t SCARED_VISOR      = 10;  // jump, visor cracks briefly
    constexpr uint8_t HAPPY_SPARK       = 11;  // bounces, chest gem glows
    constexpr uint8_t SURPRISE_BTN      = 12;  // "!" bubble, then happy
    constexpr uint8_t ATTENTION_FLOAT   = 13;  // floats up, waves
    constexpr uint8_t ATTENTION_SPARK   = 14;  // casts small spark
    constexpr uint8_t ATTENTION_ANTENNA = 15;  // antenna blinks frantically
}

namespace CthulhuAnim {
    constexpr uint8_t IDLE              =  0;  // tentacles slow-wiggle, stares
    constexpr uint8_t TENTACLE_WIGGLE   =  1;  // idle variant — tentacles sway
    constexpr uint8_t CURL_SLEEP        =  2;  // curls up, wings fold, eldritch glow
    constexpr uint8_t TENTACLE_FLAIL    =  3;  // confused flail after tap
    constexpr uint8_t PANIC_TENTACLES   =  4;  // full panic, eyes spin
    constexpr uint8_t CLING_PICKUP      =  5;  // clings with all tentacles
    constexpr uint8_t WALKING_CONTENT   =  6;  // sways, tentacles drag
    constexpr uint8_t COSMIC_ABSORB     =  7;  // eyes glow, absorbs sound
    constexpr uint8_t ELDRITCH_SURGE    =  8;  // glows green, then collapses
    constexpr uint8_t HUGS_REACH        =  9;  // reaches out with tentacles
    constexpr uint8_t STARTLED_EYES     = 10;  // wide eyes snap open
    constexpr uint8_t CUBE_OFFER        = 11;  // startled then offers artifact
    constexpr uint8_t ATTENTION_SIGN    = 12;  // holds up FREE HUGS sign
    constexpr uint8_t ATTENTION_REACH   = 13;  // tentacles reach toward screen edge
    constexpr uint8_t ATTENTION_GLOW    = 14;  // eerie pulse glow
}

namespace JackAnim {
    constexpr uint8_t IDLE              =  0;  // just sits there
    constexpr uint8_t SLOW_BLINK        =  1;  // one deliberate slow blink
    constexpr uint8_t HAT_TILT_ANNOYED  =  2;  // hat tilts, back to normal
    constexpr uint8_t HAT_BOB           =  3;  // hat bobs very slightly
    constexpr uint8_t TINY_EXCLAMATION  =  4;  // small "!" fades immediately
    constexpr uint8_t TINY_HEART        =  5;  // small heart, gone in 1s
    constexpr uint8_t HAT_MICRO_LIFT    =  6;  // hat lifts 2px, returns
    constexpr uint8_t SLEEP_ZZZ         =  7;  // barely-visible ZZZ
}

namespace LumiAnim {
    constexpr uint8_t IDLE              =  0;  // tail wag, sparkle
    constexpr uint8_t TAIL_WAG          =  1;  // idle variant — enthusiastic tail
    constexpr uint8_t EARS_PERK         =  2;  // ears perk up, listening
    constexpr uint8_t CURL_SLEEP        =  3;  // antlers dim, soft glow
    constexpr uint8_t JUMP_EXCITED      =  4;  // jumps, big eyes
    constexpr uint8_t CURL_SCARED       =  5;  // curls up, tail hides
    constexpr uint8_t PICKUP_SPARKLES   =  6;  // tail spins, sparkle burst
    constexpr uint8_t HAPPY_BOUNCE      =  7;  // bounces to walking rhythm
    constexpr uint8_t LISTEN_TILT       =  8;  // head tilts, intent listen
    constexpr uint8_t STARTLED_JUMP     =  9;  // startles, then indignant look
    constexpr uint8_t NUZZLE_HEART      = 10;  // nuzzle + sparkle heart
    constexpr uint8_t PAWS_PLAYFUL      = 11;  // paws at screen, playful
    constexpr uint8_t ATTENTION_BOUNCE  = 12;  // bounces to screen edge
    constexpr uint8_t ATTENTION_FLASH   = 13;  // antlers flash
    constexpr uint8_t ATTENTION_EYES    = 14;  // maximum puppy eyes
}
