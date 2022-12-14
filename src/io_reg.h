#ifndef GUARD_GBA_IO_REG_H
#define GUARD_GBA_IO_REG_H

extern unsigned char REG_BASE[];

// I/O register offsets

#define REG_OFFSET_SOUND1CNT_L 0x60
#define REG_OFFSET_NR10 0x60
#define REG_OFFSET_SOUND1CNT_H 0x62
#define REG_OFFSET_NR11 0x62
#define REG_OFFSET_NR12 0x63
#define REG_OFFSET_SOUND1CNT_X 0x64
#define REG_OFFSET_NR13 0x64
#define REG_OFFSET_NR14 0x65
#define REG_OFFSET_SOUND2CNT_L 0x68
#define REG_OFFSET_NR21 0x68
#define REG_OFFSET_NR22 0x69
#define REG_OFFSET_SOUND2CNT_H 0x6c
#define REG_OFFSET_NR23 0x6c
#define REG_OFFSET_NR24 0x6d
#define REG_OFFSET_SOUND3CNT_L 0x70
#define REG_OFFSET_NR30 0x70
#define REG_OFFSET_SOUND3CNT_H 0x72
#define REG_OFFSET_NR31 0x72
#define REG_OFFSET_NR32 0x73
#define REG_OFFSET_SOUND3CNT_X 0x74
#define REG_OFFSET_NR33 0x74
#define REG_OFFSET_NR34 0x75
#define REG_OFFSET_SOUND4CNT_L 0x78
#define REG_OFFSET_NR41 0x78
#define REG_OFFSET_NR42 0x79
#define REG_OFFSET_SOUND4CNT_H 0x7c
#define REG_OFFSET_NR43 0x7c
#define REG_OFFSET_NR44 0x7d
#define REG_OFFSET_SOUNDCNT_L 0x80
#define REG_OFFSET_NR50 0x80
#define REG_OFFSET_NR51 0x81
#define REG_OFFSET_SOUNDCNT_H 0x82
#define REG_OFFSET_SOUNDCNT_X 0x84
#define REG_OFFSET_NR52 0x84
#define REG_OFFSET_SOUNDBIAS 0x88
#define REG_OFFSET_SOUNDBIAS_L 0x88
#define REG_OFFSET_SOUNDBIAS_H 0x89
#define REG_OFFSET_WAVE_RAM0 0x90
#define REG_OFFSET_WAVE_RAM1 0x94
#define REG_OFFSET_WAVE_RAM2 0x98
#define REG_OFFSET_WAVE_RAM3 0x9c
#define REG_OFFSET_FIFO_A 0xa0
#define REG_OFFSET_FIFO_B 0xa4

// I/O register addresses

