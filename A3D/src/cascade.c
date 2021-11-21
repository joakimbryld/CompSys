#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./cascade.h"
#include "./sha256.h"

char tracker_ip[IP_LEN];
char tracker_port[PORT_LEN];
char my_ip[IP_LEN];
char my_port[PORT_LEN];

struct csc_file *casc_file;
csc_block_t** queue;
csc_peer_t* peers;


typedef struct missing_block {
    csc_hashdata_t hash;               // Hash of this blocks bytes
} missing_block_t;


/*
 * Frees global resources that are malloc'ed during peer downloads. 
 */
void free_resources()
{
    free(queue);
    free(peers);
    csc_free_file(casc_file);
}

/*
 * Gets a sha256 hash of specified data, sourcedata. The hash itself is 
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable 
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_data_sha(const char* sourcedata, char* hash, uint32_t data_size, int hash_size)
{    
    SHA256_CTX shactx;
    unsigned char shabuffer[hash_size];
    sha256_init(&shactx);
    sha256_update(&shactx, sourcedata, data_size);
    sha256_final(&shactx, &shabuffer);
    
    for (int i=0; i<hash_size; i++)
    {
        hash[i] = shabuffer[i];
    }
}

/*
 * Gets a sha256 hash of a specified file, sourcefile. The hash itself is 
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable 
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_file_sha(const char* sourcefile, char* hash, int size)
{
    int casc_file_size;

    FILE* fp = fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return;
    }
    
    fseek(fp, 0L, SEEK_END);
    casc_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char buffer[casc_file_size];
    fread(buffer, casc_file_size, 1, fp); 
    fclose(fp);
    
    get_data_sha(buffer, hash, casc_file_size, size);
}



/*
 * Perform all client based interactions in the P2P network for a given cascade file.
 * E.g. parse a cascade file and get all the relevent data from somewhere else on the 
 * network.
 */
void download_only_peer(char *cascade_file)
{
    printf("Managing download only for: %s\n", cascade_file);    
    if (access(cascade_file, F_OK ) != 0 ) 
    {
        fprintf(stderr, ">> File %s does not exist\n", cascade_file);
        exit(EXIT_FAILURE);
    }

    char output_file[strlen(cascade_file)]; 
    memcpy(output_file, cascade_file, strlen(cascade_file));
    char* r = strstr(cascade_file, "cascade");
    int cutoff = r - cascade_file ;
    output_file[cutoff-1] = '\0';
    printf("Downloading to: %s\n", output_file);
       
    casc_file = csc_parse_file(cascade_file, output_file);
    
    int uncomp_count = 0; 
    queue = malloc(casc_file->blockcount * sizeof(csc_block_t*));

    *queue = casc_file -> blocks;

    for(int i=0;i<casc_file->blockcount;i++)
    {   
        if(casc_file->blocks[i].completed != 1)
        {
           queue[i] = &casc_file->blocks[i];
           uncomp_count++;
        }
    }

    /*
    TODO Create a list of missing blocks
    
    HINT: Use the already allocated 'csc_block_t** queue' for the list, and keep count of missing blocks in 'uncomp_cont'.
    */
    
    /*
    TODO Compute the hash of the cascade file

    HINT: Do not implement hashing from scratch. Use the provided 'get_file_sha' function
    */
    char *hash_buf;

    get_file_sha(cascade_file, hash_buf, SHA256_HASH_SIZE);

    int peercount = 0;
    while (peercount == 0)
    {
        peercount = get_peers_list(&peers, hash_buf);
        if (peercount == 0)
        {
            printf("No peers were found. Will try again in %d seconds\n", PEER_REQUEST_DELAY);
            fflush(stdout);
            sleep(PEER_REQUEST_DELAY);
        }
        else
        {
            printf("Found %d peer(s)\n", peercount);
        }
    }

    csc_peer_t peer = (peers[0]);
    // Get a good peer if one is available
    for (int i=0; i<peercount; i++)
    {
        if (peers[i].good)
        {
            peer = peers[i];
        }
    }

    for (int i=0; i<uncomp_count; i++)
    {
        get_block(queue[i], peer, hash_buf, output_file);
    }
    
    free_resources();
}

