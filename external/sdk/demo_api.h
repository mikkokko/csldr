typedef struct
{
  int (*IsRecording)(void);
  int (*IsPlayingback)(void);
  int (*IsTimeDemo)(void);
  void (*WriteBuffer)(int, unsigned char*);
} demo_api_t;
