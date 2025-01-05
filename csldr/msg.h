typedef struct
{
	unsigned char *data;
	int size;
	int ofs;
	jmp_buf *error;
} msg_read_t;

void Msg_ReadInit(msg_read_t *ctx, void *data, int size, jmp_buf *error);
bool Msg_ReadSeek(msg_read_t *ctx, int size);
bool Msg_ReadData(msg_read_t *ctx, void *buffer, int size);
int Msg_ReadByte(msg_read_t *ctx);
int Msg_ReadChar(msg_read_t *ctx);
short Msg_ReadShort(msg_read_t *ctx);
float Msg_ReadAngle(msg_read_t *ctx);
float Msg_ReadCoord(msg_read_t *ctx);
int Msg_ReadString(msg_read_t *ctx, char *buffer, int size);
