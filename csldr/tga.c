#include "pch.h"

// FIXME: use stb_image instead of this old crusty crap

// mikkotodo make this 1024
#define MAX_TGA_DIM 2048

static const char *error_string;

static byte *LoadFromBuffer(void *buffer, int size, jmp_buf *error, int *pwidth, int *pheight, int *pcomp)
{
	msg_read_t read;
	Msg_ReadInit(&read, buffer, size, error);

	// read the header
	int idLength = Msg_ReadByte(&read);
	int colorMapType = Msg_ReadByte(&read);
	int imageType = Msg_ReadByte(&read);
	int colorMapFirstEntry = Msg_ReadShort(&read);
	int colorMapLength = Msg_ReadShort(&read);
	int colorMapEntrySize = Msg_ReadByte(&read);
	int xOrigin = Msg_ReadShort(&read);
	int yOrigin = Msg_ReadShort(&read);
	int width = Msg_ReadShort(&read);
	int height = Msg_ReadShort(&read);
	int pixelDepth = Msg_ReadByte(&read);
	int imageDescriptor = Msg_ReadByte(&read);

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

	Msg_ReadSeek(&read, idLength);

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
				count = Msg_ReadByte(&read);

				if (count & 0x80)
				{
					int k;

					count -= 0x7F;

					byte pixel[4]; // max pixel size is 4
					Msg_ReadData(&read, &pixel, pixelSize);

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
					Msg_ReadData(&read, dst, count * pixelSize);
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
			Msg_ReadData(&read, dst, width * pixelSize);
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

	jmp_buf readError;
	if (setjmp(readError))
	{
		error_string = "Corrupted TGA file";
		gEngfuncs.COM_FreeFile(file);
		return NULL;
	}

	byte *data = LoadFromBuffer(file, size, &readError, pwidth, pheight, pcomp);

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
