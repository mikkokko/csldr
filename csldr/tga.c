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

typedef union
{
	uint8 c8;
	color24 c24;
	uint32 c32;
} pixel_t;

// little endian only but this will never be built on powerpc
inline static pixel_t BGRToRGB(pixel_t x)
{
	pixel_t result;
	result.c32 = (x.c32 & 0xFF00FF00) | ((x.c32 & 0x00FF0000) >> 16) | ((x.c32 & 0x000000FF) << 16);
	return result;
}

// mikkotodo this sucks
inline static pixel_t ReadPixel(readBuffer_t *buffer, int pixelSize)
{
	if (buffer->offset + pixelSize > buffer->size)
		longjmp(readError, 1);

	pixel_t result;
	pixel_t *src = (pixel_t *)(buffer->data + buffer->offset);

	switch (pixelSize)
	{
	case 1:
		result.c8 = src->c8;
		break;

	case 3:
		result.c24 = src->c24;
		break;

	case 4:
		result.c32 = src->c32;
		break;

	default:
		assert(false);
		result.c32 = 0;
		break;
	}

	buffer->offset += pixelSize;
	return result;
}

inline static byte *WritePixel(byte *buffer, pixel_t pixel, int pixelSize)
{
	pixel_t *dest = (pixel_t *)buffer;

	switch (pixelSize)
	{
	case 1:
		dest->c8 = pixel.c8;
		break;

	case 3:
		dest->c24 = pixel.c24;
		*dest = BGRToRGB(*dest);
		break;

	case 4:
		dest->c32 = pixel.c32;
		*dest = BGRToRGB(*dest);
		break;

	default:
		assert(false);
		break;
	}

	return buffer + pixelSize;
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

	byte *data = (byte *)malloc(width * height * pixelSize);
	byte *dst = data + (height - 1) * width * pixelSize;

	if (imageType == 10 || imageType == 11)
	{
		for (int i = height - 1; i >= 0; i--)
		{
			for (int j = 0; j < width; )
			{
				int count = ReadByte(&read);

				if (count & 0x80)
				{
					count -= 127;

					pixel_t pixel = ReadPixel(&read, pixelSize);

					for (; count > 0; count--)
					{
						dst = WritePixel(dst, pixel, pixelSize);
						j++;
					}
				}
				else
				{
					count++;

					for (; count > 0; count--)
					{
						pixel_t pixel = ReadPixel(&read, pixelSize);
						dst = WritePixel(dst, pixel, pixelSize);
						j++;
					}
				}
			}

			dst -= width * (2 * pixelSize);
		}
	}
	else
	{
		for (int i = height - 1; i >= 0; i--)
		{
			for (int j = 0; j < width; j++)
			{
				pixel_t pixel = ReadPixel(&read, pixelSize);
				dst = WritePixel(dst, pixel, pixelSize);
			}

			dst -= width * (2 * pixelSize);
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
