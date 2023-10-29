/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_OUR_DRIVER_H__
#define __UART_OUR_DRIVER_H__

void set_default_value_for_uart();

void send_message_by_uart_if_available();

void append_to_sending_buffer_by_uart(char *);

char *read_char_by_uart();

#endif // __UART_OUR_DRIVER_H__
