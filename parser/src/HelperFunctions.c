/**
 * @file HelperFunctions.c
 * @brief This file contains the VCard file's helper functions to assist with parsing and structure handling.
 * @author ADD LATER
 */

//NOTE: REMEMBER TO CHANGE HARDCODED FILE LOCATIONS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "LinkedListAPI.h"
#include "HelperFunctions.h"
#include "VCardParser.h"
#define DEBUG false

char* readFileToString(char* fileName) {
    //Find and open the file in read mode
    FILE* file;
    file = fopen(fileName, "r");

    //Check if the file could open
    if (file == NULL) {
        //Error: File could not be opened
        return NULL;
    }

    //Find size of file
    //Seek to end of file
    fseek(file, 0, SEEK_END);
    //Get current file pointer
    int fileSize = ftell(file);
    //Seek back to beginning of file
    fseek(file, 0, SEEK_SET);

    if (fileSize == 0) {
        fclose(file);
        return NULL;
    }

    //Calloc size of file for string
    char* toReturn = calloc(fileSize, sizeof(char));

    //Read file contents character by character into a string
    fread(toReturn, fileSize, 1, file);

    //Close file
    fclose(file);

    //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
    return toReturn;
}

char** split(char* stringToSplit, int* length, char* delimiters, bool whiteSpaceCheck) {
    //Declare variables
    char** toReturn = 0;
    char* currentString;
    int i = 0;

    //Perform the first string tokenize
    currentString = strtok(stringToSplit, delimiters);

    //Go through all strings that end in \r\n
    while (currentString != 0) {
        //Check if the first character of the string is \t or ' ' for concatenation
        if (whiteSpaceCheck) {
            if (currentString[0] == '\t' || currentString[0] == ' ') {
                //Remove single whitespace '\t' || ' ' from string before concatenation
                int length = strlen(currentString);
                //Go through entire string to replace the first character (\t || ' ')
                for (int j = 0; j < length; j++) {
                    currentString[j] = currentString[j + 1];
                }

                //Reallocate space for string + concatenation
                toReturn[i - 1] = realloc(toReturn[i - 1], strlen(toReturn[i - 1]) + strlen(currentString) + 1);

                //Concatenate string to previous string
                strcat(toReturn[i - 1], currentString);

                //Continue to next string tokenize
                currentString = strtok(NULL, delimiters);
                continue;
            }
        }

        //Allocate space for the string
        toReturn = realloc(toReturn, (i + 1) * sizeof(char*));
        toReturn[i] = calloc(strlen(currentString) + 1, sizeof(char));

        //Copy the string into the 2d array location
        strcpy(toReturn[i], currentString);
        //Increment i
        i++;
        //Tokenize again
        currentString = strtok(NULL, delimiters);
    }
    //Set the length of the 2d array to i
    *length = i;

    //Return the 2d array of strings
    return toReturn;
}

bool verifyFileName(char* fileName) {
    //Check if passed in file name is NULL, or the string length is <= 0
    if (fileName == NULL || strlen(fileName) <= 0) {
        return false;
    }

    //Grab the extension of the file (Everything after the last dot)
    char* extension = strchr(fileName, '.');
    if (!extension) {
        return false;
    }

    //Check if the extension is correct "vcf"
    if (strcmp(extension, ".vcf") == 0 || strcmp(extension, ".vcard") == 0) {
        return true;
    }
    else {
        return false;
    }
}

Card* initializeCard() {
    Card* newCard = malloc(sizeof(Card));
    newCard->fn = NULL;
    newCard->birthday = NULL;
    newCard->anniversary = NULL;
    newCard->optionalProperties = initializeList(printProperty, deleteProperty, compareProperties);
    return newCard;
}

DateTime* createDateTime(int flexSize) {
    DateTime* newDateTime = malloc(sizeof(DateTime) + flexSize * sizeof(char));
    newDateTime->UTC = false;
    newDateTime->isText = false;
    memset(newDateTime->date, '\0', 9);
    memset(newDateTime->time, '\0', 7);
    newDateTime->text[0] = '\0';
    return newDateTime;
}

Parameter* createParameter(int flexSize) {
    Parameter* newParameter = malloc(sizeof(Parameter) + flexSize * sizeof(char));
    newParameter->name[0] = '\0';
    newParameter->value[0] = '\0';
    return newParameter;
}

Property* createProperty() {
    Property* newProperty = malloc(sizeof(Property));
    newProperty->name = NULL;
    newProperty->group = NULL;
    newProperty->parameters = initializeList(printParameter, deleteParameter, compareParameters);
    newProperty->values = initializeList(printValue, deleteValue, compareValues);
    return newProperty;
}

void stringUpper(char* string) {
    int i = 0;
    while (string[i] != '\0') {
        if (string[i] >= 'a' && string[i] <= 'z') {
            string[i] = string[i] - 32;
        }
        i++;
    }
}

int numberOfCharacters(char* string, char toSearch) {
    int countToReturn = 0;

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == toSearch) {
            countToReturn++;
        }
    }

    return countToReturn;
}

