#include "own_http.h"
#include <time.h>

volatile __uint8_t month;
volatile __uint8_t hour;
volatile __uint8_t min;

volatile __uint64_t nDays = 0;

// Calculate R0 by the month. Value in mm/dia
float __getR0(){
    switch (month) // Lat:38  value/month: 6.6 8.8 11.5 14.4 16.3 17.1 16.7 15.1 12.5 9.6 7.1 6.0
    {
    case 1:         // January
        return 6.6;
    case 2:
        return 8.8;
    case 3:
        return 11.5;
    case 4:
        return 14.4;
    case 5:
        return 16.3;
    case 6:
        return 17.1;
    case 7:
        return 16.7;
    case 8:
        return 15.1;
    case 9:
        return 12.5;
    case 10:
        return 9.6 ;
    case 11:
        return 7.1;
    case 12:        // December
        return 6.0;               
    
    default:
        printf("Error on getting the month. Recived by parameter: %i\n\n", month);
        error_flags = 1; // TODO do it properly
        return 17.1;
    }
}

// Calculate ET0 with temperature
void calcET0(int max, int min){
    float KT = 0.162;
    float R0 = __getR0(); // mm/día // TODO: Get a better way to get this number. More precise by the latitude 
    float t_med = (max + min)/2.0;
    float Rs = R0*KT*sqrt(max-min);

    ET0 = 0.0135*(t_med+17.78)*Rs;  // mm/dia
}

// Check if a digit is a number
int isDigit(char c){
	if( c >= '0' && c <= '9' )
		return 1;
	return 0;
}


// Get Status
// @param data: String to stract the data
// @param dest: String to put the data 
void getStatus(char* data, char* dest){
	int i = 0;
    strcpy(dest, "");

	while(data){					// Check all msg
		if(isDigit(data[i])){	// First number found
			strcat(dest, (char[4]) { data[i], data[i+1], data[i+2], '\0' }); // Three numbers at same time  
			break;
		}
		i++;		
	}
}

// Get Number of maximun temperature        
// data: String to stract the data
// dest: String to put the data 
void getMax(char* data, char* dest){
	int i = 0;

	char* subdata = strstr(data, "maxima");

	while(subdata){					// Check all msg
		if(isDigit(subdata[i])){	// First number found
			if(subdata[i-1] == '-') strcat(dest, (char[2]) { '-', '\0' } );	// Check for sign
			strcat(dest, (char[2]) { subdata[i], '\0' });					// First number
			if(isDigit(subdata[i+1])) strcat(dest, (char[2]) { subdata[i+1], '\0' });	// Second number
			break;
		}
		i++;		
	}
}

// Get number of minimum temperature
// data: String to stract the data
// dest: String to put the data 
void getMin(char* data, char* dest){
	int i = 0;

	char* subdata = strstr(data, "minima");

	while(subdata){					// Check all msg
		if(isDigit(subdata[i])){	// First number found
			if(subdata[i-1] == '-') strcat(dest, (char[2]) { '-', '\0' } );	// Check for sign
			strcat(dest, (char[2]) { subdata[i], '\0' });					// First number
			if(isDigit(subdata[i+1])) strcat(dest, (char[2]) { subdata[i+1], '\0' });	// Second number
			break;
		}
		i++;		
	}
}

// Get Values of temperature, max and min
void getTemp(char* data, char* max, char* min){
    strcpy(max, ""); strcpy(min, "");           // Clear Strings
    getMax(data, max);
    getMin(data, min);
}


// Get number of actual date
// Update the number of days irrigating 
// data: String to stract the data
// dest: String to put the data 
void getDate(char* extract){      // Data format: Fri, 10 Nov 2023 12:41:04 GMT
    struct tm parsed;

    static __uint8_t prevDay   = 40;
    static __uint8_t prevMonth = 40;

    // Parse the string into a struct tm.
    strptime(extract, "%a, %d %b %Y %T %Z", &parsed);

    //printf("Month: %d, DAY: %d HOUR: %d, MINUTE: %d\n", parsed.tm_mon, parsed.tm_mday, parsed.tm_hour, parsed.tm_min);

    // UPDATES DATE
    month = parsed.tm_mon++;  // Starts at 0, so increments 1
    hour = parsed.tm_hour++;  // GTM is not our Time Zone, so correct it
    min = parsed.tm_min;

    // Print it to check it parsed correctly.
    printf("parsed: %s\n", asctime(&parsed));

    if(prevDay != parsed.tm_mday){
        nDays++;
    }
    prevDay = parsed.tm_mday;
    prevMonth = month;
}

