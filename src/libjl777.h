void init_jl777();
char *process_jl777_msg(CNode* from,char *msg,int32_t duration);
extern "C" int libjl777_start(char *JSON_or_fname);
extern "C" char *libjl777_JSON(char *JSONstr);
extern "C" int32_t libjl777_broadcast(char *msg,int32_t duration);
extern "C" char *libjl777_gotpacket(char *msg,int32_t duration);


