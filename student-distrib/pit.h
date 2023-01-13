extern int pit_curr_terminal;
extern int curr_esp, curr_ebp;
extern void schedule_terminal();
extern int open_terminals;
volatile int piano;

void pit_switch_terminal();
extern void pit_init ();

 //Play sound using built in speaker
extern void play_sound(uint32_t nFrequence);
 
 //make it shutup
extern void nosound();

  //Make a beep
extern void beep();

extern void note(int freq, int time);


//Play sound using built in speaker
extern void timer_wait(int count);