bool verifyStringEndOfLines(char* string) {
    if (string[0] == '\r' || string[0] == '\n' || string[0] == '\0') {
        return false;
    }
    int quotationCounter = 0;
    int stringLength = (int)(strlen(string));

    for (int i = 0; i < stringLength; i++) {
        if (string[i] == '"') {
            quotationCounter++;
            if (quotationCounter == 2) {
                quotationCounter = 0;
            }
        }

        if (quotationCounter == 0) {
            if (string[i] == '\n') {
                if (string[i - 1] != '\r') {
                    return false;
                }
            }

            if (string[i] == '\r') {
                if (string[i + 1] != '\n') {
                    return false;
                }
            }
        }
    }

    return true;
}

void removeBackslash(char* string) {
    int quotationCounter = 0;
    int stringLength = (int)(strlen(string));

    for (int i = 0; i < stringLength; i++) {
        if (string[i] == '"') {
            quotationCounter++;
            if (quotationCounter == 2) {
                quotationCounter = 0;
            }
        }

        if (quotationCounter == 1) {
            if (string[i] == ',' || string[i] == '\\' || string[i] == ';') {
                if (string[i] == ';') {
                    //Replace semi-colon with another character to reduce parsing issues later
                    //NOTE: Look for all '^' at the end and replace it back with a ';'
                    string[i] = '^';
                }

                if (string[i - 1] == '\\') {
                    //Remove backslash
                    int indexToDelete = i - 1;
                    memmove(&string[indexToDelete], &string[indexToDelete + 1], strlen(string) - indexToDelete);
                }
            }
        }
    }
}

int stringCaseCompare(const char* string1, const char* string2) {
    int d = 0;
    for ( ; ; ) {
        const int c1 = tolower(*string1++);
        const int c2 = tolower(*string2++);
        if (((d = c1 - c2) != 0) || (c2 == '\0')) {
            break;
        }
    }
    return d;
}

void noParamOneValue(Card* mainCard, Property* toStoreIn, char* sectionOfLine, char* values, bool groupSet) {
    if (groupSet == true) {
        //Skip first tokenize on "."
        sectionOfLine = strtok(sectionOfLine, ".");
        sectionOfLine = strtok(NULL, ":");
    }
    else {
        sectionOfLine = strtok(sectionOfLine, ":");
    }

//    sectionOfLine = strtok(sectionOfLine, ":");
    if(DEBUG)printf("Property: %s\n", sectionOfLine);
    //NOTE: REMEMBER TO FREE newProperty->name
    toStoreIn->name = calloc(strlen(sectionOfLine) + 1, sizeof(char));
    strcpy(toStoreIn->name, sectionOfLine);
    if(DEBUG)printf("Value(s): %s\n", values);
    //NOTE: REMEMBER TO FREE value
    char* propertyValue = calloc(strlen(values) + 1, sizeof(char));
    strcpy(propertyValue, values);
    insertBack(toStoreIn->values, propertyValue);
}

void noParamMultipleValues(Card* mainCard, Property* toStoreIn, char* sectionOfLine, char* values, bool groupSet) {
    //Should be a property followed by a list of values - delimit on ';' && ':'
    //Count how many semi colons there are - compare to the number of values created
    int valueCount = 0;
    int semiColonCount = numberOfCharacters(sectionOfLine, ';');
    char* section = NULL;

    //Create Property* structure based on currentLine
    if (groupSet == true) {
        //Skip first tokenize on "."
        section = strtok(sectionOfLine, ".");
        section = strtok(NULL, ":;");
    }
    else {
        section = strtok(sectionOfLine, ":;");
    }

//    char* section = strtok(sectionOfLine, ":;");
    if(DEBUG)printf("Property: %s\n", section);

    //NOTE: REMEMBER TO FREE newProperty->name
    toStoreIn->name = calloc(strlen(section) + 1, sizeof(char));
    strcpy(toStoreIn->name, section);

    //Begin grabbing all values and storing into Property* structure
    while (section) {
        section = strtok(NULL, ";");
        if (section != NULL) {
            valueCount++;
            if(DEBUG)printf("Value(s): %s\n", section);
            char* value = calloc(strlen(section) + 1, sizeof(char));
            strcpy(value, section);
            insertBack(toStoreIn->values, value);
        }
    }
    int toAdd = 0;
    //Handles empty string values that must be added
    if (valueCount <= semiColonCount) {
        toAdd = semiColonCount - valueCount;
        if (toAdd >= 0) {
            toAdd++;
        }
        for (int i = 0; i < toAdd; i++) {
            if(DEBUG)printf("Value(s): \n");
            //NOTE: REMEMBER TO FREE value
            char* value = calloc(1, sizeof(char));
            strcpy(value, "");
            insertBack(toStoreIn->values, value);
        }
    }
}