/*
 * Count how many times a character occurs in a string
 */
int count_occurences(char string[], char c)
{
    int i=0;
    int count=0;
    for(i=0; i<strlen(string); i++)  
    {
        if(string[i] == c)
        {
            count++;
        }
    }
    return count;
}

// Adapted from: https://stackoverflow.com/a/35452093/
/*
 *  Convert a string of hexidecimal into a string of bytes
 */
uint8_t* hex_to_bytes(const char* string) {

    if(string == NULL) 
       return NULL;

    size_t slength = strlen(string);
    if((slength % 2) != 0) // must be even
       return NULL;

    size_t dlength = slength / 2;

    uint8_t* data = malloc(dlength);
    memset(data, 0, dlength);

    size_t index = 0;
    while (index < slength) {
        char c = string[index];
        int value = 0;
        if(c >= '0' && c <= '9')
          value = (c - '0');
        else if (c >= 'A' && c <= 'F') 
          value = (10 + (c - 'A'));
        else if (c >= 'a' && c <= 'f')
          value = (10 + (c - 'a'));
        else {
          free(data);
          return NULL;
        }

        data[(index/2)] += value << (((index + 1) % 2) * 4);

        index++;
    }

    return data;
}

/*
 * Parses a cascade file, given the sourcepath input and destination, which may or may not exist.
 * Returns a pointer to a datastructure describing the file, or NULL if the file could not be parsed
 */
