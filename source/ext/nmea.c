#ifndef NMEA_H
#define NMEA_H

//
// Repackaged parts of the original source of tweeta
// https://git.arrc.ou.edu/radar/tweeta
//
// Credits all go to the original author(s)
//
// Boonleng Cheong
// 6/10/2023
//

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#pragma pack(push, 1)

typedef struct nmea_data {
    double  utc_time;               // HHMMSS.SS
    double  latitude;               // DDMM.MMM - latitude in degrees, minutes, and decimal minutes
    double  longitude;              // DDMM.MMM - longitude in degrees, minutes, and decimal minutes
    double  heading;                // Y.Y - reference to true north
    double  course;                 // course over ground
    double  speed;                  // ground speed in kilometers per second
    double  bearing;                // direction of motion relative to true north
    bool    valid;                  // true if GPS reading is valid (pitch and roll are always valid)
} nmea_data_t;

#pragma pack(pop)

static int nmea_checksum(const char *sentence) {
    int i;
    if (sentence[0] != '$') {
        fprintf(stderr, "Expected a sentence that begins with '$'.\n");
        return -1;
    }
    char c = 0;
    for (i = 1; i < strlen(sentence) && sentence[i] != '*'; i++) {
        c ^= sentence[i];
    }
    char ic = strtol(sentence + i + 1, NULL, 16);
    return (int)(ic - c);
}

static void tokenize_nmea_gpgga_sentence(char *sentence, nmea_data_t *out) {

    // Start and end of the field
    char *s = (char *)sentence + 6;
    char *e = (char *)sentence + 6;

    char token[80];
    size_t len;
    int i = 0;

    int     b;
    float   c;
    double  utc_time, latitude, longitude;
    uint8_t quality;

    utc_time = latitude = longitude = 0.0;
    quality = 0;

    while (*e != '\0' && i < 16) {
        memset(token, 0, sizeof(token));

        do {
            e++;
        } while (*e != ',' && *e != '\0');
        // The field is contained between *s and *e
        len = (size_t)((long)e - (long)s) - 1;
        if (len) {
            strncpy(token, s + 1, len);
        }
        // convert the second token to UTC time
        if (i == 0) {
            utc_time = atof(token);
        } else if (i == 1){
            latitude = atof(token);
            b = latitude / 100;
            c = ((latitude / 100) - b) * 100;
            c /= 60;
            c += b;
            latitude = c;
        } else if (i == 2){
            if (*token == 'S'){
                latitude *= (-1);
            }
        } else if (i == 3){
            longitude = atof(token);
            b = longitude / 100;
            c = ((longitude / 100) - b) * 100;
            c /= 60;
            c += b;
            longitude = c;
        } else if (i == 4){
            if (*token == 'W'){
                longitude *= (-1);
            }
        } else if (i == 5) {
            quality = (uint8_t) atof(token);
        }
        // Add other fields of interest here.
        s = e;
        i++;
    }

    out->valid = (quality > 0) ? true : false;
    out->utc_time = utc_time;
    out->longitude = longitude;
    out->latitude = latitude;
}

static void tokenize_nmea_gphdt_sentence(char *sentence, nmea_data_t *out) {

    // Start and end of the field
    char *s = (char *)sentence + 6;
    char *e = (char *)sentence + 6;

    char token[80];
    size_t len;
    int i = 0;

    double heading = 0.0;

    while (*e != '\0' && i < 16) {
        memset(token, 0, sizeof(token));

        do {
            e++;
        } while (*e != ',' && *e != '\0');
        // The field is contained between *s and *e
        len = (size_t)((long)e - (long)s) - 1;
        if (len) {
            strncpy(token, s + 1, len);
        }
        // convert the second token to heading
        if (i == 0) {
            heading = atof(token);
        }
        s = e;
        i++;
    }

    out->heading = heading;
}

