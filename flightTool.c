#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>
#include "flightTool.h"

int flag_separator = 0;
int flag_opsys = 0;
int flag_encoding = 0;
char* file_in;
char* file_out;
int conv_type;

void show_help() {
    printf("Please use like this:\n");
    printf("./flightTool <input_file> <output_file> <conversion_type> -separator <1|2|3> -opsys <1|2|3> [-encoding <1|2|3>] [-h]\n");
}

void do_csv_to_binary() {
    FILE *fp1 = fopen(file_in, "r");
    if (fp1 == NULL) {
        printf("Error opening input file\n");
        return;
    }
    FILE *fp2 = fopen(file_out, "wb");
    if (fp2 == NULL) {
        printf("Error opening output file\n");
        fclose(fp1);
        return;
    }
    char line[2000];
    fgets(line, 2000, fp1);
    char my_sep[2];
    if (flag_separator == 1) {
        strcpy(my_sep, ",");
    } else if (flag_separator == 2) {
        strcpy(my_sep, "\t");
    } else if (flag_separator == 3) {
        strcpy(my_sep, ";");
    } else {
        strcpy(my_sep, ",");
    }
    while (fgets(line, 2000, fp1) != NULL) {
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
        if (line[strlen(line) - 1] == '\r') line[strlen(line) - 1] = '\0';
        if (strlen(line) == 0) continue;
        char *token;
        char *ptr = line;
        char tokens[10][200];
        int count = 0;
        while (count < 10) {
            token = strsep(&ptr, my_sep);
            if (token == NULL) break;
            strcpy(tokens[count], token);
            count++;
        }
        struct FlightData temprec;
        memset(&temprec, 0, sizeof(struct FlightData));
        strcpy(temprec.id, tokens[0]);
        strcpy(temprec.time, tokens[1]);
        if (strlen(tokens[2]) > 0) temprec.weight = atof(tokens[2]);
        if (strlen(tokens[3]) > 0) temprec.points = atoi(tokens[3]);
        strcpy(temprec.stat, tokens[4]);
        strcpy(temprec.dest, tokens[5]);
        strcpy(temprec.cabin, tokens[6]);
        if (strlen(tokens[7]) > 0) temprec.seat = atoi(tokens[7]);
        strcpy(temprec.version, tokens[8]);
        strcpy(temprec.name, tokens[9]);
        fwrite(&temprec, sizeof(struct FlightData), 1, fp2);
    }
    fclose(fp1);
    fclose(fp2);
}

void calculate_hex(char* text, int enc_type, char* output) {
    if (enc_type == 3 || enc_type == 0) {
        if (strlen(text) == 0) {
            strcpy(output, "");
            return;
        }
        unsigned char first_byte = text[0];
        int total_bytes = 1;
        if((first_byte & 0xE0) == 0xC0) total_bytes = 2;
        else if((first_byte & 0xF0) == 0xE0) total_bytes = 3;
        else if((first_byte & 0xF8) == 0xF0) total_bytes = 4;
        strcpy(output, "");
        for(int i = 0; i < total_bytes; i++) {
            char temp_hex[10];
            sprintf(temp_hex, "%02X", (unsigned char)text[i]);
            strcat(output, temp_hex);
        }
    } else {
        unsigned long unicode_val = 0;
        unsigned char c = text[0];
        if ((c & 0x80) == 0) {
            unicode_val = c;
        } else if ((c & 0xE0) == 0xC0) {
            unicode_val = ((c & 0x1F) << 6) | (text[1] & 0x3F);
        } else if ((c & 0xF0) == 0xE0) {
            unicode_val = ((c & 0x0F) << 12) | ((text[1] & 0x3F) << 6) | (text[2] & 0x3F);
        } else {
            unicode_val = ((c & 0x07) << 18) | ((text[1] & 0x3F) << 12) | ((text[2] & 0x3F) << 6) | (text[3] & 0x3F);
        }
        if (enc_type == 1) {
            int b1 = unicode_val & 0xFF;
            int b2 = (unicode_val >> 8) & 0xFF;
            sprintf(output, "%02X%02X", b1, b2);
        } else {
            int b1 = (unicode_val >> 8) & 0xFF;
            int b2 = unicode_val & 0xFF;
            sprintf(output, "%02X%02X", b1, b2);
        }
    }
}

