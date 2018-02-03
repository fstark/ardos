#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

//  ----------------------------------------------------------------------------
//  Base defs
//  ----------------------------------------------------------------------------

typedef unsigned char u8;
typedef unsigned int u16;

//  ----------------------------------------------------------------------------
//  Utils
//  ----------------------------------------------------------------------------

inline unsigned int min( unsigned int a, unsigned int b ) { return a>b?b:a; }

int is_printable( unsigned char c ) { return c>=0x20 && c<=0x7e; }
unsigned char printable( unsigned char c ) { return is_printable(c)?c:'.'; }

u8 *memmove( u8 *dest, const u8 *src, u16 n )
{
    u8 *res = dest;
    if (dest<src)
        while (n--)
            *dest++ = *src++;
    else
    {
        dest += n;
        src += n;

        while (n--)
            *--dest = *--src;
    }
    return res;
}

//  ----------------------------------------------------------------------------
//  Serial communication
//  ----------------------------------------------------------------------------

#define BAUD 115200

#include <util/setbaud.h>

void uart_init(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
}

#define UART_UDR0 ((u8 *)0xc6)
#define UART_UCSR0A ((volatile u8 *)0xc0)
#define UART_UDRE0 5
#define UART_RXC0 7

void uart_put_char( u8 c )
{
    while (!((*UART_UCSR0A)&(1<<UART_UDRE0)))
        ;
    *UART_UDR0 = c;
}

u8 uart_get_char( void )
{
    while (!((*UART_UCSR0A)&(1<<UART_RXC0)))
        ;
    return *UART_UDR0;
}

//  ----------------------------------------------------------------------------
//  User I/O
//  ----------------------------------------------------------------------------

void (*put_char)( u8 c ) = uart_put_char;
u8 (*get_char)( void ) = uart_get_char;


#define LEFT    0x100
#define RIGHT   0x101
#define DEL     0x102
#define INS     0x103

u16 get_ext_char()
{
    while (1)
    {
        auto c = get_char();
        if (is_printable(c) || c==0x08 || c==0x0d || c==0x7f)
            return c;
        if (c!=0x1b)
            continue;
        c = get_char();
        if (c!=0x5b)
            continue;
        c = get_char();
        switch (c)
        {
            case 0x44:
                return LEFT;
            case 0x43:
                return RIGHT;
            case 0x32:
                c = get_char();
                if (c==0x7e)
                    return INS;
            case 0x33:
                c = get_char();
                if (c==0x7e)
                    return DEL;
        }
    }
}

void put_ln()
{
    put_char( '\r' );
    put_char( '\n' );
}

void put_left()
{
    put_char( 0x08 );
}

void put_right()
{
    put_char( 0x1b );
    put_char( 0x5b );
    put_char( 0x43 );
}

void put_string(const u8 *s)
{
    while (*s)
        put_char( *s++ );
}

void put_string_ln( const u8 *s )
{
    put_string( s );
    put_ln();
}

void put_char_n( u8 c, int n )
{
    while (n--) put_char( c );
}

void put_hex4( u16 i );
void put_hex2( u8 c );
void put_hex1( u8 c );

void put_hex4( u16 i )
{
    put_hex2( i>>8 );
    put_hex2( i&0xff );
}

void put_hex2( u8 c )
{
    put_hex1( c>>4 );
    put_hex1( c&0xf );
}

void put_hex1( u8 c )
{
    c += '0';
    if (c>'9')
        c += 'A'-'9'-1;
    put_char( c );
}

void get_line( char *p, u8 size )
{
    for (int i=0;i!=size;i++)
        p[i] = 0;
    auto max = size-2;
    u8 pos = 0;
    *p = 0;

    while (1)
    {
        auto c = get_ext_char();

        if (c==0x0d)
        {
            return;
        }
        if (c==0x7f || c==DEL)
        {
            if (pos!=0)
            {
                if (c==0x7f)
                {
                    pos--;
                    put_left();
                }
                memmove( p+pos, p+pos+1, size-pos-1 );
                p[size-1] = 0;
                for (int i=pos;i!=size;i++)
                    put_char( p[i]?p[i]:' ' );
                put_char_n( 0x08, size-pos );
            }
        }
        else if (c==LEFT)
        {
            if (pos>0)
            {
                pos--;
                put_left();
            }
        }
        else if (c==RIGHT)
        {
            if (!p[pos])
                p[pos] = ' ';
            if (pos<max)
            {
                pos++;
                put_right();
            }
        }
        else
        {
            p[pos] = c;
            put_char( c );
            if (pos<max)
            {
                pos++;
            }
            else
                put_left();
        }
    }
    //  NOT REACHED
}

//  ----------------------------------------------------------------------------
//  Dump utilities
//  ----------------------------------------------------------------------------

int is_writable( u8 *p )
{
    char c = *p;
    (*p)++;
    if (*p==c)
        return 0;
    *p = c;
    return 1;
}

void dump_line( const u8 *p, u16 l )
{
    const u8 *p0 = p;
    put_hex4( (u16)p );
    put_string( " :" );
    for (int i=0;i!=l;i++)
    {
        put_char( is_writable(p)?' ':'*' );
        put_hex2( *p++ );
        put_char( ' ' );
        if (i==3)
            put_char( ' ' );
    }
    put_char_n( ' ', (8-l)*3+(l>4) );

    for (int i=0;i!=l;i++)
    {
        put_char( printable(*p0++) );
        if (i==3)
            put_char( ' ' );
    }

    put_ln();
}

void dump_memory( const u8 *p, u16 l )
{
    while (l>0)
    {
        u16 c = min(8,l);
        dump_line( p, c );
        p += c;
        l -= c;
    }
}

int main()
{
    uart_init();

    put_string_ln( "ARDOS 0.1");

    while (1)
    {
        int v;

        u8 c = 0;

        do
        {
            c = get_char();
            put_hex2( c );
            put_char( ' ' );
        } while (c!='q');

        put_ln();
        char buffer[12];
        get_line( buffer, 12 );

        put_ln();
        put_char( '[' );
        put_string( buffer );
        put_char( ']' );

        put_string( "local = " );
        put_hex4( (u16)&v );
        put_ln();
        put_string( "UDR = " );
        put_hex4( RXC0 );
        put_ln();
        
        dump_memory( (void *)0, 128 );
    }
}
