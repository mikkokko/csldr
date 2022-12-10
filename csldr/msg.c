#include "pch.h"

void Msg_ReadInit(msg_read_t *ctx, void *data, int size)
{
	ctx->data = (byte *)data;
	ctx->size = size;
	ctx->ofs = 0;
}

int Msg_ReadByte(msg_read_t *ctx)
{
	int val;

	if (ctx->ofs + (int)sizeof(byte) > ctx->size)
		return 0;

	val = *(byte *)(&ctx->data[ctx->ofs]);
	ctx->ofs += sizeof(byte);

	return val;
}

int Msg_ReadChar(msg_read_t *ctx)
{
	int val;

	if (ctx->ofs + (int)sizeof(char) > ctx->size)
		return 0;

	val = *(char *)(&ctx->data[ctx->ofs]);
	ctx->ofs += sizeof(char);

	return val;
}

float Msg_ReadAngle(msg_read_t *ctx)
{
	int val;

	if (ctx->ofs + (int)sizeof(char) > ctx->size)
		return 0;

	val = *(char *)(&ctx->data[ctx->ofs]);
	ctx->ofs += (int)sizeof(char);

	return (float)val * (360.0f / 256);
}

float Msg_ReadCoord(msg_read_t *ctx)
{
	int val;

	if (ctx->ofs + (int)sizeof(short) > ctx->size)
		return 0;

	val = *(short *)(&ctx->data[ctx->ofs]);
	ctx->ofs += (int)sizeof(short);

	return (float)val * (1.0f / 8);
}