void do_binary_to_xml() {
    FILE *bin_file = fopen(file_in, "rb");
    if (bin_file == NULL) {
        printf("Error: input dat file not found\n");
        return;
    }
    xmlDocPtr mydoc = xmlNewDoc(BAD_CAST "1.0");
    char root_tag[150];
    strcpy(root_tag, file_out);
    for(size_t i = 0; i < strlen(root_tag); i++) {
        if(root_tag[i] == '.') {
            root_tag[i] = '\0';
            break;
        }
    }
    char* final_slash = strrchr(root_tag, '/');
    char root_final[150];
    if (final_slash != NULL) {
        strcpy(root_final, final_slash + 1);
    } else {
        strcpy(root_final, root_tag);
    }
    xmlNodePtr root_node = xmlNewNode(NULL, BAD_CAST root_final);
    xmlDocSetRootElement(mydoc, root_node);
    struct FlightData record;
    int entry_id = 1;
    char buffer[200];
    while (fread(&record, sizeof(struct FlightData), 1, bin_file) > 0) {
        xmlNodePtr entry_node = xmlNewChild(root_node, NULL, BAD_CAST "entry", NULL);
        sprintf(buffer, "%d", entry_id);
        xmlNewProp(entry_node, BAD_CAST "id", BAD_CAST buffer);
        entry_id++;
        xmlNodePtr ticket_node = xmlNewChild(entry_node, NULL, BAD_CAST "ticket", NULL);
        xmlNewChild(ticket_node, NULL, BAD_CAST "ticket_id", BAD_CAST record.id);
        xmlNewChild(ticket_node, NULL, BAD_CAST "destination", BAD_CAST record.dest);
        xmlNewChild(ticket_node, NULL, BAD_CAST "app_ver", BAD_CAST record.version);
        xmlNodePtr met_node = xmlNewChild(entry_node, NULL, BAD_CAST "metrics", NULL);
        xmlNewProp(met_node, BAD_CAST "status", BAD_CAST record.stat);
        xmlNewProp(met_node, BAD_CAST "cabin_class", BAD_CAST record.cabin);
        sprintf(buffer, "%g", record.weight);
        xmlNewChild(met_node, NULL, BAD_CAST "baggage_weight", BAD_CAST buffer);
        sprintf(buffer, "%d", record.points);
        xmlNewChild(met_node, NULL, BAD_CAST "loyalty_points", BAD_CAST buffer);
        sprintf(buffer, "%d", record.seat);
        xmlNewChild(met_node, NULL, BAD_CAST "seat_num", BAD_CAST buffer);
        xmlNewChild(entry_node, NULL, BAD_CAST "timestamp", BAD_CAST record.time);
        xmlNodePtr pass_node = xmlNewChild(entry_node, NULL, BAD_CAST "passenger_name", BAD_CAST record.name);
        xmlNewProp(pass_node, BAD_CAST "current_encoding", BAD_CAST "UTF-8");
        char hex[50];
        calculate_hex(record.name, 3, hex);
        xmlNewProp(pass_node, BAD_CAST "first_char_hex", BAD_CAST hex);
    }
    fclose(bin_file);
    xmlSaveFormatFileEnc(file_out, mydoc, "UTF-8", 1);
    xmlFreeDoc(mydoc);
}

void do_validate() {
    xmlLineNumbersDefault(1);
    xmlSchemaParserCtxtPtr schema_ctxt = xmlSchemaNewParserCtxt(file_out);
    if (schema_ctxt == NULL) {
        printf("Schema context failed\n");
        return;
    }
    xmlSchemaPtr schema = xmlSchemaParse(schema_ctxt);
    xmlSchemaFreeParserCtxt(schema_ctxt);
    xmlDocPtr xml_file = xmlReadFile(file_in, NULL, 0);
    if (xml_file == NULL) {
        printf("Could not parse %s\n", file_in);
    } else {
        xmlSchemaValidCtxtPtr valid_ctxt = xmlSchemaNewValidCtxt(schema);
        int result = xmlSchemaValidateDoc(valid_ctxt, xml_file);
        if (result == 0) {
            printf("%s validates\n", file_in);
        } else if (result > 0) {
            printf("%s fails to validate\n", file_in);
        } else {
            printf("%s validation generated an internal error\n", file_in);
        }
        xmlSchemaFreeValidCtxt(valid_ctxt);
        xmlFreeDoc(xml_file);
    }
    if (schema != NULL) {
        xmlSchemaFree(schema);
    }
}

