
#define MAX_MSG_SIZE 1024

typedef struct Msg_queue_node {
    char message[MAX_MSG_SIZE];
    size_t length;
    int sender_id;
    STAILQ_ENTRY(Msg_queue_node_t) messages;
} Msg_queue_node_t;

STAILQ_HEAD(Msg_queue_head, Msg_queue_node);

typedef struct Msg_queue_head Msg_queue_head_t;


