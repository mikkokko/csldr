typedef struct
{
	unsigned char *data;
	int size;
	int ofs;
} msg_read_t;

void Msg_ReadInit(msg_read_t *ctx, void *data, int size);
int Msg_ReadByte(msg_read_t *ctx);
int Msg_ReadChar(msg_read_t *ctx);
float Msg_ReadAngle(msg_read_t *ctx);
float Msg_ReadCoord(msg_read_t *ctx);
