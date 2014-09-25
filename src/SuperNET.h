#define MAX_PUBADDR_TIME (24 * 60 * 60)

void init_jl777(char *myip);
char *process_jl777_msg(CNode* from,char *msg,int32_t duration);
extern "C" int SuperNET_start(char *JSON_or_fname,char *myip);
extern "C" char *SuperNET_JSON(char *JSONstr);
extern "C" int32_t SuperNET_broadcast(char *msg,int32_t duration);
extern "C" char *SuperNET_gotpacket(char *msg,int32_t duration,char *from_ip_port);
extern "C" int32_t SuperNET_narrowcast(char *destip,unsigned char *msg,int32_t len);
extern "C" int32_t got_newpeer(const char *ip_port);