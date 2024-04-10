#include "pch.h"

// mikkotodo make this 1024
#define MAX_TGA_DIM 2048

static const char *error_string;

// mikkotodo merge with msg.c, essentially the same thing
typedef struct
{
	byte *data;
	int size;
	int offset;
} readBuffer_t;

static jmp_buf readError;

inline static void ReadBegin(readBuffer_t *buffer, void *data, int size)
{
	buffer->data = (byte *)data;
	buffer->size = size;
	buffer->offset = 0;
}

inline static void ReadSeek(readBuffer_t *buffer, int offset)
{
	if (buffer->offset + offset > buffer->size)
		longjmp(readError, 1);

	buffer->offset += offset;
}

inline static void ReadData(readBuffer_t *buffer, void *dest, int size)
{
	if (buffer->offset + size > buffer->size)
		longjmp(readError, 1);

	memcpy(dest, buffer->data + buffer->offset, size);
	buffer->offset += size;
}

inline static byte ReadByte(readBuffer_t *buffer)
{
	if (buffer->offset + 1 > buffer->size)
		longjmp(readError, 1);

	byte value = *(byte *)(buffer->data + buffer->offset);
	buffer->offset += 1;
	return value;
}

inline static unsigned short ReadShort(readBuffer_t *buffer)
{
	if (buffer->offset + 2 > buffer->size)
		longjmp(readError, 1);

	unsigned short value = *(unsigned short *)(buffer->data + buffer->offset);
	buffer->offset += 2;
	return value;
}

static byte *LoadFromBuffer(void *buffer, int size, int *pwidth, int *pheight, int *pcomp)
{
	readBuffer_t read;
	ReadBegin(&read, buffer, size);

	// read the header
	int idLength = ReadByte(&read);
	int colorMapType = ReadByte(&read);
	int imageType = ReadByte(&read);
	int colorMapFirstEntry = ReadShort(&read);
	int colorMapLength = ReadShort(&read);
	int colorMapEntrySize = ReadByte(&read);
	int xOrigin = ReadShort(&read);
	int yOrigin = ReadShort(&read);
	int width = ReadShort(&read);
	int height = ReadShort(&read);
	int pixelDepth = ReadByte(&read);
	int imageDescriptor = ReadByte(&read);

	// validate the header
	if (colorMapType)
	{
		error_string = "Color mapped images are not supported";
		return NULL;
	}

	if (imageType != 2 && imageType != 10 && imageType != 3 && imageType != 11)
	{
		error_string = "Only true-color and grayscale images are supported";
		return NULL;
	}

	(void)colorMapFirstEntry;
	(void)colorMapLength;
	(void)colorMapEntrySize;

	if (xOrigin || yOrigin)
	{
		error_string = "Image origin is not supported";
		return NULL;
	}

	if (width * height > (MAX_TGA_DIM * MAX_TGA_DIM))
	{
		// mikkotodo make this 1024x
		error_string = "Image is too large, maximum size is 2048x2048 pixels";
		return NULL;
	}

	if (pixelDepth != 8 && pixelDepth != 24 && pixelDepth != 32)
	{
		error_string = "Only 8, 24 and 32 bit TGAs are supported";
		return NULL;
	}

	int validDescriptor = (pixelDepth == 32) ? 8 : 0;

	// mikkotodo handle this better? flipping etc
	if (imageDescriptor != validDescriptor)
	{
		error_string = "Unsupported image descriptor";
		return NULL;
	}

	ReadSeek(&read, idLength);

	int pixelSize = pixelDepth >> 3;
	assert(pixelSize == 1 || pixelSize == 3 || pixelSize == 4);

	byte *data = (byte *)Mem_TempAlloc(width * height * pixelSize);
	byte *dst = data + (height - 1) * width * pixelSize;

	if (imageType == 10 || imageType == 11)
	{
		for (int i = height - 1; i >= 0; i--)
		{
			int count;

			for (int j = 0; j < width; j += count)
			{
				count = ReadByte(&read);

				if (count & 0x80)
				{
					int k;

					count -= 0x7F;

					byte pixel[4]; // max pixel size is 4
					ReadData(&read, &pixel, pixelSize);

					switch (pixelSize)
					{
					case 1:
						memset(dst, pixel[0], count);
						dst += count;
						break;

					case 3:
						for (k = 0; k < count; k++)
						{
							*(color24 *)dst = *(color24 *)pixel;
							dst += sizeof(color24);
						}
						break;

					case 4:
						for (k = 0; k < count; k++)
						{
							*(uint32 *)dst = *(uint32 *)pixel;
							dst += sizeof(uint32);
						}
						break;

					default:
						assert(false);
						break;
					}
				}
				else
				{
					count++;
					ReadData(&read, dst, count * pixelSize);
					dst += (count * pixelSize);
				}		
			}

			dst -= width * (2 * pixelSize);
		}
	}
	else
	{
		for (int i = height - 1; i >= 0; i--)
		{
			ReadData(&read, dst, width * pixelSize);
			dst -= width * pixelSize;
		}
	}

	// convert bgr to rgb
	if (pixelSize == 3 || pixelSize == 4)
	{
		int count = width * height * pixelSize;

		for (int i = 0; i < count; i += pixelSize)
		{
			byte temp = data[i];
			data[i] = data[i + 2];
			data[i + 2] = temp;
		}
	}

	*pwidth = width;
	*pheight = height;
	*pcomp = pixelSize;

	return data;
}

byte *TgaLoad(char *path, int *pwidth, int *pheight, int *pcomp)
{
	error_string = NULL;

	int size;
	byte *file = gEngfuncs.COM_LoadFile(path, 5, &size);
	if (!file)
	{
		error_string = "File not found";
		return NULL;
	}

	if (setjmp(readError))
	{
		error_string = "Corrupted TGA file";
		gEngfuncs.COM_FreeFile(file);
		return NULL;
	}

	byte *data = LoadFromBuffer(file, size, pwidth, pheight, pcomp);

	// if data is null, error_string was set by LoadFromBuffer

	gEngfuncs.COM_FreeFile(file);
	return data;
}

const char *TgaLoadError(void)
{
	if (error_string)
		return error_string;
	return "No error";
}
