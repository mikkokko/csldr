#include "pch.h"

void Msg_ReadInit(msg_read_t *ctx, void *data, int size, jmp_buf *error)
{
	ctx->data = (byte *)data;
	ctx->size = size;
	ctx->ofs = 0;
	ctx->error = error;
}

bool Msg_ReadSeek(msg_read_t *ctx, int size)
{
	if (ctx->ofs + size > ctx->size)
	{
		if (ctx->error)
			longjmp(*ctx->error, 1);
		return false;
	}

	ctx->ofs += size;
	return true;
}

bool Msg_ReadData(msg_read_t *ctx, void *buffer, int size)
{
	if (ctx->ofs + size > ctx->size)
	{
		if (ctx->error)
			longjmp(*ctx->error, 1);
		return false;
	}

	memcpy(buffer, &ctx->data[ctx->ofs], size);
	ctx->ofs += size;
	return true;
}

int Msg_ReadByte(msg_read_t *ctx)
{
	if (ctx->ofs + 1 > ctx->size)
	{
		if (ctx->error)
			longjmp(*ctx->error, 1);
		return 0;
	}

	return ctx->data[ctx->ofs++];
}

int Msg_ReadChar(msg_read_t *ctx)
{
	if (ctx->ofs + 1 > ctx->size)
	{
		if (ctx->error)
			longjmp(*ctx->error, 1);
		return 0;
	}

	return (signed char)ctx->data[ctx->ofs++];
}

short Msg_ReadShort(msg_read_t *ctx)
{
	if (ctx->ofs + 2 > ctx->size)
	{
		if (ctx->error)
			longjmp(*ctx->error, 1);
		return 0;
	}

	int lo = ctx->data[ctx->ofs++];
	int hi = ctx->data[ctx->ofs++];

	return (short)(lo | (hi << 8));
}

float Msg_ReadAngle(msg_read_t *ctx)
{
	return (float)Msg_ReadChar(ctx) * (360.0f / 256);
}

float Msg_ReadCoord(msg_read_t *ctx)
{
	return (float)Msg_ReadShort(ctx) * (1.0f / 8);
}

int Msg_ReadString(msg_read_t *ctx, char *buffer, int size)
{
	int offset = 0;

	while (true)
	{
		int ch = Msg_ReadChar(ctx);
		if (!ch)
			break;

		if (offset < size - 1)
			buffer[offset++] = (char)ch;
	}

	buffer[offset] = '\0';
	return offset;
}
