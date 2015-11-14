#include "jtag_tap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



const char *state_ids[] = {
        "jtag_tap_test_logic_reset",
        "jtag_tap_idle",
        "jtag_tap_select_dr_scan",
        "jtag_tap_select_ir_scan",
        "jtag_tap_capture_dr",
        "jtag_tap_shift_dr",
        "jtag_tap_exit1_dr",
        "jtag_tap_pause_dr",
        "jtag_tap_exit2_dr",
        "jtag_tap_update_dr",
        "jtag_tap_capture_ir",
        "jtag_tap_shift_ir",
        "jtag_tap_exit1_ir",
        "jtag_tap_pause_ir",
        "jtag_tap_exit2_ir",
        "jtag_tap_update_ir"
};


/*
 * initialize a tap controller
 */
struct jtag_tap *jtag_tap_init() {
        struct jtag_tap *tap;
        int i;

        tap=malloc(sizeof(struct jtag_tap));
        tap->ticks=0;
        tap->state=TAP_STATE_TEST_LOGIC_RESET;
        tap->nr_devices=0;
        memset(tap->buffer, 0, MAX_JTAG_BUFFER);

        for (i=0; i<100; i++) {
                tap->devices[i]=NULL;
        }

        return tap;
}

/*
 *  destroy tap controller
 */
void jtag_tap_destroy(struct jtag_tap *tap) {
        int i;

        for (i=0; i<100; i++) {
                if (tap->devices[i]!= NULL) {
                        free(tap->devices[i]);
                        tap->devices[i]=NULL;
                }
        }

        free(tap);
}

/*
 * get the current state
 */
int jtag_tap_get_state(struct jtag_tap *tap) {
        return tap->state;
}

const char *jtag_tap_get_state_id(struct jtag_tap *tap) {
        return state_ids[tap->state];
}

void jtag_shift_xilinx(struct jtag_tap *tap, const char *data, int nr_ticks) {
  int len=strlen(data);
  char tms;
  char tdi;
  char tck;
  char help1[2];
  char help2[2];
  int i;
  int ticks=0;
  char *temp_data;

  printf("found %d number of bytes\n",len);

  temp_data=malloc(len/2);
  for(i=0; i<len/2; i=i+1) {
    temp_data[i]=0;
    help1[0]=data[i*2];
    help1[1]='\0';

    help2[0]=data[i*2+1];
    help2[1]='\0';

    temp_data[i]=(char)strtol(help1,NULL,16);
    temp_data[i]=temp_data[i]<<4;
    temp_data[i]=temp_data[i] | ((char) strtol(help2,NULL,16));
    printf("%.2hhX\n",temp_data[i]);
  }


  // processing data starts on the right side
  for(i=0; i<len/2; i=i+2) {
    tms=(temp_data[i] >> 4) & 0x0f;
    printf("Data: %.2hhX, TMS: %.2hhX\n",temp_data[i],tms);
    tck=temp_data[i+1] & 0x0f;
    printf("Data: %.2hhX, TCK: %.2hhX\n",temp_data[i+1],tck);
    if (nr_ticks-ticks >= 4) {
      jtag_tap_shift(tap, &tms, &tdi, &tck, 4);
    } else {
      jtag_tap_shift(tap, &tms, &tdi, &tck, nr_ticks-ticks);
    }
  }

  free(temp_data);
}


void jtag_tap_shift(struct jtag_tap *tap, char *tms, char *tdi, char *tck, int nr_ticks) {
        int i;
        int byte_nr;
        int bit_nr;
        unsigned char i_tms;
        unsigned char i_tdi;
        unsigned char i_tck;

        for(i=0; i<nr_ticks; i++) {
                byte_nr=i/8;
                bit_nr=i-(byte_nr*8);

                printf("Byte: %d, Bit: %d\n", byte_nr, bit_nr);

                i_tdi= (tdi[byte_nr] >> bit_nr) & 0x01;
                i_tms= (tms[byte_nr] >> bit_nr) & 0x01;
                i_tck= (tck[byte_nr] >> bit_nr) & 0x01;

                printf("TMS: %hhd, TCK: %hhd\n",i_tms, i_tck );

                switch(tap->state) {
                case TAP_STATE_TEST_LOGIC_RESET:
                        if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_RUN_TEST_IDLE;
                        }
                        break;

                case TAP_STATE_RUN_TEST_IDLE:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_SELECT_DR_SCAN;
                        }
                        break;

                case TAP_STATE_SELECT_DR_SCAN:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_SELECT_IR_SCAN;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_CAPTURE_DR;
                        }

                        break;

                case TAP_STATE_SELECT_IR_SCAN:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_TEST_LOGIC_RESET;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_CAPTURE_IR;
                        }
                        break;
                case TAP_STATE_CAPTURE_DR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_EXIT1_DR;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_SHIFT_DR;
                        }
                        break;

                case TAP_STATE_SHIFT_DR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_EXIT1_DR;
                        }
                        break;

                case TAP_STATE_EXIT1_DR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_UPDATE_DR;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_PAUSE_DR;
                        }
                        break;

                case TAP_STATE_PAUSE_DR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_EXIT2_DR;
                        }
                        break;

                case TAP_STATE_EXIT2_DR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_UPDATE_DR;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_SHIFT_DR;
                        }
                       break;

                case TAP_STATE_UPDATE_DR:
                      if (i_tck == 1 && i_tms == 1) {
                              tap->state = TAP_STATE_SELECT_DR_SCAN;
                      } else if (i_tck == 1 && i_tms == 0) {
                              tap->state = TAP_STATE_RUN_TEST_IDLE;
                      }
                      break;
                case TAP_STATE_CAPTURE_IR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_EXIT1_IR;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_SHIFT_IR;
                        }
                        break;

                case TAP_STATE_SHIFT_IR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_EXIT1_IR;
                        }
                        break;

                case TAP_STATE_EXIT1_IR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_UPDATE_IR;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_PAUSE_IR;
                        }
                        break;

                case TAP_STATE_PAUSE_IR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_EXIT2_IR;
                        }
                        break;

                case TAP_STATE_EXIT2_IR:
                        if (i_tck == 1 && i_tms == 1) {
                                tap->state = TAP_STATE_UPDATE_IR;
                        } else if (i_tck == 1 && i_tms == 0) {
                                tap->state = TAP_STATE_SHIFT_IR;
                        }
                       break;

                case TAP_STATE_UPDATE_IR:
                      if (i_tck == 1 && i_tms == 1) {
                              tap->state = TAP_STATE_SELECT_DR_SCAN;
                      } else if (i_tck == 1 && i_tms == 0) {
                              tap->state = TAP_STATE_RUN_TEST_IDLE;
                      }
                      break;
          }


        }




}
