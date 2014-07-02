// non-pin constants and compile-time settings

/**
 * Power states
 */
// most time is spent in this state
// all devices turned off, uC is in deepest sleepstate
// only interrupts from the light button or the rtc can wake the system up
#define POWERSTATE_SLEEP 1

// state for background-operation
// not visible for user, everything seems to be turned off
// system wakes up from time to time for maintenance tasks (dfc..)
// all devices except rtc are turned off, uC is running
#define POWERSTATE_ON 2

// state for user interaction
// display is on and user can press all buttons
// enabled: uC, rtc, display
#define POWERSTATE_DISPLAY 3

// state for playing music (aka ringing)
// enabled: uC, rtc, display, amplifier, sdcard
#define POWERSTATE_RINGING 4

// state for receiving the time signal
// not visible for user
// everything turned off except dcf and uC
#define POWERSTATE_DCF 5


/**
 * Reasons for wakeup
 */
#define WAKEREASON_LIGHT 1 // user pressed the light button
#define WAKEREASON_TIMER 2 // timer by rtc

#define DISPLAY_ADDRESS1 0x71
