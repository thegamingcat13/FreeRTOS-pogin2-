#include <admin.h>
#include <stdbool.h>
#include "main.h"
#include "gps.h"
#include "cmsis_os.h"

FILE* pdata;

void txtOpen()
{
// Open het bestand in append-modus
	pdata = fopen(":", "a");
    if (pdata == NULL)
    {
        UART_puts("error with opening file\n"); // Geef een foutmelding als het bestand niet geopend kan worden
    }

}

void txtWriteChar (char *Name, char *DataInputChar)
{
	fprintf(pdata, "%s :", Name);
	fprintf(pdata, "%s", DataInputChar);
	fprintf(pdata, "\n");
}

void txtWriteInt (char *Name, int DataInputInt)
{
	fprintf(pdata, "%s :", Name);
	fprintf(pdata, "%d", DataInputInt);
	fprintf(pdata, "\n");
}

void txtWriteFloat (char *Name, float DataInputFloat)
{
	fprintf(pdata, "%s :", Name);
	fprintf(pdata, "%f", DataInputFloat);
	fprintf(pdata, "\n");
}


void txtClose()
{
	fclose(pdata);
}
