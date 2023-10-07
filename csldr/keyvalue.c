#include "pch.h"

#define DEFAULT_SUBKEY_COUNT 4

// can't remember why this was done but ok
#define Q_ISSPACE(c) (((c) == ' ') || ((unsigned int)(c) - '\t' < 5))

static char *SkipWhite(char *s)
{
	while (1)
	{
		while (*s && Q_ISSPACE(*s))
			s++;

		if (s[0] == '/' && s[1] == '/')
		{
			s += 2;

			while (*s && *s != '\r' && *s != '\n')
				s++;
		}
		else
		{
			break;
		}
	}

	return s;
}

// this rapes the input buffer so we don't have to allocate memory
static char *ParseString(char *s, char **dest)
{
	char *start = s;

	if (*s == '"')
	{
		s++;
		start++;

		while (*s && *s != '"')
			s++;
	}
	else
	{
		while (*s && !Q_ISSPACE(*s))
			s++;
	}

	*dest = start;

	if (*s == '\0')
		return s;

	*s = '\0';
	return s + 1;
}

static char *RecursiveParse(keyValue_t *kv, char *s)
{
	int capacity = DEFAULT_SUBKEY_COUNT;

	kv->type = keyValueKey;
	kv->subkeys = (keyValue_t *)Mem_TempAlloc(sizeof(*kv->subkeys) * capacity);
	kv->numSubkeys = 0;

	s = SkipWhite(s);

	while (*s)
	{
		keyValue_t *subkey;

		switch (*s)
		{
		case '}':
			s++;
			return s;

		default:
			if (kv->numSubkeys >= capacity)
			{
				capacity *= 2;
				kv->subkeys = (keyValue_t *)realloc(kv->subkeys, sizeof(*kv->subkeys) * capacity);
			}

			subkey = &kv->subkeys[kv->numSubkeys];
			memset(subkey, 0, sizeof(*subkey));

			s = ParseString(s, &subkey->name);
			if (!*subkey->name)
				return NULL;
			break;
		}

		s = SkipWhite(s);

		switch (*s)
		{
		case '{':
			s++;
			s = RecursiveParse(subkey, s);
			if (!s)
				return NULL;
			break;

		default:
			subkey->type = keyValueString;
			s = ParseString(s, &subkey->string);
			if (!*subkey->string)
				return NULL;
			break;
		}

		s = SkipWhite(s);
		kv->numSubkeys++;
	}

	return s;
}

bool KeyValueParse(keyValue_t *kv, char *s)
{
	kv->name = NULL;
	return RecursiveParse(kv, s) ? true : false;
}

void KeyValueFree(keyValue_t *kv)
{
	if (kv->type == keyValueString)
		return; // no memory allocated

	for (int i = 0; i < kv->numSubkeys; i++)
		KeyValueFree(&kv->subkeys[i]);

	Mem_TempFree(kv->subkeys);
}