csc_file_t* csc_parse_file(const char* sourcefile, const char* destination)
{
    FILE* fp = fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return NULL;
    }

    const int FILE_HEADER_SIZE = 8+8+8+8+32;

    char header[FILE_HEADER_SIZE];
    if (fread(header, 1, FILE_HEADER_SIZE, fp) != FILE_HEADER_SIZE) 
    {
        printf("Failed to read magic 8 bytes header from file\n");
        fclose(fp);
        return NULL;
    }

    if (memcmp(header, "CASCADE1", 8) != 0)
    {
        printf("File does not contain magic 8 bytes in header\n");
        fclose(fp);
        return NULL;
    }

    /* initialising data struct*/

    csc_file_t* casc_file_data = (csc_file_t*)malloc(sizeof(csc_file_t));
  
    casc_file_data->targetsize = be64toh(*((unsigned long long*)&header[16]));
    casc_file_data->blocksize = be64toh(*((unsigned long long*)&header[24]));

    memcpy(casc_file_data->targethash.x, header+32, 32); // skal måske være strncpy


    /* counting blocks*/
    casc_file_data->blockcount=floor((casc_file_data->blocksize+casc_file_data->targetsize-1)/casc_file_data->blocksize);
    
    /* creating array of block structs*/
    // csc_block_t* casc_file_block [casc_file_data->blockcount]; 

    /* trail block size*/
    casc_file_data->trailblocksize = casc_file_data->targetsize % casc_file_data->blocksize;

    /* allocating memory for blocks*/
    casc_file_data->blocks = malloc(casc_file_data->blockcount*sizeof(csc_block_t));

    /* getting the blocks */
    for(int i = 0; i < (casc_file_data->blockcount-1); i++){
        char block_buffer[casc_file_data->blocksize];
        char shabuffer[SHA256_HASH_SIZE];

        fread(block_buffer, casc_file_data->blocksize, 1, fp);

        get_data_sha(block_buffer, shabuffer, casc_file_data->blocksize, SHA256_HASH_SIZE);
        casc_file_data->blocks[i].index = (uint64_t) i;
        casc_file_data->blocks[i].offset = i*casc_file_data->blocksize;
        casc_file_data->blocks[i].length = casc_file_data->blocksize; 
        casc_file_data->blocks[i].completed = 0;

        for(unsigned long long j = 0; j < 32; j++){
            casc_file_data->blocks[i].hash.x[j] = shabuffer[j];
        }        
    }

    char block_buffer[casc_file_data->trailblocksize];
    char shabuffer[SHA256_HASH_SIZE];

    fread(block_buffer, casc_file_data->trailblocksize, 1, fp);

    casc_file_data->blocks[casc_file_data->blocksize-1].index = casc_file_data->blockcount-1;
    casc_file_data->blocks[casc_file_data->blocksize-1].offset = (casc_file_data->blockcount-1)*casc_file_data->blocksize;
    casc_file_data->blocks[casc_file_data->blocksize-1].length = casc_file_data->trailblocksize;
    casc_file_data->blocks[casc_file_data->blocksize-1].completed = 0;

    for(unsigned long long i = 0; i < 32; i++){
  
        casc_file_data->blocks[casc_file_data->blocksize-1].hash.x[i] = shabuffer[i];
    }




    /* check om antal hashes passer med blocks?/*
    /* point to first block in array, korrekt?*/
    // casc_file_data->blocks=casc_file_block;

    /*
    To do Parse the cascade file and store the data in an appropriate data structure    
    
    HINT Use the definition of the 'csc_file' struct in cascade.h, as well as the 
    assignment handout for guidance on what each attribute is and where it is stored 
    in the files header/body.
    
    */

    
    fclose(fp);

    fp = fopen(destination, "a+w");
    if (fp == NULL)
    {
        printf("Failed to open destination file %s\n", destination);
        csc_free_file(casc_file_data);
        return NULL;
    }
    
    void* buffer = malloc(casc_file_data->blocksize);
    if (buffer == NULL)
    {
        printf("No block buffer asigned: %d\n", casc_file_data->blocksize);
        csc_free_file(casc_file_data);
        fclose(fp);
        return NULL;
    }


    SHA256_CTX shactx;
    for(unsigned long long i = 0; i < casc_file_data->blockcount; i++)
    {
        char shabuffer[SHA256_HASH_SIZE];
        unsigned long long size = casc_file_data->blocks[i].length;        
        if (fread(buffer, size, 1, fp) != 1)
        {
            break;
        }
        sha256_init(&shactx);
        sha256_update(&shactx, buffer, size);
        sha256_final(&shactx, &shabuffer);

        /*void *memcpy(void *dest, const void * src, size_t n)??*/
        if (casc_file_data->blocks[i].hash.x == shabuffer) {
            casc_file_data->blocks[i].completed <- 1;
        }
        
        /*
        TODO Compare the hashes taken from the Cascade file with those of the local data 
        file and keep a record of any missing blocks
        
        HINT The code above takes a hash of each block of data in the local file in turn 
        and stores it in the 'shabuffer' variable. You can compare then compare 'shabuffer'
        directly to the hashes of each block you have hopefully already assigned as part 
        of the 'casc_file_data' struct
        */
    }
    fclose(fp);
    return casc_file_data;
    
}

/*
 * Releases the memory allocated by a file datastructure
 */
void csc_free_file(csc_file_t* file)
{
    free(file->blocks);
    file->blocks = NULL;
    free(file);
}

/*
 * Get a specified block from a peer on the network. The block is retrieved and then inserted directly into
 * the appropriate data file at the appropriate location.
 */
