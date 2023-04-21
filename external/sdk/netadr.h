typedef enum {
  NA_UNUSED = 0,
  NA_LOOPBACK = 1,
  NA_BROADCAST = 2,
  NA_IP = 3,
  NA_IPX = 4,
  NA_BROADCAST_IPX = 5
} netadrtype_t;

typedef struct
{
  netadrtype_t type;
  unsigned char ip[4];
  unsigned char ipx[10];
  unsigned short port;
} netadr_t;
