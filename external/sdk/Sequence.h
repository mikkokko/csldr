typedef struct
{
  int effect;
  byte r1;
  byte g1;
  byte b1;
  byte a1;
  byte r2;
  byte g2;
  byte b2;
  byte a2;
  float x;
  float y;
  float fadein;
  float fadeout;
  float holdtime;
  float fxtime;
  const char* pName;
  const char* pMessage;
} client_textmessage_t;

typedef struct sequenceCommandLine_ {
  int commandType;
  client_textmessage_t clientMessage;
  char* speakerName;
  char* listenerName;
  char* soundFileName;
  char* sentenceName;
  char* fireTargetNames;
  char* killTargetNames;
  float delay;
  int repeatCount;
  int textChannel;
  int modifierBitField;
  struct sequenceCommandLine_* nextCommandLine;
} sequenceCommandLine_s;

typedef struct sequenceEntry_ {
  char* fileName;
  char* entryName;
  sequenceCommandLine_s* firstCommand;
  struct sequenceEntry_* nextEntry;
  qboolean isGlobal;
} sequenceEntry_s;
