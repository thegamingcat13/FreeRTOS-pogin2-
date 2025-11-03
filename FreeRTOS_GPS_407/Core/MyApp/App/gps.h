/**
* @file my_gps.h
* @brief Bevat basic (provisorische) defines & externals voor de gps routines
* @attention
* <h3>&copy; Copyright (c) 2023 (HU) Michiel Scager.</h3>
* @author MSC
*
* @date 5/9/2023
*/
int hex2int(char *c);
int hexchar2int(char c);
int checksum_valid(char *string);


// GNRMC struct: all with char-members - should/could be improved with proper data-elements
typedef struct _GNRMC
{
	char    head[7];       // 1. header
	char    time[10];      // 2. hhmmss.sss
	char    status;        // 3. A=valid, V=not valid
	char    latitude[10];  // 4. ddmm.mmmm (double)
	char    NS_ind;        // 5. N,S
	char    longitude[10+2]; // 6. ddmm.mmmm (double)
	char    EW_ind;        // 7. E,W
	char    speed[6];      // 8. 0.13 knots (double)
	char    course[6];     // 9. 309.62 degrees (double)
	char    date[7];       // 10. ddmmyy
	char    mag_var[6];    // 11. E,W degrees (double)
	char    mag_var_pos;   // 12.
	char    mode;          // 13. A=autonomous, D,E
	char    cs[4];         // 14. checkum *34
} GNRMC;


typedef struct sParsed_GNRMC
{
	char    head[7];       // 1. header
	int     time;          // 2. hhmmss.sss
	char    status;        // 3. A=valid, V=not valid
	double  latitude;      // 4. ddmm.mmmm (double)
	char    NS_ind;        // 5. N,S
	double  longitude;     // 6. ddmm.mmmm (double)
	char    EW_ind;        // 7. E,W
	double  speed;         // 8. 0.13 knots (double)
	double  course;        // 9. 309.62 degrees (double)
	char    date[7];       // 10. ddmmyy
	double  mag_var;       // 11. E,W degrees (double)
	char    mag_var_pos;   // 12.
	char    mode;          // 13. A=autonomous, D,E
	char    cs[4];         // 14. checkum *34
} Parsed_GNRMC;

extern Parsed_GNRMC parsed_gnrmc;

// enum voor NMEA protocolstrings (starting 'e' for enum)
// gebruikt het type van de nmea-msg in de queue vast te leggen
enum NMEA
{
	eGNRMC = 1,  // 1
	eGPGSA,      // 2
	eGNGGA       // 3 - kun je verder uitbreiden natuurlijk
};

