#ifndef CONFIG_H
#define CONFIG_H

volatile unsigned int threshold = 50;

void toggleOption();
void selectOption();
void setThreshold();
void getThreshold(char *);

#endif
