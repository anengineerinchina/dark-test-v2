//
//  contacts.h
//  libjl777
//
//  Created by jl777 on 10/15/14.
//  Copyright (c) 2014 jl777. All rights reserved.
//


#ifndef contacts_h
#define contacts_h

struct telepathy_args
{
    uint64_t mytxid,othertxid,refaddr,bestaddr,refaddrs[8],otheraddrs[8];
    bits256 mypubkey,otherpubkey;
    int numrefs;
};

struct contact_info
{
    bits256 pubkey,shared;
    char handle[64];
    uint64_t nxt64bits;
    int32_t numsent,numrecv;
} *Contacts;
int32_t Num_contacts,Max_contacts;
portable_mutex_t Contacts_mutex;

struct contact_info *_find_handle(char *handle)
{
    int32_t i;
    if ( Num_contacts != 0 )
    {
        for (i=0; i<Num_contacts; i++)
            if ( strcmp(Contacts[i].handle,handle) == 0 )
                return(&Contacts[i]);
    }
    return(0);
}

struct contact_info *_find_contact_nxt64bits(uint64_t nxt64bits)
{
    int32_t i;
    if ( Num_contacts != 0 )
    {
        for (i=0; i<Num_contacts; i++)
            if ( Contacts[i].nxt64bits == nxt64bits )
                return(&Contacts[i]);
    }
    return(0);
}

struct contact_info *_find_contact(char *contactstr)
{
    int32_t len;
    uint64_t nxt64bits = 0;
    struct contact_info *contact = 0;
    if ( (contact= _find_handle(contactstr)) == 0 )
    {
        if ( (len= is_decimalstr(contactstr)) > 0 && len < 22 )
            nxt64bits = calc_nxt64bits(contactstr);
        else if ( strncmp("NXT-",contactstr,4) == 0 )
            nxt64bits = conv_rsacctstr(contactstr,0);
        contact = _find_contact_nxt64bits(nxt64bits);
    }
    return(contact);
}

struct contact_info *find_contact(char *handle)
{
    struct contact_info *contact = 0;
    portable_mutex_lock(&Contacts_mutex);
    contact = _find_contact(handle);
    portable_mutex_unlock(&Contacts_mutex);
    return(contact);
}

char *addcontact(struct sockaddr *prevaddr,char *NXTaddr,char *NXTACCTSECRET,char *sender,char *handle,char *acct)
{
    bits256 mysecret,mypublic;
    struct coin_info *cp = get_coin_info("BTCD");
    struct contact_info *contact;
    char retstr[1024],pubkeystr[128],sharedstr[128];
    if ( cp == 0 )
    {
        printf("addcontact: no BTCD cp?\n");
        return(0);
    }
    handle[sizeof(contact->handle)-1] = 0;
    portable_mutex_lock(&Contacts_mutex);
    if ( (contact= _find_contact(handle)) == 0 )
    {
        if ( Num_contacts >= Max_contacts )
        {
            Max_contacts = (Num_contacts + 1 );
            Contacts = realloc(Contacts,(sizeof(*Contacts) * Max_contacts));
        }
        contact = &Contacts[Num_contacts++];
        safecopy(contact->handle,handle,sizeof(contact->handle));
    }
    else if ( strcmp(handle,"myhandle") == 0 )
       return(clonestr("{\"error\":\"cant override myhandle\"}"));
    contact->nxt64bits = conv_rsacctstr(acct,0);
    contact->pubkey = issue_getpubkey(acct);
    init_hexbytes(pubkeystr,contact->pubkey.bytes,sizeof(contact->pubkey));
    if ( contact->pubkey.txid == 0 )
        sprintf(retstr,"{\"error\":\"(%s) acct.(%s) has no pubkey.(%s)\"}",handle,acct,pubkeystr);
    else
    {
        conv_NXTpassword(mysecret.bytes,mypublic.bytes,cp->privateNXTACCTSECRET);
        contact->shared = curve25519(mysecret,contact->pubkey);
        init_hexbytes(sharedstr,contact->shared.bytes,sizeof(contact->shared));
        printf("shared.(%s)\n",sharedstr);
        sprintf(retstr,"{\"result\":\"(%s) acct.(%s) (%llu) has pubkey.(%s)\"}",handle,acct,(long long)contact->nxt64bits,pubkeystr);
    }
    portable_mutex_unlock(&Contacts_mutex);
    printf("ADD.(%s -> %s)\n",handle,acct);
    return(clonestr(retstr));
}

