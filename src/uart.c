
typedef struct uart {
        int txdata;
        int rxdata;
        int txctrl;
        int rxctrl;
        int ie;
        int ip;
        int div;
} UART;

extern volatile UART uart0;

void uart_init(void) {
        uart0.txctrl = 1;
}

void uart_put(char c) {
        while (uart0.txdata < 0);
        uart0.txdata = c;
}

void uart_puts(const char *c) {
        while (*c != '\0') {
                uart_put(*c);
                c++;
        }
}
