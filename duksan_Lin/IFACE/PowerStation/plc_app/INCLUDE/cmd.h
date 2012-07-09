#define MAX_BUFFER				1024
#define COMMAND_SERVER_PORT 	1004

#define MAX_RETRY_COUNT			10

#define CMD_PLC_MSG				0
#define CMD_DNP_MSG				1

int main(int argc, char *argv[]);
void Msg_handler(int nType, int nValue);
void DnpSet_handler(int nType, int nPno, int nValue);
void DnpInfo_handler(int nType, int nPno);
void DnpInfo_Parse(unsigned char *pData);