void params(Card* mainCard, Property* toStoreIn, List* parameters, char* sectionOfLine, char* values, bool groupSet) {
    //Now there are parameters
    if (groupSet == true) {
        sectionOfLine = strchr(sectionOfLine, '.');
        sectionOfLine++;
    }

    if(DEBUG)printf("Property: %s\n", sectionOfLine);
    //NOTE: REMEMBER TO FREE newProperty->name
    toStoreIn->name = calloc(strlen(sectionOfLine) + 1, sizeof(char));
    strcpy(toStoreIn->name, sectionOfLine);

    //Create a copy of values
    int valuesLength = strlen(values);
    char* valuesCopy = calloc(valuesLength + 1, sizeof(char));
    strcpy(valuesCopy, values);
    //NOTE: REMEMBER TO FREE valuesCopyPointer
    char* valuesCopyPointer = valuesCopy;

    int valueCount = 0;
    int semiColonCount = numberOfCharacters(valuesCopy, ';');

    //If validated, parse using strtok currentLine ";" based on the return integer value of # of semi-colons
    //Read through line until a colon is reached - once the colon is reached, all parameters have been received
    while (sectionOfLine) {
	sectionOfLine = strtok(NULL, ";");
        if (numberOfCharacters(sectionOfLine, '^') > 0) {
            //Go through line and replace all ^ with ;
            int length = strlen(sectionOfLine);
            for (int j = 0; j < length; j++) {
                if (sectionOfLine[j] == '^') {
                    sectionOfLine[j] = ';';
                }
            }
        }
        if (sectionOfLine) {
            //if currentLine contains ':', read until ':' - That's the last parameter
            //Break from loop after last parameter created
            //NOTE: Deal with last parameter and get rid of value
            if (numberOfCharacters(sectionOfLine, ':') > 0) {
                //Read until ':' - Last parameter
                char* lastParam = sectionOfLine;
                lastParam = strtok(lastParam, ":");
                //NOTE: REMEMBER TO FREE tempParameter (Gets freed in freeList)
                char* tempParameter = calloc(strlen(lastParam) + 1, sizeof(char));
                strcpy(tempParameter, lastParam);
                insertBack(parameters, tempParameter);
                break;
            }
            else {
                //NOTE: REMEMBER TO FREE tempParameter (Gets freed in freeList)
                char* tempParameter = calloc(strlen(sectionOfLine) + 1, sizeof(char));
                strcpy(tempParameter, sectionOfLine);
                insertBack(parameters, tempParameter);
            }
	}
    }

    valuesCopy = strtok(valuesCopy, ";");

    while (valuesCopy) {
        if (valuesCopy != NULL) {
            valueCount++;
            if(DEBUG)printf("Value(s): %s\n", valuesCopy);
            char* value = calloc(strlen(valuesCopy) + 1, sizeof(char));
            strcpy(value, valuesCopy);
            insertBack(toStoreIn->values, value);
        }
        valuesCopy = strtok(NULL, ";");
    }
    int toAdd = 0;
    //Handles empty string values that must be added
    if (valueCount <= semiColonCount) {
        toAdd = semiColonCount - valueCount;
        if (toAdd >= 0) {
            toAdd++;
        }
        for (int i = 0; i < toAdd; i++) {
            if(DEBUG)printf("Value(s): \n");
            //NOTE: REMEMBER TO FREE value
            char* value = calloc(1, sizeof(char));
            strcpy(value, "");
            insertBack(toStoreIn->values, value);
        }
    }

    free(valuesCopyPointer);
}

VCardErrorCode addParametersToProperty(Property* toStoreIn, List* parameters) {
    if (toStoreIn == NULL || parameters == NULL) {
        return INV_PROP;
    }

    if (parameters->length > 0) {
        Node* temp = parameters->head;
        while (temp) {
	    if (temp->data == NULL) {
                return INV_PROP;
	    }
            char* parameterToParse = (char*)(temp->data);
	    if (parameterToParse == NULL) {
                //Error
		return INV_PROP;
	    }
            char* parameterCopy = calloc(strlen(parameterToParse) + 1, sizeof(char));
            strcpy(parameterCopy, parameterToParse);

            //NOTE: REMEMBER TO FREE parameterCopyPointer
	    if (numberOfCharacters(parameterCopy, '=') == 0) {
                free(parameterCopy);
		return INV_PROP;
	    }
            char* parameterCopyPointer = parameterCopy;
            parameterCopy = strtok(parameterCopy, "=");

            //NOTE: REMEMBER TO FREE parameterName
	    if (parameterCopy == NULL) {
                //Error
		free(parameterCopyPointer);
		return INV_PROP;
	    }
            char* parameterName = calloc(strlen(parameterCopy) + 1, sizeof(char));
            strcpy(parameterName, parameterCopy);
            if(DEBUG)printf("Parameter Name: %s\n", parameterName);

            parameterCopy = strtok(NULL, "=");

	    if (parameterCopy == NULL) {
                //Error
		free(parameterCopyPointer);
		free(parameterName);
		return INV_PROP;
	    }

            //NOTE: REMEMBER TO FREE parameterValue
            char* parameterValue = calloc(strlen(parameterCopy) + 1, sizeof(char));
            strcpy(parameterValue, parameterCopy);
            if(DEBUG)printf("Parameter Value: %s\n", parameterValue);

            Parameter* newParameter = createParameter(strlen(parameterValue) + 1);
            strcpy(newParameter->name, parameterName);
            strcpy(newParameter->value, parameterValue);

            insertBack(toStoreIn->parameters, newParameter);

            free(parameterName);
            free(parameterValue);
            free(parameterCopyPointer);
            temp = temp->next;
        }
    }
    return OK;
}

