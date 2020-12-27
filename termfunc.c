#include "system.h"

#define ST_READY           1
#define ST_BUSY            3
#define ST_TRANSMITTED     5
#define ST_RECEIVED        5

#define CMD_ACK            1
#define CMD_RESET          0
#define CMD_TRANSMIT       2
#define CMD_RECEIVE        2

#define CHAR_OFFSET        8
#define TERM_STATUS_MASK   0xFF

static termreg_t *term0_reg = (termreg_t *) DEV_REG_ADDR(IL_TERMINAL, 0);

static unsigned int tx_status(termreg_t *tp)
{
    return ((tp->transm_status) & TERM_STATUS_MASK);
}

static int term_putchar(char c)
{
    unsigned int stat;

    stat = tx_status(term0_reg);
    if (stat != ST_READY && stat != ST_TRANSMITTED)
        return -1;

    term0_reg->transm_command = ((c << CHAR_OFFSET) | CMD_TRANSMIT);

    while ((stat = tx_status(term0_reg)) == ST_BUSY)
        ;

    term0_reg->transm_command = CMD_ACK;

    if (stat != ST_TRANSMITTED)
        return -1;
    else
        return 0;
}


void term_puts(const char *str)
{
    while (*str)
        if (term_putchar(*str++))
            return;
}

//funzioni che gestiscono la ricezione dei caratteri su terminale

static unsigned int rx_status(termreg_t *tp)
{
    return ((tp->recv_status) & TERM_STATUS_MASK);
}

static int term_getchar()
{
    unsigned int stat;

    stat = rx_status(term0_reg);
    if (stat != ST_READY && stat != ST_RECEIVED)
        return -1;
     
    term0_reg->recv_command = CMD_RECEIVE;

    while ((stat = rx_status(term0_reg)) == ST_BUSY)
        ;
    int c =((term0_reg->recv_status) >> CHAR_OFFSET);
    term0_reg->recv_command = CMD_ACK;

    if (stat != ST_RECEIVED) {
        return -1;
        }
    else {
        return c;
        }
}

void term_gets(char *str, const int n)
{
   unsigned int i = 0;
   char c = term_getchar(); 
    while (i < n && c !='\n') {
         *str++ = c;
         c = term_getchar();
         i++;
     }
    *str++ = '\0';
}