char *removecontact(struct sockaddr *prevaddr,char *NXTaddr,char *NXTACCTSECRET,char *sender,char *handle)
{
    struct contact_info *contact;
    char retstr[1024];
    handle[sizeof(contact->handle)-1] = 0;
    if ( strcmp("myhandle",handle) == 0 )
        return(0);
    portable_mutex_lock(&Contacts_mutex);
    if ( (contact= _find_contact(handle)) != 0 )
    {
        if ( contact != &Contacts[--Num_contacts] )
        {
            *contact = Contacts[Num_contacts];
            memset(&Contacts[Num_contacts],0,sizeof(Contacts[Num_contacts]));
        }
        if ( Num_contacts == 0 )
        {
            Max_contacts = 0;
            free(Contacts);
            Contacts = 0;
            printf("freed all contacts\n");
        }
        sprintf(retstr,"{\"result\":\"handle.(%s) deleted num.%d max.%d\"}",handle,Num_contacts,Max_contacts);
    } else sprintf(retstr,"{\"error\":\"handle.(%s) doesnt exist\"}",handle);
    portable_mutex_unlock(&Contacts_mutex);
    printf("REMOVE.(%s)\n",handle);
    return(clonestr(retstr));
}

void set_contactstr(char *contactstr,struct contact_info *contact)
{
    char pubkeystr[128],rsacctstr[128];
    rsacctstr[0] = 0;
    conv_rsacctstr(rsacctstr,contact->nxt64bits);
    if ( strcmp(contact->handle,"myhandle") == 0 )
        init_hexbytes(pubkeystr,Global_mp->mypubkey.bytes,sizeof(Global_mp->mypubkey));
    else init_hexbytes(pubkeystr,contact->pubkey.bytes,sizeof(contact->pubkey));
    sprintf(contactstr,"{\"result\":\"handle\":\"%s\",\"acct\":\"%s\",\"NXT\":\"%llu\",\"pubkey\":\"%s\"}",contact->handle,rsacctstr,(long long)contact->nxt64bits,pubkeystr);
}

char *dispcontact(struct sockaddr *prevaddr,char *NXTaddr,char *NXTACCTSECRET,char *sender,char *handle)
{
    int32_t i;
    struct contact_info *contact;
    char retbuf[1024],*retstr = 0;
    handle[sizeof(contact->handle)-1] = 0;
    retbuf[0] = 0;
    portable_mutex_lock(&Contacts_mutex);
    if ( strcmp(handle,"*") == 0 )
    {
        retstr = clonestr("[");
        for (i=0; i<Num_contacts; i++)
        {
            set_contactstr(retbuf,&Contacts[i]);
            retstr = realloc(retstr,strlen(retstr)+strlen(retbuf)+2);
            strcat(retstr,retbuf);
            if ( i < Num_contacts-1 )
                strcat(retstr,",");
        }
        strcat(retstr,"]");
    }
    else
    {
        if ( (contact= _find_contact(handle)) != 0 )
            set_contactstr(retbuf,contact);
        else sprintf(retbuf,"{\"error\":\"handle.(%s) doesnt exist\"}",handle);
        retstr = clonestr(retbuf);
    }
    portable_mutex_unlock(&Contacts_mutex);
    printf("Contact.(%s)\n",retstr);
    return(retstr);
}

double calc_nradius(uint64_t *addrs,int32_t n,uint64_t testaddr,double refdist)
{
    int32_t i;
    double dist,sum = 0.;
    if ( n == 0 )
        return(0.);
    for (i=0; i<n; i++)
    {
        dist = (bitweight(addrs[i] ^ testaddr) - refdist);
        sum += (dist * dist);
    }
    if ( sum < 0. )
        printf("huh? sum %f n.%d -> %f\n",sum,n,sqrt(sum/n));
    return(sqrt(sum/n));
}

