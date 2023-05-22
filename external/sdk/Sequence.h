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
  sdk_string_const char* speakerName;
  sdk_string_const char* listenerName;
  sdk_string_const char* soundFileName;
  sdk_string_const char* sentenceName;
  sdk_string_const char* fireTargetNames;
  sdk_string_const char* killTargetNames;
  float delay;
  int repeatCount;
  int textChannel;
  int modifierBitField;
  struct sequenceCommandLine_* nextCommandLine;
} sequenceCommandLine_s;

typedef struct sequenceEntry_ {
  sdk_string_const char* fileName;
  sdk_string_const char* entryName;
  sequenceCommandLine_s* firstCommand;
  struct sequenceEntry_* nextEntry;
  qboolean isGlobal;
} sequenceEntry_s;
