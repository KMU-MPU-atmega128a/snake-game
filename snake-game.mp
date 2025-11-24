Project:      .\snake-game
Build Number: 10
Date:         Fri Nov 21 14:41:23 2025
Compiler:     ICCAVR
Version:      8.30.00
Device:       ATMega128

NOTE: AVR Code addresses are word addresses
All other addresses (including ones in FLASH) are byte addresses
  ROM 1% full. 893 out of 131072 bytes used (does not include absolute areas).
  RAM 61 bytes used (does not include stack usage).

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                           idata   00D4     3B =     59. bytes (rel,con,rom,lit)

       Addr  Global Symbol
      -----  --------------------------------
       00D4  __idata_start
       010F  __idata_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            text   0110    342 =    834. bytes (rel,con,rom,code)

       Addr  Global Symbol
      -----  --------------------------------
       0088  __text_start
       0088  __start
       00AA  _exit
       00AB  _InRange
       00BD  _snake_handler
       00CB  _ext_int0_isr
       00CC  _ext_int1_isr
       00CD  _ext_int7_isr
       00DC  _Interrupt_Init
       00E6  _main
       0105  _LCD_intro
       0131  _LCD_delay
       013A  _PortInit
       0140  _LCD_Data
       0161  _LCD_Comm
       0182  _LCD_Shift
       0194  _LCD_CHAR
       019D  _LCD_STR
       01AD  _LCD_pos
       01BD  _LCD_Clear
       01C2  _LCD_Init
       01DC  _Cursor_Home
       01E1  _delay_ms
       01F8  _delay_us
       020F  <LIBRARY>
       020F  empy16s
       021F  push_xgsetF000
       0224  pop_xgsetF000
       0229  __text_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                            data   0100     3B =     59. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       0100  __data_start
       0100  _flag
       0101  _snake
       0102  _x_max
       0103  _y_max
       0104  _pixel_x
       0105  _pixel_y
       0106  _head_x
       0107  _head_y
       0108  _tail_x
       0109  _tail_y
       010A  _dir_num
       010B  _dx
       010F  _dy
       013B  __data_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                             bss   013B      2 =      2. bytes (rel,con,ram)

       Addr  Global Symbol
      -----  --------------------------------
       013B  _ny
       013B  __bss_start
       013C  _nx
       013D  __bss_end

Area                               Addr   Size   Decimal Bytes (Attributes)
--------------------------------   ----   ----   ------- ----- ------------
                          vector   0004     24 =     36. bytes (abs,ovr,rom,code)

Files Linked      [ module(s) ]

C:\iccv8avr\lib\crtatmega.o	[ crtatmega.s ]
.\.objs\main.o	[ main.c ]
.\.objs\utils\intro.o	[ intro.c ]
.\.objs\utils\lcd.o	[ lcd.c ]
.\.objs\utils\my128.o	[ my128.c ]
<library>	[ emul16s.s, gpushy.s ]

User Global Definitions

ram_end = 0x10ff
hwstk_size = 0x14

Area Definitions

-bfunc_lit:0xd4.0x1000
-bdata:0x100.0x10ff

Additional Flags
  -e:0x20000