void update_encoding_tree(xmlNodePtr n, int my_enc) {
    xmlNodePtr current = NULL;
    for (current = n; current != NULL; current = current->next) {
        if (current->type == XML_ELEMENT_NODE && strcmp((char*)current->name, "passenger_name") == 0) {
            char *txt = (char *)xmlNodeGetContent(current);
            if (txt != NULL) {
                char new_hex[100];
                calculate_hex(txt, my_enc, new_hex);
                if (my_enc == 1) {
                    xmlSetProp(current, BAD_CAST "current_encoding", BAD_CAST "UTF-16LE");
                } else if (my_enc == 2) {
                    xmlSetProp(current, BAD_CAST "current_encoding", BAD_CAST "UTF-16BE");
                } else if (my_enc == 3) {
                    xmlSetProp(current, BAD_CAST "current_encoding", BAD_CAST "UTF-8");
                }
                xmlSetProp(current, BAD_CAST "first_char_hex", BAD_CAST new_hex);
                xmlFree(txt);
            }
        }
        update_encoding_tree(current->children, my_enc);
    }
}

void do_encoding_change() {
    xmlDocPtr my_doc = xmlReadFile(file_in, NULL, 0);
    if (my_doc == NULL) {
        printf("Cannot open XML for encoding\n");
        return;
    }
    update_encoding_tree(xmlDocGetRootElement(my_doc), flag_encoding);
    xmlChar *memory_buffer = NULL;
    int size = 0;
    if (flag_encoding == 1) {
        xmlDocDumpFormatMemoryEnc(my_doc, &memory_buffer, &size, "UTF-16LE", 1);
    } else if (flag_encoding == 2) {
        xmlDocDumpFormatMemoryEnc(my_doc, &memory_buffer, &size, "UTF-16BE", 1);
    } else {
        xmlDocDumpFormatMemoryEnc(my_doc, &memory_buffer, &size, "UTF-8", 1);
    }
    if (memory_buffer != NULL) {
        FILE *final_file = fopen(file_out, "wb");
        if (final_file != NULL) {
            if (flag_encoding == 1) {
                unsigned char bom1 = 0xFF;
                unsigned char bom2 = 0xFE;
                fwrite(&bom1, 1, 1, final_file);
                fwrite(&bom2, 1, 1, final_file);
            } else if (flag_encoding == 2) {
                unsigned char bom1 = 0xFE;
                unsigned char bom2 = 0xFF;
                fwrite(&bom1, 1, 1, final_file);
                fwrite(&bom2, 1, 1, final_file);
            }
            fwrite(memory_buffer, 1, size, final_file);
            fclose(final_file);
        }
        xmlFree(memory_buffer);
    }
    xmlFreeDoc(my_doc);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0) {
                show_help();
                return 0;
            }
        }
    }
    if (argc < 8) {
        printf("Missing arguments! Type -h for help.\n");
        return 1;
    }
    file_in = argv[1];
    file_out = argv[2];
    conv_type = atoi(argv[3]);
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-separator") == 0 && i + 1 < argc) {
            flag_separator = atoi(argv[i+1]);
        }
        if (strcmp(argv[i], "-opsys") == 0 && i + 1 < argc) {
            flag_opsys = atoi(argv[i+1]);
        }
        if (strcmp(argv[i], "-encoding") == 0 && i + 1 < argc) {
            flag_encoding = atoi(argv[i+1]);
        }
    }
    LIBXML_TEST_VERSION
    if (conv_type == 1) {
        do_csv_to_binary();
    } else if (conv_type == 2) {
        do_binary_to_xml();
    } else if (conv_type == 3) {
        do_validate();
    } else if (conv_type == 4) {
        do_encoding_change();
    } else {
        printf("Wrong conversion type.\n");
    }
    xmlCleanupParser();
    xmlMemoryDump();
    return 0;
}
