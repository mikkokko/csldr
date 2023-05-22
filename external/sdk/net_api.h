struct net_response_s; /* no body */

typedef void (*net_api_response_func_t)(struct net_response_s*);

typedef struct net_response_s {
  int error;
  int context;
  int type;
  netadr_t remote_address;
  double ping;
  void* response;
} net_response_t;

typedef struct
{
  int connected;
  netadr_t local_address;
  netadr_t remote_address;
  int packet_loss;
  double latency;
  double connection_time;
  double rate;
} net_status_t;

typedef struct
{
  void (*InitNetworking)(void);
  void (*Status)(net_status_t*);
  void (*SendRequest)(int, int, int, double, netadr_t*, net_api_response_func_t);
  void (*CancelRequest)(int);
  void (*CancelAllRequests)(void);
  sdk_string_const char* (*AdrToString)(netadr_t*);
  int (*CompareAdr)(netadr_t*, netadr_t*);
  int (*StringToAdr)(sdk_string_const char*, netadr_t*);
  const char* (*ValueForKey)(const char*, const char*);
  void (*RemoveKey)(sdk_string_const char*, const char*);
  void (*SetValueForKey)(sdk_string_const char*, const char*, const char*, int);
} net_api_t;