int32_t Task_mindmeld(void *_args,int32_t argsize)
{
    static bits256 zerokey;
    struct telepathy_args *args = _args;
    int32_t i,j,iter,dist;
    double sum,metric,bestmetric;
    cJSON *json;
    uint64_t calcaddr;
    struct coin_info *cp = get_coin_info("BTCD");
    char key[64],datastr[1024],sender[64],otherkeystr[512],*retstr;
    if ( cp == 0 )
        return(-1);
    if ( memcmp(&args->otherpubkey,&zerokey,sizeof(zerokey)) == 0 )
    {
        expand_nxt64bits(key,args->othertxid);
        gen_randacct(sender);
        retstr = kademlia_find("findvalue",0,cp->srvNXTADDR,cp->srvNXTACCTSECRET,sender,key,0);
        if ( retstr != 0 )
        {
            if ( (json= cJSON_Parse(retstr)) != 0 )
            {
                copy_cJSON(datastr,cJSON_GetObjectItem(json,"data"));
                if ( strlen(datastr) == sizeof(zerokey)*2 )
                {
                    printf("set otherpubkey to (%s)\n",datastr);
                    decode_hex(args->otherpubkey.bytes,sizeof(args->otherpubkey),datastr);
                }
                free_json(json);
            }
            free(retstr);
        }
    }
    sum = 0.;
    for (i=0; i<args->numrefs; i++)
    {
        for (j=0; j<args->numrefs; j++)
        {
            if ( i == j )
                dist = bitweight(args->refaddr ^ args->refaddrs[j]);
            else
            {
                dist = bitweight(args->refaddrs[i] ^ args->refaddrs[j]);
                sum += dist;
            }
            printf("%2d ",dist);
        }
        printf("\n");
    }
    printf("dist from privateaddr above -> ");
    sum /= (args->numrefs * args->numrefs - args->numrefs);
    if ( args->bestaddr == 0 )
        randombytes((uint8_t *)&args->bestaddr,sizeof(args->bestaddr));
    bestmetric = calc_nradius(args->refaddrs,args->numrefs,args->bestaddr,(int)sum);
    printf("bestmetric %.3f avedist %.1f\n",bestmetric,sum);
    for (iter=0; iter<1000000; iter++)
    {
        //ind = (iter % 65);
        //if ( ind == 64 )
        if( (iter & 1) != 0 )
            randombytes((unsigned char *)&calcaddr,sizeof(calcaddr));
        else calcaddr = (args->bestaddr ^ (1L << ((rand()>>8)&63)));
        metric = calc_nradius(args->refaddrs,args->numrefs,calcaddr,(int)sum);
        if ( metric < bestmetric )
        {
            bestmetric = metric;
            args->bestaddr = calcaddr;
        }
    }
    for (i=0; i<args->numrefs; i++)
    {
        for (j=0; j<args->numrefs; j++)
        {
            if ( i == j )
                printf("%2d ",bitweight(args->bestaddr ^ args->refaddrs[j]));
            else printf("%2d ",bitweight(args->refaddrs[i] ^ args->refaddrs[j]));
        }
        printf("\n");
    }
    printf("bestaddr.%llu bestmetric %.3f\n",(long long)args->bestaddr,bestmetric);
    init_hexbytes(otherkeystr,args->otherpubkey.bytes,sizeof(args->otherpubkey));
    printf("Other pubkey.(%s)\n",otherkeystr);
    for (i=0; i<args->numrefs; i++)
        printf("%llu ",(long long)args->refaddrs[i]);
    printf("mytxid.%llu othertxid.%llu | myaddr.%llu\n",(long long)args->mytxid,(long long)args->othertxid,(long long)args->refaddr);
    return(0);
}

uint64_t calc_privatetx(bits256 *passp,struct contact_info *contact,int32_t sequenceid)
{
    static bits256 zerokey;
    char buf[512];
    bits256 location;
    if ( memcmp(&zerokey,&contact->shared,sizeof(zerokey)) == 0 )
        return(0);
    sprintf(buf,"%llx.%d",(long long)contact->nxt64bits,sequenceid);
    calc_sha256cat(location.bytes,(uint8_t *)buf,(int32_t)strlen(buf),contact->shared.bytes,(int32_t)sizeof(contact->shared));
    calc_sha256(0,passp->bytes,location.bytes,(int32_t)sizeof(location));
    return(location.txid);
}

uint8_t *AES_codec(int32_t *lenp,int32_t decryptflag,char *msg,uint8_t *data,int32_t *datalenp,char *name,char *password)
{
    int32_t i,*cipherids,len;
    char **privkeys,*decompressed;
    uint8_t *retdata,*combined = 0;
    struct compressed_json *compressed = 0;
    privkeys = gen_privkeys(&cipherids,name,password,GENESIS_SECRET,"");
    if ( decryptflag == 0 )
    {
        len = (int32_t)strlen(msg);
        if ( data != 0 && *datalenp > 0 )
        {
            combined = calloc(1,1 + len + *datalenp);
            memcpy(combined,msg,len + 1);
            memcpy(combined+len,data,*datalenp);
            len += (*datalenp);
            compressed = 0;//encode_json((char *)combined,len);
        } else compressed = 0;//encode_json(msg,len);
        if ( compressed != 0 )
        {
            *lenp = compressed->complen;
            data = (uint8_t *)compressed;
            free(combined);
        }
        else if ( combined != 0 )
        {
            data = combined;
            *lenp = len;
        }
        else
        {
            data = (uint8_t *)clonestr(msg);
            len++;
            *lenp = len;
        }
    }
    else *lenp = *datalenp;
    retdata = ciphers_codec(decryptflag,privkeys,cipherids,data,lenp);
    if ( decryptflag != 0 )
    {
        compressed = (struct compressed_json *)retdata;
        decompressed = 0;//decode_json(compressed,0);
        if ( decompressed != 0 )
        {
            free(retdata);
            retdata = (uint8_t *)decompressed;
            *lenp = compressed->origlen;
            *datalenp = (int32_t)(compressed->origlen - strlen(decompressed) - 1);
        }
    } else free(data);
    if ( privkeys != 0 )
    {
        for (i=0; privkeys[i]!=0; i++)
            free(privkeys[i]);
        free(privkeys);
        free(cipherids);
    }
    return(retdata);
}