static void tokenize_nmea_gpvtg_sentence(char *sentence, nmea_data_t *out) {
    // Start and end of the field
    char *s = (char *)sentence + 6;
    char *e = (char *)sentence + 6;

    char token[80];
    size_t len;
    int i = 0;

    double true_bearing = 0.0, mag_bearing = 0.0, ground_speed_kph = 0.0;

    while (*e != '\0' && i < 8) {
        memset(token, 0, sizeof(token));

        do {
            e++;
        } while (*e != ',' && *e != '\0');
        // The field is contained between *s and *e
        len = (size_t)((long)e - (long)s) - 1;
        if (len) {
            strncpy(token, s + 1, len);
        }
        if (i == 0) {
            true_bearing = atof(token);
        } else if (i == 1) {
            if (*token != 'T') {
                true_bearing = NAN;
            }
        } else if (i == 2) {
            mag_bearing = atof(token);
        } else if (i == 3) {
            if (*token != 'M') {
                mag_bearing = NAN;
            }
        } else if (i == 6) {
            ground_speed_kph = atof(token);
        } else if (i == 7) {
            if (*token != 'K') {
                fprintf(stderr, "Inconsistency detected.\n");
            }
        }
        s = e;
        i++;
    }
    //printf("--> %.4f %.4f\n", mag_bearing, true_bearing);
    if (mag_bearing == 0.0 && true_bearing == 0.0) {
        out->speed = 0.0;
    } else {
        out->bearing = true_bearing;
        out->speed = ground_speed_kph;
    }
}

static void tokenize_nmea_gprmc_sentence(char *sentence, nmea_data_t *out) {

    // Start and end of the field
    char *s = (char *)sentence + 6;
    char *e = (char *)sentence + 6;

    char token[80];
    size_t len;
    int i = 0;

    int     b;
    float   c;
    double  utc_time, latitude, longitude, course;
    bool    valid;

    utc_time = latitude = longitude = course = 0.0;
    valid = false;

    while (*e != '\0' && i < 16) {
        memset(token, 0, sizeof(token));

        do {
            e++;
        } while (*e != ',' && *e != '\0');
        // The field is contained between *s and *e
        len = (size_t)((long)e - (long)s) - 1;
        if (len) {
            strncpy(token, s + 1, len);
        }
        // convert the second token to UTC time
        if (i == 0) {
            utc_time = atof(token);
        } else if (i == 1){
            if (*token == 'A'){
                valid = true;
            } else {
                valid = false;
            }
        } else if (i == 2){
            latitude = atof(token);
            b = latitude / 100;
            c = ((latitude / 100) - b) * 100;
            c /= 60;
            c += b;
            latitude = c;
        } else if (i == 3){
            if (*token == 'S'){
                latitude *= (-1);
            }
        } else if (i == 4){
            longitude = atof(token);
            b = longitude / 100;
            c = ((longitude / 100) - b) * 100;
            c /= 60;
            c += b;
            longitude = c;
        } else if (i == 5){
            if (*token == 'W'){
                longitude *= (-1);
            }
        } else if (i == 7) {
            course = atof(token);
        }
        s = e;
        i++;
    }

    //printf("valid = %d   %ld %ld  utc_time = %ld\n", valid, longitude, latitude, utc_time);
    out->valid = valid;
    out->utc_time = utc_time;
    out->longitude = longitude;
    out->latitude = latitude;
    out->course = course;
}

int nmea_parse_sentence(nmea_data_t *out, char *sentence) {
    int c = nmea_checksum(sentence);
    if (c) {
        out->valid = false;
        return c;
    }
    if (!strncmp(sentence, "$GPRMC", 6)) {
        tokenize_nmea_gprmc_sentence(sentence, out);
    } else if (!strncmp(sentence, "$GPGGA", 6)) {
        tokenize_nmea_gpgga_sentence(sentence, out);
    } else if (!strncmp(sentence, "$GPHDT", 6)) {
        tokenize_nmea_gphdt_sentence(sentence, out);
    } else if (!strncmp(sentence, "$GPVTG", 6)) {
        tokenize_nmea_gpvtg_sentence(sentence, out);
    } else {
        return 1;
    }
    return 0;
}

#endif