DateTime* parseDate(Property* currentProperty) {
    bool isText = false;
    bool UTC = false;
    char* date = NULL;
    char* time = NULL;
    char* text = NULL;
    int flexSize = 0;

    if (currentProperty->values->length > 0) {
        //Grab values of the dateTime
        char* valuesString = toString(currentProperty->values);
        //NOTE: REMEMBER TO FREE valuesStringPtr
        char* valuesStringPtr = valuesString;

        //Get rid of the concatenated \n at the beginning of the string due to toString
        valuesString++;
	int valuesStringLength = 0;
        if (valuesString == NULL) {
            valuesStringLength = 0;
	}
	else {
            valuesStringLength = (int)(strlen(valuesString));
	}

        //Get flexible array size
        flexSize = valuesStringLength + 1;

        //Check if Z is at the end of the values line - UTC = true if yes
        if (valuesString[valuesStringLength - 1] == 'Z') {
            //Get rid of 'Z'
            valuesString[valuesStringLength - 1] = '\0';
            //Set UTC = true
            UTC = true;
        }

        if (currentProperty->parameters->length > 0) {
            Node* temp = currentProperty->parameters->head;
            while (temp != NULL) {
                Parameter* tempParam = (Parameter*)(temp->data);
                if (stringCaseCompare(tempParam->name, "VALUE") == 0) {
                    if (stringCaseCompare(tempParam->value, "text") == 0) {
                        isText = true;
                        text = calloc(flexSize, sizeof(char));
                        strcpy(text, valuesString);
                    }
                    else if (stringCaseCompare(tempParam->value, "date-and-or-time") == 0) {
                        //Do Nothing
                    }
                    else {
                        //INVProp
                    }
		}
		temp = temp->next;
            }
        }

        //Tokenize on 'T' to get date/time values if isText == false
        if (isText == false) {
            if (strchr(valuesString, 'T') != NULL) {
		if (valuesString[0] == 'T') {
                    //Only a time
		    valuesString = strtok(valuesString, "T");
		    time = calloc(strlen(valuesString) + 1, sizeof(char));
		    strcpy(time, valuesString);
		}
		else {
                    valuesString = strtok(valuesString, "T");
                    date = calloc(strlen(valuesString) + 1, sizeof(char));
                    strcpy(date, valuesString);

                    valuesString = strtok(NULL, "T");
		    if (valuesString == NULL) {
                        time = calloc(1, sizeof(char));
		        strcpy(time, "");
		    }
		    else {
                        time = calloc(strlen(valuesString) + 1, sizeof(char));
                        strcpy(time, valuesString);
		    }
		}
            }
            else {
                date = calloc(strlen(valuesString) + 1, sizeof(char));
                strcpy(date, valuesString);
            }
        }

        free(valuesStringPtr);
    }

    DateTime* newDate = createDateTime(flexSize);
    newDate->UTC = UTC;
    newDate->isText = isText;
    if (isText == false) {
        if (date != NULL) {
            strcpy(newDate->date, date);
            free(date);
        }
        if (time != NULL) {
            strcpy(newDate->time, time);
            free(time);
        }
    }
    else {
        strcpy(newDate->text, text);
        free(text);
    }

    return newDate;
}

