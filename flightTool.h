#ifndef FLIGHTTOOL_H
#define FLIGHTTOOL_H

#include <libxml/tree.h>

struct FlightData {
    char id[20];
    char time[50];
    float weight;
    int points;
    char stat[50];
    char dest[100];
    char cabin[30];
    int seat;
    char version[30];
    char name[200];
};

void show_help(void);
void do_csv_to_binary(void);
void calculate_hex(char* text, int enc_type, char* output);
void do_binary_to_xml(void);
void do_validate(void);
void update_encoding_tree(xmlNodePtr n, int my_enc);
void do_encoding_change(void);

#endif /* FLIGHTTOOL_H */
