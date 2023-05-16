#ifndef KEYVALUE_H
#define KEYVALUE_H

typedef enum
{
	keyValueKey,
	keyValueString
} keyValueType_t;

typedef struct keyValue_s
{
	char *name;
	keyValueType_t type;

	union
	{
		struct keyValue_s *subkeys;
		char *string;
	};

	int numSubkeys;
} keyValue_t;

bool KeyValueParse(keyValue_t *kv, char *data);
void KeyValueFree(keyValue_t *kv);

#endif // KEYVALUE_H
