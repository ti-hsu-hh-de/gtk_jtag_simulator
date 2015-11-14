#ifndef __JTAG_TAP_H__
#define __JTAG_TAP_H__

#define TAP_STATE_TEST_LOGIC_RESET  0
#define TAP_STATE_RUN_TEST_IDLE     1
#define TAP_STATE_SELECT_DR_SCAN    2
#define TAP_STATE_SELECT_IR_SCAN    3
#define TAP_STATE_CAPTURE_DR        4
#define TAP_STATE_SHIFT_DR          5
#define TAP_STATE_EXIT1_DR          6
#define TAP_STATE_PAUSE_DR          7
#define TAP_STATE_EXIT2_DR          8
#define TAP_STATE_UPDATE_DR         9
#define TAP_STATE_CAPTURE_IR        10
#define TAP_STATE_SHIFT_IR          11
#define TAP_STATE_EXIT1_IR          12
#define TAP_STATE_PAUSE_IR          13
#define TAP_STATE_EXIT2_IR          14
#define TAP_STATE_UPDATE_IR         15

#define MAX_JTAG_BUFFER             200
#define JTAG_STATES_NR              16

extern const char *state_ids[];

/*
 * structure representing a generic jtag device
 *
 */
struct jtag_device {
    int id;
    int ir_size;
    char *dr;
    char *ir;
};

/* structure representing the state of the JTAG TAP Controller*/
struct jtag_tap {
  int ticks;
  int nr_devices;
  struct jtag_device *devices[100];
  char buffer[MAX_JTAG_BUFFER];
  unsigned char state;

};




struct jtag_tap *jtag_tap_init();
void jtag_tap_destroy();
int jtag_tap_get_state(struct jtag_tap *tap);
const char *jtag_tap_get_state_id(struct jtag_tap *tap);
void jtag_tap_shift(struct jtag_tap *tap, char *tms, char *tdi, char *tck, int nr_ticks);
void jtag_shift_xilinx(struct jtag_tap *tap, const char *data, int nr_ticks);
#endif
