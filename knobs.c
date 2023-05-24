#include <stdint.h>
#include <stdbool.h>

#include "mzapo_regs.h"
#include "knobs.h"

uint32_t get_knobs(unsigned char *mem_base)
{  
    volatile uint32_t* knobs = (volatile uint32_t*)(mem_base + SPILED_REG_KNOBS_8BIT_o);
    return *knobs;
}

rotation_t get_knob_change(int *lastRotation, unsigned char* mem_base)
{ 
    rotation_t result;
    Knob knob;

    uint32_t currentRotation = get_knobs(mem_base);

    set_default_settings(&result);
    
    //Previous values of knobs.
    knob.red_previous = (*lastRotation >> RED_KNOB) & 0xFF;
    knob.green_previous = (*lastRotation >> GREEN_KNOB) & 0xFF;
    knob.blue_previous = (*lastRotation >> BLUE_KNOB) & 0xFF;

    //Current values of knobs.
    knob.red_current = (currentRotation >> RED_KNOB) & 0xFF;
    knob.green_current = (currentRotation >> GREEN_KNOB) & 0xFF;
    knob.blue_current = (currentRotation >> BLUE_KNOB) & 0xFF;

    //Clicks of knobs.
    result.red_click = (currentRotation >> RED_CLICK & 0x1) == 1;
    result.green_click = (currentRotation >> GREEN_CLICK & 0x1) == 1;
    result.blue_click = (currentRotation >> BLUE_CLICK & 0x1) == 1;

    //Red knob
    if ((knob.red_current - 3) > knob.red_previous)
    {
      if ((knob.red_current - knob.red_previous) <= 128)
      {
        result.red_change = 1;
        *lastRotation += (4 << RED_KNOB);
      }
      else
      {
        result.red_change = -1;
        *lastRotation -= (4 << RED_KNOB);
      }
    }
    else if ((knob.red_current + 3) < knob.red_previous)
    {
      if ((knob.red_previous - knob.red_current) <= 128)
      {
        result.red_change = -1;
        *lastRotation -= (4 << RED_KNOB);
      }
      else
      {
        result.red_change = 1;
        *lastRotation += (4 << RED_KNOB);
      }
    }
    else
    {
      result.red_change = 0;
    }

    //Green knob
    if ((knob.green_current - 3) > knob.green_previous)
    {
      if ((knob.green_current - knob.green_previous) <= 128)
      {
        result.green_change = 1;
        *lastRotation += (4 << GREEN_KNOB);
      }
      else
      {
        result.green_change = -1;
        *lastRotation -= (4 << GREEN_KNOB);
      }
    }
    else if ((knob.green_current + 3) < knob.green_previous)
    {
      if ((knob.green_previous - knob.green_current) <= 128)
      {
        result.green_change = -1;
        *lastRotation -= (4 << GREEN_KNOB);
      }
      else
      {
        result.green_change = 1;
        *lastRotation += (4 << GREEN_KNOB);
      }
    }
    else
    {
      result.green_change = 0;
    }

    //Blue knob
    if ((knob.blue_current - 3) > knob.blue_previous)
    {
      if ((knob.blue_current - knob.blue_previous) <= 128)
      {
        result.blue_change = 1;
        *lastRotation += (4 << BLUE_KNOB);
      }
      else
      {
        result.blue_change = -1;
        *lastRotation -= (4 << BLUE_KNOB);
      }
    }
    else if ((knob.blue_current + 3) < knob.blue_previous)
    {
      if ((knob.blue_previous - knob.blue_current) <= 128)
      {
        result.blue_change = -1;
        *lastRotation -= (4 << BLUE_KNOB);
      }
      else
      {
        result.blue_change = 1;
        *lastRotation += (4 << BLUE_KNOB);
      }
    }
    else
    {
      result.blue_change = 0;
    }

    return result;
}

void set_default_settings(rotation_t *result)
{
    result->red_click = false; 
    result->red_change = 0;

    result->green_click = false;
    result->green_change = 0;

    result->blue_click = false;
    result->blue_change = 0;
}