char *telepathy_func(char *NXTaddr,char *NXTACCTSECRET,struct sockaddr *prevaddr,char *sender,int32_t valid,cJSON **objs,int32_t numobjs,char *origargstr)
{
    struct coin_info *cp = get_coin_info("BTCD");
    char passwordstr[1024],storedatastr[MAX_JSON_FIELD];
    char retbuf[1000],key[64],desthandle[MAX_JSON_FIELD],msg[MAX_JSON_FIELD],datastr[MAX_JSON_FIELD],*retstr = 0;
    //struct telepathy_args args;
    bits256 password;
    uint64_t location;
    int32_t len,datalen,decodedlen,encodedlen;
    uint8_t *encoded,*decoded,*dataptr,data[4096];
    struct contact_info *contact;
    if ( prevaddr != 0 || cp == 0 )
        return(0);
    copy_cJSON(desthandle,objs[0]);
    contact = find_contact(desthandle);
    copy_cJSON(msg,objs[1]);
    copy_cJSON(datastr,objs[2]);
    if ( contact != 0 && desthandle[0] != 0 && msg[0] != 0 && sender[0] != 0 && valid > 0 )
    {
        if ( (location= calc_privatetx(&password,contact,contact->numsent)) != 0 )
        {
            init_hexbytes(passwordstr,password.bytes,sizeof(password));
            dataptr = 0;
            datalen = 0;
            if ( datastr[0] != 0 && is_hexstr(datastr) )
            {
                datalen = (int32_t)strlen(datastr);
                if ( datalen > 1 && (datalen & 1) == 0 )
                {
                    datalen >>= 1;
                    dataptr = data;
                    decode_hex(data,datalen,datastr);
                } else datalen = 0;
            }
            encoded = AES_codec(&encodedlen,0,msg,dataptr,&datalen,contact->handle,passwordstr);
            if ( encoded != 0 )
            {
                len = encodedlen;
                decoded = AES_codec(&decodedlen,1,0,encoded,&len,contact->handle,passwordstr);
                if ( decoded != 0 )
                {
                    if ( strcmp(msg,(char *)decoded) == 0 )
                    {
                        printf("decrypted.(%s)\n",(char *)decoded);
                        if ( dataptr != 0 && memcmp(msg+strlen(msg)+1,dataptr,len - strlen(msg) - 1) != 0 )
                            printf("AES_codec error on datastr\n");
                    } else printf("AES_codec error on msg\n");
                    free(decoded);
                } else printf("AES_codec unexpected null decoded\n");
                init_hexbytes(storedatastr,encoded,encodedlen);
                free(encoded);
                sprintf(retbuf,"{\"result\":\"pending\",\"location\":\"%llu\",\"sequence\":\"%d\",\"len\":%d}",(long long)location,contact->numsent,encodedlen);
                expand_nxt64bits(key,location);
                if ( contact->numsent == 0 )
                    retstr = kademlia_storedata(0,GENESISACCT,GENESIS_SECRET,GENESISACCT,key,storedatastr);
                else retstr = kademlia_find("findnode",0,GENESISACCT,GENESIS_SECRET,GENESISACCT,key,storedatastr);
                contact->numsent++;
            } else printf("AES_codec unexpected null encoded\n");
            /*if ( retstr != 0 )
                free(retstr);
            memset(&args,0,sizeof(args));
            args.mytxid = myhash.txid;
            args.othertxid = otherhash.txid;
            args.refaddr = cp->privatebits;
            args.numrefs = scan_nodes(args.refaddrs,sizeof(args.refaddrs)/sizeof(*args.refaddrs),NXTACCTSECRET);
            start_task(Task_mindmeld,"telepathy",1000000,(void *)&args,sizeof(args));
            retstr = clonestr(retbuf);*/
        }
    }
    else retstr = clonestr("{\"error\":\"invalid telepathy_func arguments\"}");
    return(retstr);
}

#endif
