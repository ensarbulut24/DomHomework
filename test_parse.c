#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void parse(char *line) {
    char *tokens[10];
    for (int i=0; i<10; i++) tokens[i] = "";
    
    int n = 0;
    char *start = line;
    while (start && n < 10) {
        char *end = strchr(start, ',');
        if (end) *end = '\0';
        tokens[n++] = start;
        start = end ? end + 1 : NULL;
    }
    
    if (n == 9) {
        // Shift tokens from 3 to 8 -> 4 to 9
        for (int i=8; i>=3; i--) tokens[i+1] = tokens[i];
        tokens[3] = tokens[2]; // loyalty
        tokens[2] = ""; // weight missing
    }
    
    printf("ID: %s, Weight: %s, Loyalty: %s, Status: %s, Dest: %s, Class: %s, Seat: %s, App: %s, Name: %s\n",
        tokens[0], tokens[2], tokens[3], tokens[4], tokens[5], tokens[6], tokens[7], tokens[8], tokens[9]);
}

int main() {
    char l1[] = "THY1001,2025-03-01T08:10:00,15.4,,🟢,London Heathrow,ECONOMY,11,v1.0.0,Ahmet Yılmaz";
    char l2[] = "PEG1002,,850,⚠️,Berlin Brandenburg,BUSINESS,12,v1.0.1,Elif Şahin";
    parse(l1);
    parse(l2);
    return 0;
}
