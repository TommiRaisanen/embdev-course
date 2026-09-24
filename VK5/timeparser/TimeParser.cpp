#include <stdlib.h>
#include <string.h>
#include "TimeParser.h"

// time format: HHMMSS (6 characters)
int time_parse(char *time) {

	// how many seconds, default returns error
	int seconds = TIME_LEN_ERROR;
	int hours = TIME_LEN_ERROR;
	int minutes = TIME_LEN_ERROR;
	// TODO: Check that string is not null
	if(time == NULL) {
		return TIME_NULL_ERROR;
	}

	if (strlen(time) != 6) {
    	return TIME_LEN_ERROR;
	}
	for (int i = 0; i < 6; i++) {
    	if (time[i] < '0' || time[i] > '9') {
        	return TIME_VALUE_ERROR;
    	}
	}
	// Parse values from time string
	// For example: 124033 -> 12hour 40min 33sec
    int values[3];
	values[2] = atoi(time+4); // seconds
	time[4] = 0;
	values[1] = atoi(time+2); // minutes
	time[2] = 0;
	values[0] = atoi(time); // hours
	// Now you have:
	// values[0] hour
	// values[1] minute
	// values[2] second
	hours = values[0];
	minutes = values[1];
	seconds = values[2];
	// TODO: Add boundary check time values: below zero or above limit not allowed
	// limits are 59 for minutes, 23 for hours, etc
	if(hours < 0 || hours > 23) return TIME_VALUE_ERROR;
	if(minutes < 0 || minutes > 59) return TIME_VALUE_ERROR;
	if(seconds < 0 || seconds > 59) return TIME_VALUE_ERROR;
	// TODO: Calculate return value from the parsed minutes and seconds
	// Otherwise error will be returned!
	// seconds = ...
	seconds = seconds + (minutes * 60);

	return seconds;
}
