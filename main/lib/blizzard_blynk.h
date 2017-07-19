#ifndef BLIZZARD_BLYNK_H
#define BLIZZARD_BLYNK_H


#ifdef __cplusplus
extern "C" {
#endif

#define BLYNK_TIMER_INTERVAL 1000L //in ms

void start_blynk(void);
void blizzard_timer_event(void);
void update_blynk_wdmx_led(void);

#ifdef __cplusplus
}
#endif

#endif // ifndef  DMX_ARTNET_H