#define REG_ADDR_SOUND1CNT_L (REG_BASE + REG_OFFSET_SOUND1CNT_L)
#define REG_ADDR_NR10 (REG_BASE + REG_OFFSET_NR10)
#define REG_ADDR_SOUND1CNT_H (REG_BASE + REG_OFFSET_SOUND1CNT_H)
#define REG_ADDR_NR11 (REG_BASE + REG_OFFSET_NR11)
#define REG_ADDR_NR12 (REG_BASE + REG_OFFSET_NR12)
#define REG_ADDR_SOUND1CNT_X (REG_BASE + REG_OFFSET_SOUND1CNT_X)
#define REG_ADDR_NR13 (REG_BASE + REG_OFFSET_NR13)
#define REG_ADDR_NR14 (REG_BASE + REG_OFFSET_NR14)
#define REG_ADDR_SOUND2CNT_L (REG_BASE + REG_OFFSET_SOUND2CNT_L)
#define REG_ADDR_NR21 (REG_BASE + REG_OFFSET_NR21)
#define REG_ADDR_NR22 (REG_BASE + REG_OFFSET_NR22)
#define REG_ADDR_SOUND2CNT_H (REG_BASE + REG_OFFSET_SOUND2CNT_H)
#define REG_ADDR_NR23 (REG_BASE + REG_OFFSET_NR23)
#define REG_ADDR_NR24 (REG_BASE + REG_OFFSET_NR24)
#define REG_ADDR_SOUND3CNT_L (REG_BASE + REG_OFFSET_SOUND3CNT_L)
#define REG_ADDR_NR30 (REG_BASE + REG_OFFSET_NR30)
#define REG_ADDR_SOUND3CNT_H (REG_BASE + REG_OFFSET_SOUND3CNT_H)
#define REG_ADDR_NR31 (REG_BASE + REG_OFFSET_NR31)
#define REG_ADDR_NR32 (REG_BASE + REG_OFFSET_NR32)
#define REG_ADDR_SOUND3CNT_X (REG_BASE + REG_OFFSET_SOUND3CNT_X)
#define REG_ADDR_NR33 (REG_BASE + REG_OFFSET_NR33)
#define REG_ADDR_NR34 (REG_BASE + REG_OFFSET_NR34)
#define REG_ADDR_SOUND4CNT_L (REG_BASE + REG_OFFSET_SOUND4CNT_L)
#define REG_ADDR_NR41 (REG_BASE + REG_OFFSET_NR41)
#define REG_ADDR_NR42 (REG_BASE + REG_OFFSET_NR42)
#define REG_ADDR_SOUND4CNT_H (REG_BASE + REG_OFFSET_SOUND4CNT_H)
#define REG_ADDR_NR43 (REG_BASE + REG_OFFSET_NR43)
#define REG_ADDR_NR44 (REG_BASE + REG_OFFSET_NR44)
#define REG_ADDR_SOUNDCNT_L (REG_BASE + REG_OFFSET_SOUNDCNT_L)
#define REG_ADDR_NR50 (REG_BASE + REG_OFFSET_NR50)
#define REG_ADDR_NR51 (REG_BASE + REG_OFFSET_NR51)
#define REG_ADDR_SOUNDCNT_H (REG_BASE + REG_OFFSET_SOUNDCNT_H)
#define REG_ADDR_SOUNDCNT_X (REG_BASE + REG_OFFSET_SOUNDCNT_X)
#define REG_ADDR_NR52 (REG_BASE + REG_OFFSET_NR52)
#define REG_ADDR_SOUNDBIAS (REG_BASE + REG_OFFSET_SOUNDBIAS)
#define REG_ADDR_SOUNDBIAS_L (REG_BASE + REG_OFFSET_SOUNDBIAS_L)
#define REG_ADDR_SOUNDBIAS_H (REG_BASE + REG_OFFSET_SOUNDBIAS_H)
#define REG_ADDR_WAVE_RAM0 (REG_BASE + REG_OFFSET_WAVE_RAM0)
#define REG_ADDR_WAVE_RAM1 (REG_BASE + REG_OFFSET_WAVE_RAM1)
#define REG_ADDR_WAVE_RAM2 (REG_BASE + REG_OFFSET_WAVE_RAM2)
#define REG_ADDR_WAVE_RAM3 (REG_BASE + REG_OFFSET_WAVE_RAM3)
#define REG_ADDR_FIFO_A (REG_BASE + REG_OFFSET_FIFO_A)
#define REG_ADDR_FIFO_B (REG_BASE + REG_OFFSET_FIFO_B)

// I/O registers