bool validateDateTime(DateTime* currentDateTime) {
    if (currentDateTime->isText) {
        //Check text[]
    }
    else {
        //Check date[9] && Check time[7]
        //Confirm every character in each is a digit (If it is not empty)
        if (currentDateTime->date[0] != '\0') {
            int dateLength = (int)(strlen(currentDateTime->date));
            for (int i = 0; i < dateLength; i++) {
                if (!isdigit(currentDateTime->date[i])) {
                    if (currentDateTime->date[i] != '-') {
                        return false;
                    }
                }
            }

            //No year
            if (currentDateTime->date[0] == '-' && currentDateTime->date[1] == '-') {
                //No month
                if (currentDateTime->date[2] == '-') {
                    //No month
                }
                //Yes month
                else {
                    if (currentDateTime->date[2] == '1') {
                        if (currentDateTime->date[3] != '1' || currentDateTime->date[3] != '2' || currentDateTime->date[6] != '-') {
                            return false;
                        }
                    }
                }
            }
            //Yes year - can move to date[4]
            else {
                if (currentDateTime->date[4] == '-') {
                    if (currentDateTime->date[5] == '1') {
                        if (currentDateTime->date[6] != '1' || currentDateTime->date[6] != '2' || currentDateTime->date[6] != '-') {
                            return false;
                        }
                    }
                }
                else {
                    if (currentDateTime->date[4] == '1') {
                        if (currentDateTime->date[5] != '1' || currentDateTime->date[5] != '2' || currentDateTime->date[5] != '-') {
                            return false;
                        }
                    }
                }
            }
        }

        if (currentDateTime->time[0] != '\0') {
            int timeLength = (int) (strlen(currentDateTime->time));
            for (int j = 0; j < timeLength; j++) {
                if (!isdigit(currentDateTime->time[j])) {
                    if (currentDateTime->time[j] != '-') {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

char* groupCheck(char* line) {
    char lineCopy[998];
    lineCopy[0] = '\0';
    strcpy(lineCopy, line);
    char* returned = strchr(lineCopy, '.');
    if (returned != NULL) {
        *returned = '\0';
        if (numberOfCharacters(lineCopy, ';') > 0 || numberOfCharacters(lineCopy, ':') > 0) {
            return NULL;
        }
        else {
            char* toReturn = calloc(strlen(lineCopy) + 1, sizeof(char));
            strcpy(toReturn, lineCopy);
            return toReturn;
        }
    }
    return NULL;
}

VCardErrorCode propertyCheck(char* line) {
    char* lineCopy = NULL;
    lineCopy = calloc(strlen(line) + 1, sizeof(char));
    strcpy(lineCopy, line);

    char* lineCopyPointer = lineCopy;

    lineCopy = strtok(lineCopy, ";:");
    lineCopy = strtok(NULL, ";:");
    if (lineCopy == NULL || strcmp(lineCopy, "") == 0) {
	    free(lineCopyPointer);
        return INV_PROP;
    }
    free(lineCopyPointer);
    return OK;
}

bool verifyOnlyOnce(char* line) {
    //Check first line of VCard
    if (stringCaseCompare(line, "BEGIN:VCARD") == 0) {
        return false;
    }

    //Check second line of VCard
    if (stringCaseCompare(line, "VERSION:4.0") == 0) {
        return false;
    }

    //Check last line of VCard
    if (stringCaseCompare(line, "END:VCARD") == 0) {
        return false;
    }

    return true;
}

void writeProperty(FILE* file, Property* toWrite) {
    if (file != NULL && toWrite != NULL) {
        if (strcmp(toWrite->group, "") != 0) {
            //Write group
            fprintf(file, "%s.", toWrite->group);
        }

        //Write Property* name with appropriate ; OR :
        if (toWrite->parameters->length > 0) {
            fprintf(file, "%s;", toWrite->name);
        }
        else {
            fprintf(file, "%s:", toWrite->name);
        }

        //Check for Parameters*
        if (toWrite->parameters->length > 0) {
            Node* temp = toWrite->parameters->head;
            while (temp != NULL) {
                //Check if at tail
                if (temp->next == NULL) {
                    Parameter* currentParam = (Parameter*)(temp->data);
                    fprintf(file, "%s=%s:", currentParam->name, currentParam->value);
                    temp = temp->next;
                }
                else {
                    Parameter* currentParam = (Parameter*)(temp->data);
                    fprintf(file, "%s=%s;", currentParam->name, currentParam->value);
                    temp = temp->next;
                }
            }
        }

        //Check for Property* values
        if (toWrite->values->length > 0) {
            Node* temp = toWrite->values->head;
            while (temp != NULL) {
                //Check if at tail
                if (temp->next == NULL) {
                    if (strcmp((char*)(temp->data), "") == 0) {
                        fprintf(file, "%s;\r\n", (char*)(temp->data));
                    }
                    else {
                        fprintf(file, "%s\r\n", (char*)(temp->data));
                    }
                    temp = temp->next;
                }
                else {
                    fprintf(file, "%s;", (char*)(temp->data));
                    temp = temp->next;
                }
            }
        }
    }
}

void writeDateTime(FILE* file, DateTime* toWrite, bool isAnniversary, bool isBDay) {
    if (file == NULL || toWrite == NULL) {
        return;
    }

    if (isAnniversary) {
        fprintf(file, "ANNIVERSARY");
    }

    if (isBDay) {
        fprintf(file, "BDAY");
    }

    if (toWrite->isText) {
        if (toWrite->UTC) {
            //Must append a 'Z' to the end
            fprintf(file, ";VALUE=text:%sZ\r\n", toWrite->text);
        }
        else {
            fprintf(file, ";VALUE=text:%s\r\n", toWrite->text);
        }
    }
    else {
        fprintf(file, ":");

        if (toWrite->UTC) {
            //Must append a 'Z' to the end
            if (toWrite->date[0] != '\0') {
                fprintf(file, "%s", toWrite->date);
            }
            if (toWrite->time[0] != '\0') {
                fprintf(file, "T%s", toWrite->time);
            }
            fprintf(file, "Z\r\n");
        }
        else {
            if (toWrite->date[0] != '\0') {
                fprintf(file, "%s", toWrite->date);
            }
            if (toWrite->time[0] != '\0') {
                fprintf(file, "T%s", toWrite->time);
            }
            fprintf(file, "\r\n");
        }
    }
}

void populatePropertyList(List* toPopulate) {

    if (toPopulate == NULL) {
        return;
    }

    /**
     * Cardinality:
     * 1 - Exactly one instance MUST be present in card
     * * - One or more instances MAY be present in card
     * *1 - Exactly one instance MAY be present in card
     * 1* - One or more instances MUST be present in card
     */
    //Insert all valid properties into list toPopulate (BEGIN/VERSION/END not added - may only appear once and validated elsewhere)
    char* source = calloc(7, sizeof(char));
    strcpy(source, "SOURCE");
    insertBack(toPopulate, source); //Cardinality: * - Property values count: 1
    char* kind = calloc(5, sizeof(char));
    strcpy(kind, "KIND");
    insertBack(toPopulate, kind); //Cardinality: *1 - Property values count: 1
    char* xml = calloc(4, sizeof(char));
    strcpy(xml, "XML");
    insertBack(toPopulate, xml); //Cardinality: * - Property values count: 1
    char* fn = calloc(3, sizeof(char));
    strcpy(fn, "FN");
    insertBack(toPopulate, fn); //Cardinality: 1* - Property values count: 1
    char* n = calloc(2, sizeof(char));
    strcpy(n, "N");
    insertBack(toPopulate, n); //Cardinality: *1 - Property values count: 5
    char* nickname = calloc(9, sizeof(char));
    strcpy(nickname, "NICKNAME");
    insertBack(toPopulate, nickname); //Cardinality: * - Property values count: * (List - Can have unlimited values)
    char* photo = calloc(6, sizeof(char));
    strcpy(photo, "PHOTO");
    insertBack(toPopulate, photo); //Cardinality: * - Property values count: 1
    char* gender = calloc(7, sizeof(char));
    strcpy(gender, "GENDER");
    insertBack(toPopulate, gender); //Cardinality: *1 - Property values count: 1-2 (May have text after the first value)
    char* adr = calloc(4, sizeof(char));
    strcpy(adr, "ADR");
    insertBack(toPopulate, adr); //Cardinality: * - Property values count: 7
    char* tel = calloc(4, sizeof(char));
    strcpy(tel, "TEL");
    insertBack(toPopulate, tel); //Cardinality: * - Property values count: 1-2
    char* email = calloc(6, sizeof(char));
    strcpy(email, "EMAIL");
    insertBack(toPopulate, email); //Cardinality: * - Property values count: 1
    char* impp = calloc(5, sizeof(char));
    strcpy(impp, "IMPP");
    insertBack(toPopulate, impp); //Cardinality: * - Property values count: 1
    char* lang = calloc(5, sizeof(char));
    strcpy(lang, "LANG");
    insertBack(toPopulate, lang); //Cardinality: * - Property values count: 1
    char* tz = calloc(3, sizeof(char));
    strcpy(tz, "TZ");
    insertBack(toPopulate, tz); //Cardinality: * - Property values count: 1
    char* geo = calloc(4, sizeof(char));
    strcpy(geo, "GEO");
    insertBack(toPopulate, geo); //Cardinality: * - Property values count: 1
    char* title = calloc(6, sizeof(char));
    strcpy(title, "TITLE");
    insertBack(toPopulate, title); //Cardinality: * - Property values count: 1
    char* role = calloc(5, sizeof(char));
    strcpy(role, "ROLE");
    insertBack(toPopulate, role); //Cardinality: * - Property values count: 1
    char* logo = calloc(5, sizeof(char));
    strcpy(logo, "LOGO");
    insertBack(toPopulate, logo); //Cardinality: * - Property values count: 1
    char* org = calloc(4, sizeof(char));
    strcpy(org, "ORG");
    insertBack(toPopulate, org); //Cardinality: * - Property values count: * (Can have unlimited values)
    char* member = calloc(7, sizeof(char));
    strcpy(member, "MEMBER");
    insertBack(toPopulate, member); //Cardinality: * - Property values count: 1
    char* related = calloc(8, sizeof(char));
    strcpy(related, "RELATED");
    insertBack(toPopulate, related); //Cardinality: * - Property values count: 1
    char* categories = calloc(11, sizeof(char));
    strcpy(categories, "CATEGORIES");
    insertBack(toPopulate, categories); //Cardinality: * - Property values count: * (List - Can have unlimited values)
    char* note = calloc(5, sizeof(char));
    strcpy(note, "NOTE");
    insertBack(toPopulate, note); //Cardinality: * - Property values count: 1
    char* prodid = calloc(7, sizeof(char));
    strcpy(prodid, "PRODID");
    insertBack(toPopulate, prodid); //Cardinality: *1 - Property values count: 1
    char* rev = calloc(4, sizeof(char));
    strcpy(rev, "REV");
    insertBack(toPopulate, rev); //Cardinality: *1 - Property values count: 1
    char* sound = calloc(6, sizeof(char));
    strcpy(sound, "SOUND");
    insertBack(toPopulate, sound); //Cardinality: * - Property values count: 1
    char* uid = calloc(4, sizeof(char));
    strcpy(uid, "UID");
    insertBack(toPopulate, uid); //Cardinality: *1 - Property values count: 1
    char* clientpidmap = calloc(13, sizeof(char));
    strcpy(clientpidmap, "CLIENTPIDMAP");
    insertBack(toPopulate, clientpidmap); //Cardinality: * - Property values count: 2
    char* url = calloc(4, sizeof(char));
    strcpy(url, "URL");
    insertBack(toPopulate, url); //Cardinality: * - Property values count: 1
    char* key = calloc(4, sizeof(char));
    strcpy(key, "KEY");
    insertBack(toPopulate, key); //Cardinality: * - Property values count: 1
    char* fburl = calloc(6, sizeof(char));
    strcpy(fburl, "FBURL");
    insertBack(toPopulate, fburl); //Cardinality: * - Property values count: 1
    char* caladruri = calloc(10, sizeof(char));
    strcpy(caladruri, "CALADRURI");
    insertBack(toPopulate, caladruri); //Cardinality: * - Property values count: 1
    char* caluri = calloc(7, sizeof(char));
    strcpy(caluri, "CALURI");
    insertBack(toPopulate, caluri); //Cardinality: * - Property values count: 1
}

VCardErrorCode errorCheckProperty(Property* toCheck) {
    if (toCheck == NULL) {
        return INV_PROP;
    }

    if (toCheck->name == NULL || strcmp(toCheck->name, "") == 0) {
        return INV_PROP;
    }

    if (toCheck->group == NULL) {
        return INV_PROP;
    }

    if (toCheck->parameters == NULL) {
        return INV_PROP;
    }

    if (toCheck->parameters->length > 0) {
        Node* currentParam = toCheck->parameters->head;
        while (currentParam != NULL) {
            Parameter* currentParameter = (Parameter*)(currentParam->data);
            if (currentParameter->name[0] == '\0') {
                return INV_PROP;
            }
            if (currentParameter->value[0] == '\0') {
                return INV_PROP;
            }
            currentParam = currentParam->next;
        }
    }

    if (toCheck->values == NULL || toCheck->values->length < 1) {
        return INV_PROP;
    }

    Node* currentValue = toCheck->values->head;
    while (currentValue != NULL) {
        if (currentValue->data == NULL) {
            return INV_PROP;
        }
        currentValue = currentValue->next;
    }


    return OK;
}

VCardErrorCode errorCheckDateTime(DateTime* toCheck) {
    if (toCheck == NULL) {
        return INV_DT;
    }

    //Verify isText constraints
    if (toCheck->isText == true) {
        //Verify that date[9] && time[7] are empty strings && UTC == false
        if (toCheck->date[0] != '\0') {
            return INV_DT;
        }
        if (toCheck->time[0] != '\0') {
            return INV_DT;
        }
        if (toCheck->UTC == true) {
            return INV_DT;
        }
    }
    else {
        //Verify date[9] && time[7] have correct format
        int dateLength = (int)(strlen(toCheck->date));
        //Confirm all characters in date[9] are either a number or a '-'
        for (int i = 0; i < dateLength; i++) {
            if (!isdigit(toCheck->date[i])) {
                if (toCheck->date[i] != '-') {
                    return INV_DT;
                }
            }
        }

        //No year
        if (toCheck->date[0] == '-' && toCheck->date[1] == '-') {
            if (toCheck->date[2] == '-') {
                //No month
            }
            else {
                //Yes month
                if (toCheck->date[2] == '1') {
                    if (toCheck->date[3] != '1' && toCheck->date[3] != '2' && toCheck->date[6] != '-') {
                        return INV_DT;
                    }
                }
            }
        }
        //Yes year - can move to date[4]
        else {
            if (toCheck->date[4] == '-') {
                if (toCheck->date[5] == '1') {
                    if (toCheck->date[6] != '1' && toCheck->date[6] != '2' && toCheck->date[6] != '-') {
                        return INV_DT;
                    }
                }
            }
            else {
                if (toCheck->date[4] == '1') {
                    if (toCheck->date[5] != '1' && toCheck->date[5] != '2' && toCheck->date[5] != '-') {
                        return INV_DT;
                    }
                }
            }
        }

        //Confirm all characters in time[7] are either a number or a '-'
        int timeLength = (int)(strlen(toCheck->time));
        for (int j = 0; j < timeLength; j++) {
            if (!isdigit(toCheck->time[j])) {
                if (toCheck->time[j] != '-') {
                    return INV_DT;
                }
            }
        }
    }

    return OK;
}

VCardErrorCode checkPropertyValueCount(Property* toCheck) {
    if (toCheck == NULL) {
        return INV_PROP;
    }

    if (stringCaseCompare(toCheck->name, "N") == 0) {
        if (toCheck->values->length != 5) {
            return INV_PROP;
        }
    }
    else if (stringCaseCompare(toCheck->name, "ADR") == 0) {
        if (toCheck->values->length != 7) {
            return INV_PROP;
        }
    }
    else if (stringCaseCompare(toCheck->name, "NICKNAME") == 0 || stringCaseCompare(toCheck->name, "ORG") == 0 || stringCaseCompare(toCheck->name, "CATEGORIES") == 0) {
        if (toCheck->values->length <= 0) {
            return INV_PROP;
        }
    }
    else if (stringCaseCompare(toCheck->name, "GENDER") == 0 || stringCaseCompare(toCheck->name, "TEL") == 0) {
        if (toCheck->values->length <= 0 || toCheck->values->length > 2) {
            return INV_PROP;
        }
    }
    else if (stringCaseCompare(toCheck->name, "CLIENTPIDMAP") == 0) {
        if (toCheck->values->length != 2) {
            return INV_PROP;
        }
    }
    else {
        if (toCheck->values->length != 1) {
            return INV_PROP;
        }
    }

    return OK;
}

bool listPropertyNameSearch(List* validPropertyList, char* propertyName) {
    if (validPropertyList == NULL || propertyName == NULL) {
        return false;
    }

    Node* current = validPropertyList->head;

    while (current != NULL) {
        if (stringCaseCompare(current->data, propertyName) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

char* substring(int startIndex, char* str) {
    if (str == NULL) {
        return NULL;
    }

    int strLength = (int)(strlen(str));
    char* result = calloc(strLength, sizeof(char));
    int i = 0;

    if (startIndex > 0) {
        while (i < strLength) {
            result[i] = str[startIndex + i];
            i++;
        }
    }
    result[i] = '\0';

    return result;
}

void removeChar(char* stringToCheck, char characterToRemove) {
    int j, n = (int)(strlen(stringToCheck));

    for (int i = j = 0; i < n; i++) {
        if (stringToCheck[i] != characterToRemove) {
            stringToCheck[j++] = stringToCheck[i];
        }
    }
    stringToCheck[j] = '\0';
}

char* getFileLog(char* fileName) {
    if (fileName == NULL) {
        return NULL;
    }

    Card* object = NULL;
    VCardErrorCode err = createCard(fileName, &object);
    char* errorCode = printError(err);
    if (strcmp(errorCode, "OK") != 0) {
        return NULL;
    }

    free(errorCode);

    VCardErrorCode err2 = validateCard(object);
    char* errorCode2 = printError(err2);
    if (strcmp(errorCode2, "OK") != 0) {
        return NULL;
    }

    free(errorCode2);

    //Get object->fn->name
    char* FNName = calloc((int)(strlen(object->fn->values->head->data)) + 1, sizeof(char));
    strcpy(FNName, object->fn->values->head->data);

    //Get object->optionalProperties->length
    int optPropLength = object->optionalProperties->length;

    if (object->anniversary != NULL) {
        optPropLength++;
    }
    if (object->birthday != NULL) {
        optPropLength++;
    }

    //Create JSON to return
    //{"indiname":"Simon Perreault","addiprops":"15"}
    int FNNameLength = (int)(strlen(FNName)) + 1;
    char* toReturn = calloc(100 + FNNameLength, sizeof(char));
    sprintf(toReturn, "{\"indiname\":\"%s\",\"addiprops\":\"%d\"}", FNName, optPropLength);

    deleteCard(object);

    return toReturn;
}

char* getCardView(char* fileName) {
    //ex. return value (Array of property objects):
    //[{"propname":"FN","propvalues":"Simon Perrault"},{"propname":"N","propvalues":"Perrault, Simon, ing. jr, M.Sc."}]

    if (fileName == NULL) {
        return NULL;
    }

    Card* object = NULL;
    VCardErrorCode err = createCard(fileName, &object);
    char* errorCode = printError(err);
    if (strcmp(errorCode, "OK") != 0) {
        return NULL;
    }

    free(errorCode);

    VCardErrorCode err2 = validateCard(object);
    char* errorCode2 = printError(err2);
    if (strcmp(errorCode2, "OK") != 0) {
        return NULL;
    }

    free(errorCode2);

    char buffer[9000];
    //char* toReturn = calloc(50, sizeof(char));
    strcpy(buffer, "[{\"propname\":\"FN\",\"propvalues\":\"");

    //Get object->fn->values
    if (object->fn->values > 0) {
        Node* current = object->fn->values->head;
        while (current) {
            //Add current->data to buffer
            strcat(buffer, current->data);

            if (current->next == NULL) {
                strcat(buffer, "\"");
            }
            else {
                strcat(buffer, ", ");
            }

            current = current->next;
        }

        strcat(buffer, "}");
    }

    //Get object->birthday if not NULL
    if (object->birthday != NULL) {
        strcat(buffer, ",{\"propname\":\"BDAY\",");

        //Check if isText == true
        if (object->birthday->isText) {
            strcat(buffer, "\"propvalues\":\"");
            strcat(buffer, object->birthday->text);
            strcat(buffer, "\"}");
        }
        else {
            if (object->birthday->time[0] != '\0') {
                strcat(buffer, "\"propvalues\":\"");
                strcat(buffer, object->birthday->time);
                if (object->birthday->date[0] != '\0') {
                    strcat(buffer, object->birthday->date);
                    strcat(buffer, "\"}");
                }
                else {
                    strcat(buffer, "\"}");
                }
            }
            else {
                if (object->birthday->date[0] != '\0') {
                    strcat(buffer, "\"propvalues\":\"");
                    strcat(buffer, object->birthday->date);
                    strcat(buffer, "\"}");
                }
            }
        }
    }

    //Get object->anniversary if not NULL
    if (object->anniversary != NULL) {
        strcat(buffer, ",{\"propname\":\"ANNIVERSARY\",");

        //Check if isText == true
        if (object->anniversary->isText) {
            strcat(buffer, "\"propvalues\":\"");
            strcat(buffer, object->anniversary->text);
            strcat(buffer, "\"}");
        }
        else {
            if (object->anniversary->time[0] != '\0') {
                strcat(buffer, "\"propvalues\":\"");
                strcat(buffer, object->anniversary->time);
                if (object->anniversary->date[0] != '\0') {
                    strcat(buffer, object->anniversary->date);
                    strcat(buffer, "\"}");
                }
                else {
                    strcat(buffer, "\"}");
                }
            }
            else {
                if (object->anniversary->date[0] != '\0') {
                    strcat(buffer, "\"propvalues\":\"");
                    strcat(buffer, object->anniversary->date);
                    strcat(buffer, "\"}");
                }
            }
        }
    }

    //Get object->optionalProperties if not empty
    if (object->optionalProperties->length > 0) {
        strcat(buffer, ",");
        Node* current = object->optionalProperties->head;
        while (current) {
            Property* currentProperty = (Property*)(current->data);
            //Get current property name
            strcat(buffer, "{\"propname\":\"");
            strcat(buffer, currentProperty->name);
            strcat(buffer, "\",");

            //Get current property values
            if (currentProperty->values->length > 0) {
                strcat(buffer, "\"propvalues\":\"");
                Node* currentValue = currentProperty->values->head;
                while (currentValue) {
                    char* currentValueString = (char*)(currentValue->data);
                    if (strcmp(currentValueString, "") != 0) {
                        //Concatenate the value to buffer
                        strcat(buffer, currentValueString);
                        if (currentValue->next != NULL) {
                            if (strcmp((char*)(currentValue->next->data), "") != 0) {
                                strcat(buffer, ", ");
                            }
                        }
                    }
                    currentValue = currentValue->next;
                }
                strcat(buffer, "\"");
            }

            if (current->next == NULL) {
                strcat(buffer, "}");
            }
            else {
                strcat(buffer, "},");
            }

            current = current->next;
        }
    }

    strcat(buffer, "]");

    deleteCard(object);

    char* toReturn = calloc(strlen(buffer), sizeof(char));
    strcpy(toReturn, buffer);
    return toReturn;
}