void get_block(csc_block_t* block, csc_peer_t peer, unsigned char* hash, char* output_file)
{
    printf("Attempting to get block %d from %d:%s for %s\n", block->index, peer.ip, peer.port, output_file);
    
    rio_t rio;    
    char rio_buf[PEER_REQUEST_HEADER_SIZE]; /* read header size instead of maxline*/
    int peer_socket;

    struct ClientRequest client_request;
    peer_socket = open_clientfd(peer.ip,peer.port);

    rio_readinitb(&rio, peer_socket);

    memcpy(client_request.protocol, "CASCADE1", 8);
    uint64_t block_no = block->index;
    client_request.block_num = htobe64(block_no);

    memcpy(client_request.hash, hash, SHA256_HASH_SIZE);
    uint64_t reserved[2] = {0, 0};

    memcpy(client_request.reserved, reserved, 16);
    memcpy(rio_buf, &client_request, 64);
    rio_writen(peer_socket, rio_buf, PEER_REQUEST_HEADER_SIZE);
    rio_readnb(&rio, rio_buf, PEER_REQUEST_HEADER_SIZE); /*ændret fra MAXLINE*/

    /* headeren*/
    struct ClientResponseHeader response_header;
    response_header.error[0] = rio_buf[0];
    response_header.length = be64toh(*(uint64_t*)&rio_buf[1]);
    printf("Error: %d", response_header.error[0]);

    /* body*/
    char msg_buf[response_header.length]; /*længde af body */
    rio_readnb(&rio, msg_buf, response_header.length); /*block data ligger i msg_buf*/
    
    /*
    TODO Request a block from a peer DONE
    */

    /* checker her om vi har fået fat i den rigtige block*/
    char sha_buf[SHA256_HASH_SIZE];
    get_data_sha(msg_buf,sha_buf, block->length,SHA256_HASH_SIZE);

    if (sha_buf != block->hash.x){
         printf("Hash %s doesn't match hash %s", sha_buf, block->hash.x);
    }

    FILE* fp = fopen(output_file, "rb+");
    if (fp == 0)
    {
        printf("Failed to open destination: %s\n", output_file);
        Close(peer_socket);
        return;
    }

    /* skriv msg_buf (block_data) til output_file*/
    fseek(fp, block->index, SEEK_SET); /*fp peger et sted hen i output file, vi ændrer den pointer til block index*/
    fwrite(msg_buf, block->length, 1, fp);

    /*
    TODO Write the block into the data file DONE
    */
    
    printf("Got block %d. Wrote from %d to %d\n", block->index, block->offset); /*, block->offset+write_count-1);*/
    Close(peer_socket);
    fclose(fp);
}

/*
 * Get a list of peers on the network from a tracker. Note that this query is doing double duty according to
 * the protocol, and by asking for a list of peers we are also enrolling on the network ourselves.
 */
int get_peers_list(csc_peer_t** peers, unsigned char* hash)
{
    rio_t rio;    
    char rio_buf[MAX_LINE]; /*MESSAGE_SIZE i stedet for maxline*/
    char body_reply[BODY_SIZE];

    int tracker_socket;
    
    tracker_socket = open_clientfd("127.0.0.1","8888");
    rio_readinitb(&rio, tracker_socket);

    struct RequestHeader request_header;
    strncpy(request_header.protocol, "CASC", 4);
    request_header.version = htonl(1);
    request_header.command = htonl(1);
    request_header.length = htonl(BODY_SIZE);
    memcpy(rio_buf, &request_header, HEADER_SIZE);

    struct sockaddr_in sa;
    inet_pton(AF_INET, "127.0.1.1", &(sa.sin_addr));
    
    struct RequestBody request_body;
    request_body.ip = sa.sin_addr;
    request_body.port = 8888;
    // for (int i = 0; i<SHA256_HASH_SIZE; i++) {
    //     request_body.hash[i] = hash[i];
    //     // printf("%s \n", hash[i]);

    // }
    // printf("%s \n",request_body.hash);
    strncpy(request_body.hash,"713a203b7e89753a8893f7d452e8bcc19de65024595f50ddf7899f1da9d95fff", SHA256_HASH_SIZE);


    struct FullRequest { 
        struct RequestHeader request_header;
        struct RequestBody request_body;
    };
    struct FullRequest full_request;
    full_request.request_header = request_header;
    full_request.request_body = request_body;

    /*
    TODO Complete the peer list request MANGLER HASH
    */

    memcpy(rio_buf, &full_request, MESSAGE_SIZE);
    rio_writen(tracker_socket, rio_buf, MESSAGE_SIZE);
    rio_readnb(&rio, rio_buf, MAX_LINE); /*læser header reply ind i rio_buf*/
    
    /* læser reply header ind i reply_header*/
    char reply_header[MAX_LINE];
    memcpy(reply_header, rio_buf, MAX_LINE);




    uint32_t msglen = ntohl(*(uint32_t*)&reply_header[1]);
    if (msglen == 0)
    {
        Close(tracker_socket);
        return 0;        
    }
    
    if (reply_header[0] != 0) 
    {
        char* error_buf = malloc(msglen + 1);
        if (error_buf == NULL)
        {
            printf("Tracker error %d and out-of-memory reading error\n", reply_header[0]);
            Close(tracker_socket);
            return NULL;
        }
        memset(error_buf, 0, msglen + 1);
        memcpy(error_buf, &rio_buf[REPLY_HEADER_SIZE], msglen); // Fixed by Rune
        printf("Tracker gave error: %d - %s\n", reply_header[0], error_buf);
        free(error_buf);
        Close(tracker_socket);
        return NULL;
    }

    if (msglen % 12 != 0)
    {
        printf("LIST response from tracker was length %llu but should be evenly divisible by 12\n", msglen);
        Close(tracker_socket);
        return NULL;
    }

    int peercount = msglen/12; /*da trackeren returnerer en liste med peers må peercount være msglen/12 12 byte per peer*/

    for(int i = 0; i < peercount; i++)
    {

        /* læser reply body ind i body_reply*/
        rio_readnb(&rio, body_reply, 4); /*peer ip*/
        memcpy(body_reply, &peers[i]->ip, 4); 

        rio_readnb(&rio, body_reply, 2); /*peer port*/
        memcpy(body_reply,  &peers[i]->port, 2); 

        rio_readnb(&rio, body_reply, 4); /*last seen timestamp*/
        peers[i]->lastseen = (uint32_t) body_reply;
        
        rio_readnb(&rio, body_reply, 1); /*goodpeer*/
        peers[i]->good = (uint8_t) body_reply;
        
    }

    /*
    TODO Parse the body of the response to get a list of peers
    
    HINT Some of the later provided code expects the peers to be stored in the ''peers' variable, which 
    is an array of 'csc_peer's, as defined in cascade.h
    */

    Close(tracker_socket);
    return peercount;
}

