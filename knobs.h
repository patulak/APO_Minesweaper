#ifndef KNOBS_H
#define KNOBS_H

//Shifted values of knobs and their clicks.
#define RED_KNOB 16 
#define RED_CLICK 26

#define GREEN_KNOB 8 
#define GREEN_CLICK 25

#define BLUE_KNOB 0 
#define BLUE_CLICK 24

typedef struct
{
    int red_change;
    int blue_change;
    int green_change;

    bool red_click;
    bool green_click;
    bool blue_click;
} rotation_t;

typedef struct
{
  uint8_t red_current;
  uint8_t green_current;
  uint8_t blue_current;

  uint8_t red_previous;
  uint8_t green_previous;
  uint8_t blue_previous;
} Knob;

/*
 * @brief Gets int value of position of knob.
 * @param mem_base Pointer to memory mapping.
 * @return Value of knob (0 - 255)
*/
uint32_t get_knobs(unsigned char *mem_base);

/*
 * @brief Gets change of knob.
 * @param lastRotation Pointer to memory with previous value of knob.
 * @param mem_base Pointer to memory mapping.
 * @return Structure with informations about direction of change and clicks.
*/
rotation_t get_knob_change(int *lastRotation, unsigned char* mem_base);

void set_default_settings(rotation_t *result);

#endif /*KNOBS_H*/