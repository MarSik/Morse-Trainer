#ifndef __MT_sine_MS_20120309_
#define __MT_sine_MS_20120309_

void sine_init(uint16_t pitch);
void sine_deinit(void);
void sine_start(void);
void sine_stop(void);

void inline disable_sine_int(void)
{
    TIMSK &= ~_BV(OCIE0A);
}

void inline enable_sine_int(void)
{
    TIMSK |= _BV(OCIE0A);
}


#endif /* __MT_sine_MS_20120309_ */
