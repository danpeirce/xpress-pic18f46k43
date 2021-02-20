
void menu(void);
unsigned int readbatteryvoltage(void);
unsigned int* readsensors(void);
unsigned int readposition(void);
void sendbatteryvoltage(void);
void send_APSC1299(void);
void display_signature(void);
void LCD_print(char *str, char length);
void forward(unsigned char speed);
void backward(unsigned char speed);
void spinleft(unsigned char speed);
void spinright(unsigned char speed);
void LCD_line2(void);
void sendchar(char a_char);
void clearLCD(void);
void calibrate(void);
void go_pd(unsigned char speed);
void stop_pd(void);
void print_sensors(void);