/*
 * The entry point for the code. Parses command line arguments and starts up the appropriate peer code.
 */
int main(int argc, char **argv) 
{
    if (argc != MAIN_ARGNUM + 1) 
    {
        fprintf(stderr, "Usage: %s <cascade file(s)> <tracker server ip> <tracker server port> <peer ip> <peer port>.\n", argv[0]);
        exit(EXIT_FAILURE);
    } 
    else if (!is_valid_ip(argv[2])) 
    {
        fprintf(stderr, ">> Invalid tracker IP: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    } 
    else if (!is_valid_port(argv[3])) 
    {
        fprintf(stderr, ">> Invalid tracker port: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    } 
    else if (!is_valid_ip(argv[4])) 
    {
        fprintf(stderr, ">> Invalid peer IP: %s\n", argv[4]);
        exit(EXIT_FAILURE);
    } 
    else if (!is_valid_port(argv[5])) 
    {
        fprintf(stderr, ">> Invalid peer port: %s\n", argv[5]);
        exit(EXIT_FAILURE);
    }

    snprintf(tracker_ip,   IP_LEN,   argv[2]);
    snprintf(tracker_port, PORT_LEN, argv[3]);
    snprintf(my_ip,   IP_LEN,   argv[4]);
    snprintf(my_port, PORT_LEN, argv[5]);
    
    char cas_str[strlen(argv[1])];
    snprintf(cas_str, strlen(argv[1])+1, argv[1]);
    char delim[] = ":";

    int casc_count = count_occurences(argv[1], ':') + 1;
    char *cascade_files[casc_count];

    char *ptr = strtok(cas_str, delim);
    int i = 0;

    while (ptr != NULL)
    {
        if (strstr(ptr, ".cascade") != NULL)
        {
            cascade_files[i++] = ptr;
            ptr = strtok(NULL, delim);
        }
        else
        {
            printf("Abort on %s\n", ptr);   
            fprintf(stderr, ">> Invalid cascade file: %s\n", ptr);
            exit(EXIT_FAILURE);
        }
    }

    for (int j=0; j<casc_count; j++)
    {
        download_only_peer(cascade_files[j]);
    }

    exit(EXIT_SUCCESS);

}

