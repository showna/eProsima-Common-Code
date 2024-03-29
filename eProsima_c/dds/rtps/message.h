#ifndef _EPROSIMA_C_DDS_RTPS_MESSAGE_H_
#define _EPROSIMA_C_DDS_RTPS_MESSAGE_H_

#define RTPS_HEADER_PROTOCOL_SIZE 4
#define RTPS_HEADER_VERSION_SIZE 2
#define RTPS_HEADER_VENDORID_SIZE 2
#define RTPS_HEADER_GUIDPREFIX_SIZE 12

#define RTPS_SUBMESSAGE_HEADER_ID_SIZE 1
#define RTPS_SUBMESSAGE_HEADER_FLAGS_SIZE 1
#define RTPS_SUBMESSAGE_HEADER_OCTETSTONEXTHEADER_SIZE 2

// TODO Change BODY
#define RTPS_SUBMESSAGE_BODY_EXTRAFLAGS_SIZE 2
#define RTPS_SUBMESSAGE_BODY_OCTETSTOINLINEQOS_SIZE 2
#define RTPS_SUBMESSAGE_BODY_ENTITIESID_SIZE 8
#define RTPS_SUBMESSAGE_BODY_SEQUENCENUMBER_SIZE 8

#define RTPS_SUBMESSAGE_INFOTS_TIMESTAMP_SEC_SIZE 4
#define RTPS_SUBMESSAGE_INFOTS_TIMESTAMP_NANOSEC_SIZE 4
#define RTPS_SUBMESSAGE_INFOTS_TIMESTAMP_SIZE RTPS_SUBMESSAGE_INFOTS_TIMESTAMP_SEC_SIZE + RTPS_SUBMESSAGE_INFOTS_TIMESTAMP_NANOSEC_SIZE

enum RTPS_SubmessageKind
{
    RTPS_SUBMESSAGE_PAD = 0x01, /* Pad */
    RTPS_SUBMESSAGE_ACKNACK = 0x06, /* AckNack */
    RTPS_SUBMESSAGE_HEARTBEAT = 0x07, /* Heartbeat */
    RTPS_SUBMESSAGE_GAP = 0x08, /* Gap */
    RTPS_SUBMESSAGE_INFO_TS = 0x09, /* InfoTimestamp */
    RTPS_SUBMESSAGE_INFO_SRC = 0x0c, /* InfoSource */
    RTPS_SUBMESSAGE_INFO_REPLY_IP4 = 0x0d, /* InfoReplyIp4 */
    RTPS_SUBMESSAGE_INFO_DST = 0x0e, /* InfoDestination */
    RTPS_SUBMESSAGE_INFO_REPLY = 0x0f, /* InfoReply */
    RTPS_SUBMESSAGE_NACK_FRAG = 0x12, /* NackFrag */
    RTPS_SUBMESSAGE_HEARTBEAT_FRAG = 0x13, /* HeartbeatFrag */
    RTPS_SUBMESSAGE_DATA = 0x15, /* Data */
    RTPS_SUBMESSAGE_DATA_FRAG = 0x16, /* DataFrag */
    RTPS_SUBMESSAGE_NUMBER_OF_SUBMESSAGE_KINDS = 13
};

#endif // _EPROSIMA_C_DDS_RTPS_MESSAGE_H_