#define REG_SOUND1CNT_L (*(volatile uint16_t *)REG_ADDR_SOUND1CNT_L)
#define REG_NR10 (*(volatile uint8_t *)REG_ADDR_NR10)
#define REG_SOUND1CNT_H (*(volatile uint16_t *)REG_ADDR_SOUND1CNT_H)
#define REG_NR11 (*(volatile uint8_t *)REG_ADDR_NR11)
#define REG_NR12 (*(volatile uint8_t *)REG_ADDR_NR12)
#define REG_SOUND1CNT_X (*(volatile uint16_t *)REG_ADDR_SOUND1CNT_X)
#define REG_NR13 (*(volatile uint8_t *)REG_ADDR_NR13)
#define REG_NR14 (*(volatile uint8_t *)REG_ADDR_NR14)
#define REG_SOUND2CNT_L (*(volatile uint16_t *)REG_ADDR_SOUND2CNT_L)
#define REG_NR21 (*(volatile uint8_t *)REG_ADDR_NR21)
#define REG_NR22 (*(volatile uint8_t *)REG_ADDR_NR22)
#define REG_SOUND2CNT_H (*(volatile uint16_t *)REG_ADDR_SOUND2CNT_H)
#define REG_NR23 (*(volatile uint8_t *)REG_ADDR_NR23)
#define REG_NR24 (*(volatile uint8_t *)REG_ADDR_NR24)
#define REG_SOUND3CNT_L (*(volatile uint16_t *)REG_ADDR_SOUND3CNT_L)
#define REG_NR30 (*(volatile uint8_t *)REG_ADDR_NR30)
#define REG_SOUND3CNT_H (*(volatile uint16_t *)REG_ADDR_SOUND3CNT_H)
#define REG_NR31 (*(volatile uint8_t *)REG_ADDR_NR31)
#define REG_NR32 (*(volatile uint8_t *)REG_ADDR_NR32)
#define REG_SOUND3CNT_X (*(volatile uint16_t *)REG_ADDR_SOUND3CNT_X)
#define REG_NR33 (*(volatile uint8_t *)REG_ADDR_NR33)
#define REG_NR34 (*(volatile uint8_t *)REG_ADDR_NR34)
#define REG_SOUND4CNT_L (*(volatile uint16_t *)REG_ADDR_SOUND4CNT_L)
#define REG_NR41 (*(volatile uint8_t *)REG_ADDR_NR41)
#define REG_NR42 (*(volatile uint8_t *)REG_ADDR_NR42)
#define REG_SOUND4CNT_H (*(volatile uint16_t *)REG_ADDR_SOUND4CNT_H)
#define REG_NR43 (*(volatile uint8_t *)REG_ADDR_NR43)
#define REG_NR44 (*(volatile uint8_t *)REG_ADDR_NR44)
#define REG_SOUNDCNT_L (*(volatile uint16_t *)REG_ADDR_SOUNDCNT_L)
#define REG_NR50 (*(volatile uint8_t *)REG_ADDR_NR50)
#define REG_NR51 (*(volatile uint8_t *)REG_ADDR_NR51)
#define REG_SOUNDCNT_H (*(volatile uint16_t *)REG_ADDR_SOUNDCNT_H)
#define REG_SOUNDCNT_X (*(volatile uint16_t *)REG_ADDR_SOUNDCNT_X)
#define REG_NR52 (*(volatile uint8_t *)REG_ADDR_NR52)
#define REG_SOUNDBIAS (*(volatile uint16_t *)REG_ADDR_SOUNDBIAS)
#define REG_SOUNDBIAS_L (*(volatile uint8_t *)REG_ADDR_SOUNDBIAS_L)
#define REG_SOUNDBIAS_H (*(volatile uint8_t *)REG_ADDR_SOUNDBIAS_H)
#define REG_WAVE_RAM0 (*(volatile uint32_t *)REG_ADDR_WAVE_RAM0)
#define REG_WAVE_RAM1 (*(volatile uint32_t *)REG_ADDR_WAVE_RAM1)
#define REG_WAVE_RAM2 (*(volatile uint32_t *)REG_ADDR_WAVE_RAM2)
#define REG_WAVE_RAM3 (*(volatile uint32_t *)REG_ADDR_WAVE_RAM3)
#define REG_FIFO_A (*(volatile uint32_t *)REG_ADDR_FIFO_A)
#define REG_FIFO_B (*(volatile uint32_t *)REG_ADDR_FIFO_B)

// I/O register fields

// SOUNDCNT_H
#define SOUND_CGB_MIX_QUARTER 0x0000
#define SOUND_CGB_MIX_HALF 0x0001
#define SOUND_CGB_MIX_FULL 0x0002
#define SOUND_A_MIX_HALF 0x0000
#define SOUND_A_MIX_FULL 0x0004
#define SOUND_B_MIX_HALF 0x0000
#define SOUND_B_MIX_FULL 0x0008
#define SOUND_ALL_MIX_FULL 0x000E
#define SOUND_A_RIGHT_OUTPUT 0x0100
#define SOUND_A_LEFT_OUTPUT 0x0200
#define SOUND_A_TIMER_0 0x0000
#define SOUND_A_TIMER_1 0x0400
#define SOUND_A_FIFO_RESET 0x0800
#define SOUND_B_RIGHT_OUTPUT 0x1000
#define SOUND_B_LEFT_OUTPUT 0x2000
#define SOUND_B_TIMER_0 0x0000
#define SOUND_B_TIMER_1 0x4000
#define SOUND_B_FIFO_RESET 0x8000

// SOUNDCNT_X
#define SOUND_1_ON 0x0001
#define SOUND_2_ON 0x0002
#define SOUND_3_ON 0x0004
#define SOUND_4_ON 0x0008
#define SOUND_MASTER_ENABLE 0x0080

#endif  // GUARD_GBA_IO_REG_H
