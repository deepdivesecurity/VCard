/**
 * @file VCardParser.c
 * @brief This file contains the VCard file's main parsing functionality.
 * @author ADD LATER
 */

//NOTE: REMEMBER TO CHANGE HARDCODED FILE LOCATIONS
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "LinkedListAPI.h"
#include "VCardParser.h"
#include "HelperFunctions.h"
#define DEBUG false

VCardErrorCode createCard(char* fileName, Card** newCardObject) {
    //Malloc the Card** structure and initialize it
    //NOTE: REMEMBER TO FREE newCard
    *newCardObject = malloc(sizeof(Card));
    Card* newCard = *newCardObject;
    newCard->fn = NULL;
    newCard->anniversary = NULL;
    newCard->birthday = NULL;
    newCard->optionalProperties = initializeList(printProperty, deleteProperty, compareProperties);

    //Create error code to return
    VCardErrorCode errorToReturn;

    //Check file name and extension
    if (verifyFileName(fileName) == false) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;
        errorToReturn = INV_FILE;
        return errorToReturn;
    }

    //Read in the file to a string
    //Note: Remember to free returnedString
    char* returnedString = readFileToString(fileName);

    //Check if file was read in correctly
    if (returnedString == NULL) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;
        errorToReturn = INV_FILE;
        return errorToReturn;
    }

    //Check that every \n has a \r before it
    if (verifyStringEndOfLines(returnedString) == false) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;
        free(returnedString);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }

    //Go through string looking for commas in between quotation marks - Remove backslash before them
    removeBackslash(returnedString);

    //Declare variables
    int numberOfLines = 0;
    int i = 0;

    //Split the file contents and unfold
    //Note: Remember to free lines 2d array - Requires freeing all strings inside table, then freeing table
    char** lines = split(returnedString, &numberOfLines, "\r\n", true);

    //Free the single string of file content
    free(returnedString);
    returnedString = NULL;

    //Check first line of VCard
    if (stringCaseCompare(lines[0], "BEGIN:VCARD") != 0) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;

        //Free all lines of the 2d string array
        for (i = 0; i < numberOfLines; i++) {
            free(lines[i]);
        }
        //Free the 2d array
        free(lines);

        //Return INV_CARD
        errorToReturn = INV_CARD;
        return errorToReturn;
    }

    //Check second line of VCard
    if (stringCaseCompare(lines[1], "VERSION:4.0") != 0) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;

        //Free all lines of the 2d string array
        for (i = 0; i < numberOfLines; i++) {
            free(lines[i]);
        }
        //Free the 2d array
        free(lines);

        //Return INV_CARD
        errorToReturn = INV_CARD;
        return errorToReturn;
    }

    //Check last line of VCard
    if (stringCaseCompare(lines[numberOfLines - 1], "END:VCARD") != 0) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;

        //Free all lines of the 2d string array
        for (i = 0; i < numberOfLines; i++) {
            free(lines[i]);
        }
        //Free the 2d array
        free(lines);

        //Return INV_CARD
        errorToReturn = INV_CARD;
        return errorToReturn;
    }

    //Error check line lengths (MUST not be more than 998 characters)
    for (i = 0; i < numberOfLines; i++) {
        if (strlen(lines[i]) > 998) {
            deleteCard(newCard);
            newCard = NULL;
            *newCardObject = newCard;

            //Free all lines of the 2d string array
            for (i = 0; i < numberOfLines; i++) {
                free(lines[i]);
            }
            //Free the 2d array
            free(lines);

            //Return INV_CARD
            errorToReturn = INV_PROP;
            return errorToReturn;
        }
    }

    //Check if the line contains a ':', if not, return INV_PROP
    for (i = 0; i < numberOfLines; i++) {
        if (numberOfCharacters(lines[i], ':') == 0) {
            deleteCard(newCard);
            newCard = NULL;
            *newCardObject = newCard;

            //Free all lines of the 2d string array
            for (i = 0; i < numberOfLines; i++) {
                free(lines[i]);
            }
            //Free the 2d array
            free(lines);
            //INV_PROP
            errorToReturn = INV_PROP;
            return INV_PROP;
        }
    }

    //Go through the lines creating structures
    bool FNSet = false;
    bool BDAYSet = false;
    bool anniversarySet = false;
    //TODO: Error check DateTime property specs
    for (i = 2; i < numberOfLines - 1; i++) {
        bool groupSet = false;

        //Make sure the line isn't BEGIN/END/VERSION
        if (verifyOnlyOnce(lines[i]) == false) {

            //Free all lines of the 2d string array
            for (i = 0; i < numberOfLines; i++) {
                free(lines[i]);
            }
            //Free the 2d array
            free(lines);

            deleteCard(newCard);
            newCard = NULL;
            *newCardObject = newCard;

            errorToReturn = INV_PROP;
            return errorToReturn;
        }

        //NOTE: REMEMBER TO FREE newProperty
        Property* newProperty = createProperty();

        int currentLineLength = (int)(strlen(lines[i]));
        char* currentLine = calloc(currentLineLength + 1, sizeof(char));
        strcpy(currentLine, lines[i]);

        //NOTE: REMEMBER TO FREE currentLinePointer
        char* currentLinePointer = currentLine;

        //Values check
        //returned = All values after furthest-left ':'
        char* returned = strchr(currentLine, ':');
        returned++;

        if (returned == NULL || strcmp(returned, "") == 0) {
            free(currentLine);

            //Free all lines of the 2d string array
            for (i = 0; i < numberOfLines; i++) {
                free(lines[i]);
            }
            //Free the 2d array
            free(lines);

            deleteProperty(newProperty);
            deleteCard(newCard);
            newCard = NULL;
            *newCardObject = newCard;

            errorToReturn = INV_PROP;
            return errorToReturn;
        }

        //Group check and set
        char* group = groupCheck(currentLine);
        if (group != NULL) {
            newProperty->group = calloc(strlen(group) + 1, sizeof(char));
            strcpy(newProperty->group, group);
            if(DEBUG)printf("group: %s\n", group);
            free(group);
            groupSet = true;
        }

	    VCardErrorCode propCheck = propertyCheck(currentLine);
	    if (propCheck != OK) {
	        free(currentLine);
	        for (i = 0; i < numberOfLines; i++) {
                free(lines[i]);
	        }
	        free(lines);
	        deleteProperty(newProperty);
	        deleteCard(newCard);
	        newCard = NULL;
	        *newCardObject = newCard;
	        errorToReturn = INV_PROP;
	        return errorToReturn;
	    }

        //If there's no semi-colon in the string, then everything to the left of the colon is the property
        //NOTE: if statement deals with string that has no parameters and only one value (Not a list of values)
        if (numberOfCharacters(currentLine, ';') == 0) {
            //Call function to store property name and values
            noParamOneValue(newCard, newProperty, currentLine, returned, groupSet);
        }
        //NOTE: else statement deals with parameters or a list of Property* structure values
        else {
            //NOTE: REMEMBER TO FREE parameterList
            List* parameterList = initializeList(printValue, deleteValue, compareValues);

            //Otherwise must check parameters (returned == values) && (currentLine == Property + parameters)
            //Everything before the first semi-colon is a property - Read until first ';' to get property
            char* dupCurrentLine = calloc(currentLineLength + 1, sizeof(char));
            strcpy(dupCurrentLine, currentLine);
            //NOTE: REMEMBER TO FREE dupCurrentLinePointer
            char* dupCurrentLinePointer = dupCurrentLine;

            //Recheck how many ':' are found in current line - if there is a colon, all of the ';' separate values of the Property* structure
            //NOTE: if statement deals with parameters and values if there are any
	        currentLine = strtok(currentLine, ";");

            if (numberOfCharacters(currentLine, ':') == 0) {

                params(newCard, newProperty, parameterList, currentLine, returned, groupSet);

                //Go through parameter list and create/add Parameter*s to Property*
                VCardErrorCode addError = addParametersToProperty(newProperty, parameterList);
		        if (addError != OK) {
                    free(dupCurrentLinePointer);
		            freeList(parameterList);
	                free(currentLine);
	                for (i = 0; i < numberOfLines; i++) {
                        free(lines[i]);
	                }
	                free(lines);
	                deleteProperty(newProperty);
                    deleteCard(newCard);
	                newCard = NULL;
                    *newCardObject = newCard;
                    errorToReturn = INV_PROP;
                    return errorToReturn;
		        }
            }
            //NOTE: else statement deals with no parameters but multiple Property* structure values
            else {
                //Should be a property followed by a list of values - delimit on ';' && ':'
                noParamMultipleValues(newCard, newProperty, dupCurrentLine, returned, groupSet);
            }
            free(dupCurrentLinePointer);
            freeList(parameterList);
        }

        free(currentLinePointer);

        //Check if newProperty->name valid (Verify working)
        if (newProperty->name == NULL || strcmp(newProperty->name, "") == 0) {
            //Free all lines of the 2d string array
            for (i = 0; i < numberOfLines; i++) {
                free(lines[i]);
            }
            //Free the 2d array
            free(lines);

            deleteCard(newCard);
            newCard = NULL;
            *newCardObject = newCard;

            errorToReturn = INV_PROP;
            return errorToReturn;
        }

        //If a group has not been set, initialize it with an empty string
        if (newProperty->group == NULL) {
            newProperty->group = calloc(1, sizeof(char));
            strcpy(newProperty->group, "");
        }

        //Set the property to one of the three main card structures (FN/BDAY/ANNIVERSARY) - Or insert to optionalProperties
        if (stringCaseCompare(newProperty->name, "FN") == 0 && FNSet == false) {
            //Found FN property - Set newCard->fn = newProperty
            newCard->fn = newProperty;
            //Set FNSet = true
            FNSet = true;
        }
        else if (stringCaseCompare(newProperty->name, "BDAY") == 0 && BDAYSet == false) {
            DateTime* newBDay = parseDate(newProperty);
            if (validateDateTime(newBDay) != true) {
                deleteProperty(newProperty);
		        deleteDate(newBDay);

                //Free all lines of the 2d string array
                for (i = 0; i < numberOfLines; i++) {
                    free(lines[i]);
                }
                //Free the 2d array
                free(lines);

                deleteCard(newCard);
                newCard = NULL;
                *newCardObject = newCard;

                errorToReturn = INV_PROP;
                return errorToReturn;
            }
            BDAYSet = true;
            //Delete Property* structure
            deleteProperty(newProperty);
            newCard->birthday = newBDay;
        }
        else if (stringCaseCompare(newProperty->name, "ANNIVERSARY") == 0 && anniversarySet == false) {
            DateTime* newAnniversary = parseDate(newProperty);
            if (validateDateTime(newAnniversary) != true) {
                deleteProperty(newProperty);
                deleteDate(newAnniversary);

                //Free all lines of the 2d string array
                for (i = 0; i < numberOfLines; i++) {
                    free(lines[i]);
                }
                //Free the 2d array
                free(lines);

                deleteCard(newCard);
                newCard = NULL;
                *newCardObject = newCard;

                errorToReturn = INV_PROP;
                return errorToReturn;
            }
            anniversarySet = true;
            //Delete Property* structure
            deleteProperty(newProperty);
            newCard->anniversary = newAnniversary;
        }
        else {
            insertBack(newCard->optionalProperties, newProperty);
        }
    }

    //Free all lines of the 2d string array
    for (i = 0; i < numberOfLines; i++) {
        free(lines[i]);
    }
    //Free the 2d array
    free(lines);

    //Check if FN was set, if not, return INV_CARD
    if (FNSet == false) {
        deleteCard(newCard);
        newCard = NULL;
        *newCardObject = newCard;

        errorToReturn = INV_CARD;
        return errorToReturn;
    }

    //Return OK on successful card creation
    *newCardObject = newCard;
    errorToReturn = OK;
    return errorToReturn;
}

VCardErrorCode writeCard(const char* fileName, const Card* obj) {
    //TODO: Confirm empty values get written correctly
    //Create error code to return
    VCardErrorCode errorToReturn;

    //Check if Card* object is NULL
    if (obj == NULL) {
        errorToReturn = WRITE_ERROR;
        return errorToReturn;
    }

    //Make a copy of the fileName if it's not NULL
    char* fileNameCopy = NULL;
    if (fileName != NULL) {
        //NOTE: REMEMBER TO FREE fileNameCopy
        fileNameCopy = calloc(strlen(fileName) + 1, sizeof(char));
        strcpy(fileNameCopy, fileName);
    }
    else {
        errorToReturn = WRITE_ERROR;
        return errorToReturn;
    }

    //Check file name and extension
    if (verifyFileName(fileNameCopy) == false) {
        errorToReturn = WRITE_ERROR;
        return errorToReturn;
    }

    free(fileNameCopy);

    //Create file and open it in append mode (This will create a new file if one hasn't been created)
    FILE* file;
    file = fopen(fileName, "w");

    //Check if the file could open
    if (file == NULL) {
        //Error: File could not be opened
        errorToReturn = WRITE_ERROR;
        return errorToReturn;
    }

    //Print the initial headings that must be in a VCard
    fprintf(file, "BEGIN:VCARD\r\nVERSION:4.0\r\n");

    //Call writeProperty for FN
    writeProperty(file, obj->fn);

    //Call writeDateTime for Anniversary
    if (obj->anniversary != NULL) {
        writeDateTime(file, obj->anniversary, true, false);
    }

    //Call writeDateTime for BDay
    if (obj->birthday != NULL) {
        writeDateTime(file, obj->birthday, false, true);
    }

    //Call writeProperty for all optional Property* structures
    if (obj->optionalProperties->length > 0) {
        Node* temp = obj->optionalProperties->head;
        while (temp != NULL) {
            Property* toWrite = (Property*)(temp->data);
            writeProperty(file, toWrite);
            temp = temp->next;
        }
    }

    //Print remaining required line
    fprintf(file, "END:VCARD");

    //Close file
    fclose(file);

    //Return OK on successful write
    errorToReturn = OK;
    return errorToReturn;

}

VCardErrorCode validateCard(const Card* obj) {
    //Create error code to return
    VCardErrorCode errorToReturn;

    //Check if Card* object is NULL
    if (obj == NULL) {
        errorToReturn = INV_CARD;
        return errorToReturn;
    }
    if (obj->fn == NULL) {
        errorToReturn = INV_CARD;
        return errorToReturn;
    }
    if (obj->optionalProperties == NULL) {
        errorToReturn = INV_CARD;
        return errorToReturn;
    }

    //Create a list of valid properties
    //NOTE: REMEMBER TO FREE validProperties
    List* validProperties = initializeList(printValue, deleteValue, compareValues);
    populatePropertyList(validProperties);

    //Initialize counters for Property* cardinality
    //KIND MAY only appear once
    int KINDCounter = 0;
    //N MAY only appear once
    int NCounter = 0;
    //GENDER MAY only appear once
    int GENDERCounter = 0;
    //PRODID MAY only appear once
    int PRODIDCounter = 0;
    //REV MAY only appear once
    int REVCounter = 0;
    //UID MAY only appear once
    int UIDCounter = 0;

    //Begin validation
    //Verify obj->fn name/value constraints
    if (errorCheckProperty(obj->fn) == INV_PROP) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (checkPropertyValueCount(obj->fn) == INV_PROP) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (stringCaseCompare(obj->fn->name, "FN") != 0) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }

    //Verify obj->birthday
    if (obj->birthday != NULL) {
        DateTime* birthday = obj->birthday;
        if (errorCheckDateTime(birthday) == INV_DT) {
            freeList(validProperties);
            errorToReturn = INV_DT;
            return errorToReturn;
        }
    }

    //Verify obj->anniversary
    if (obj->anniversary != NULL) {
        DateTime* anniversary = obj->anniversary;
        if (errorCheckDateTime(anniversary) == INV_DT) {
            freeList(validProperties);
            errorToReturn = INV_DT;
            return errorToReturn;
        }
    }

    //Go through each optional Property* and verify the name && value count (Increment counters if name is flagged)
    if (obj->optionalProperties->length > 0) {
        Node* temp = obj->optionalProperties->head;
        while (temp) {
            Property* tempProperty = (Property*)(temp->data);

            //Validate property
            if (errorCheckProperty(tempProperty) == INV_PROP) {
                freeList(validProperties);
                errorToReturn = INV_PROP;
                return errorToReturn;
            }

            //Validate property name
            if (stringCaseCompare(tempProperty->name, "VERSION") == 0 || stringCaseCompare(tempProperty->name, "BEGIN") == 0 || stringCaseCompare(tempProperty->name, "END") == 0) {
                freeList(validProperties);
                errorToReturn = INV_CARD;
                return errorToReturn;
            }

            if (stringCaseCompare(tempProperty->name, "BDAY") == 0 || stringCaseCompare(tempProperty->name, "ANNIVERSARY") == 0) {
                freeList(validProperties);
                errorToReturn = INV_DT;
                return errorToReturn;
            }

            if (listPropertyNameSearch(validProperties, tempProperty->name) == false) {
                freeList(validProperties);
                errorToReturn = INV_PROP;
                return errorToReturn;
            }

            //Check if the name needs to be incremented for cardinality validation purposes
            if (stringCaseCompare(tempProperty->name, "KIND") == 0) {
                KINDCounter++;
            }
            if (stringCaseCompare(tempProperty->name, "N") == 0) {
                NCounter++;
            }
            if (stringCaseCompare(tempProperty->name, "GENDER") == 0) {
                GENDERCounter++;
            }
            if (stringCaseCompare(tempProperty->name, "PRODID") == 0) {
                PRODIDCounter++;
            }
            if (stringCaseCompare(tempProperty->name, "REV") == 0) {
                REVCounter++;
            }
            if (stringCaseCompare(tempProperty->name, "UID") == 0) {
                UIDCounter++;
            }

            //Validate property value count
            if (checkPropertyValueCount(tempProperty) == INV_PROP) {
                freeList(validProperties);
                errorToReturn = INV_PROP;
                return errorToReturn;
            }

            temp = temp->next;
        }
    }

    //Check all flagged counters to validate cardinality constraints
    if (KINDCounter > 1) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (NCounter > 1) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (GENDERCounter > 1) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (PRODIDCounter > 1) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (REVCounter > 1) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }
    if (UIDCounter > 1) {
        freeList(validProperties);
        errorToReturn = INV_PROP;
        return errorToReturn;
    }

    //Free list and content
    freeList(validProperties);

    //Return OK
    errorToReturn = OK;
    return errorToReturn;
}

void deleteCard(Card* obj) {
    //Check if passed in Card* is NULL
    if (obj != NULL) {
        //Free fn Property* structure (FN Property* isn't allowed to be NULL
        if (obj->fn != NULL) {
            deleteProperty(obj->fn);
        }

        //Free birthday DateTime* structure
        if (obj->birthday != NULL) {
            deleteDate(obj->birthday);
        }

        //Free anniversary DateTime* structure
        if (obj->anniversary != NULL) {
            deleteDate(obj->anniversary);
        }

        //Free optionalProperties list* data structure
        if (obj->optionalProperties != NULL) {
            if (obj->optionalProperties->length > 0) {
                freeList(obj->optionalProperties);
            }
            else {
                free(obj->optionalProperties);
            }
        }

        //Free Card* structure
        free(obj);
    }
}

char* strListToJSON(const List* strList) {
    //Check if list is NULL
    if (strList == NULL) {
        return NULL;
    }

    char* toReturn = NULL;

    if (strList->length == 0) {
        toReturn = calloc(3, sizeof(char));
	strcpy(toReturn, "[]");
	return toReturn;
    }

    //Check length of list
    if (strList->length > 0) {
        Node* current = strList->head;
        toReturn = calloc(2, sizeof(char));
        strcpy(toReturn, "[");
        while (current) {
            //Add current->data to toReturn
            int currentDataLength = 0;
            if (current->data != NULL) {
                currentDataLength = (int)(strlen(current->data));
            }

            toReturn = realloc(toReturn, strlen(toReturn) + currentDataLength + 4);
            strcat(toReturn, "\"");
            strcat(toReturn, current->data);

            if (current->next != NULL) {
                strcat(toReturn, "\",");
            }
            else {
                strcat(toReturn, "\"");
            }

            current = current->next;
        }
        toReturn = realloc(toReturn, strlen(toReturn) + 2);
        strcat(toReturn, "]");
    }

    return toReturn;
}

List* JSONtoStrList(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    //NOTE: REMEMBER TO FREE strCopyPointer
    char* strCopy = calloc(strlen(str) + 1, sizeof(char));
    strcpy(strCopy, str);
    char* strCopyPointer = strCopy;

    //Convert str into a List*
    List* toReturn = initializeList(printValue, deleteValue, compareValues);
    int numberOfValues = numberOfCharacters(strCopy, '"');
    numberOfValues = numberOfValues / 2;

    strCopy = strtok(strCopy, "[]\"");
    while (strCopy) {
        if (strcmp(strCopy, ",") != 0) {
            //Add to list
            char* toAdd = calloc(strlen(strCopy) + 1, sizeof(char));
            strcpy(toAdd, strCopy);
            insertBack(toReturn, toAdd);
        }
        strCopy = strtok(NULL, "[]\"");
    }

    if (toReturn->length < numberOfValues) {
        int currentLength = toReturn->length;
        for (int i = currentLength; i < numberOfValues; i++) {
            char* toAdd = calloc(1, sizeof(char));
            insertBack(toReturn, toAdd);
        }
    }

    free(strCopyPointer);
    return toReturn;
}

char* propToJSON(const Property* prop) {
    if (prop == NULL) {
        return NULL;
    }

    char* toReturn = NULL;
    toReturn = calloc(11, sizeof(char));
    strcpy(toReturn, "{\"group\":\"");

    if (prop->group != NULL) {
        int groupLength = (int)(strlen(prop->group));
        toReturn = realloc(toReturn, strlen(toReturn) + groupLength + 2);
        strcat(toReturn, prop->group);
        strcat(toReturn, "\"");
    }

    if (prop->name != NULL) {
        int nameLength = (int)(strlen(prop->name));
        toReturn = realloc(toReturn, strlen(toReturn) + nameLength + 11);
        strcat(toReturn, ",\"name\":\"");
        strcat(toReturn, prop->name);
        strcat(toReturn, "\"");
    }

    if (prop->values->length > 0) {
        char* returnedValues = strListToJSON(prop->values);
        int valuesLength = (int)(strlen(returnedValues));
        toReturn = realloc(toReturn, strlen(toReturn) + valuesLength + 11);
        strcat(toReturn, ",\"values\":");
        strcat(toReturn, returnedValues);
        free(returnedValues);
    }

    toReturn = realloc(toReturn, strlen(toReturn) + 2);
    strcat(toReturn, "}");

    return toReturn;
}

Property* JSONtoProp(const char* str) {
    /*
     * Algorithm:
     * strtok on {}:,
     * Check if section contains two "
     * If it does and length of string is 2, it's an empty string to add
     * Else remove all occurrences of " and check the value of section
     * Add the section to the Property*
     */

    if (str == NULL) {
        return NULL;
    }

    //NOTE: REMEMBER TO FREE strCopyPointer
    char* strCopy = calloc(strlen(str) + 1, sizeof(char));
    strcpy(strCopy, str);
    char* strCopyPointer = strCopy;

    Property* toReturn = createProperty();

    strCopy = strtok(strCopy, "{}:,");
    while (strCopy) {
        if (strcmp(strCopy, "\"group\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            //strCopy should be the group now unless empty
            if (strCopy != NULL) {
                if (numberOfCharacters(strCopy, '\"') == 2 && strlen(strCopy) == 2) {
                    //Group is empty
                    toReturn->group = calloc(1, sizeof(char));
                    strcpy(toReturn->group, "");
                }
                else {
                    removeChar(strCopy, '\"');
                    toReturn->group = calloc(strlen(strCopy) + 1, sizeof(char));
                    strcpy(toReturn->group, strCopy);
                }
            }
        }

        if (strcmp(strCopy, "\"name\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            if (strCopy != NULL) {
                //Remove all quotation marks from string
                removeChar(strCopy, '\"');
                toReturn->name = calloc(strlen(strCopy) + 1, sizeof(char));
                strcpy(toReturn->name, strCopy);
            }
        }

        if (strcmp(strCopy, "\"values\"") == 0) {
            strCopy = strtok(NULL, "[]{}:,");
            int numberOfValues = 0;

            while(strCopy) {
                //Can't strtok on , due to , being allowed in value
                removeChar(strCopy, '\"');
                if (strcmp(strCopy, ",") != 0) {
                    char* valueToAdd = calloc(strlen(strCopy) + 1, sizeof(char));
                    strcpy(valueToAdd, strCopy);
                    insertBack(toReturn->values, valueToAdd);
                    numberOfValues++;
                }
                else {
                    //Empty value to add later
                    numberOfValues++;
                }
                strCopy = strtok(NULL, "\"]}");
            }

            for (int j = toReturn->values->length; j < numberOfValues - 1; j++) {
                char* emptyValueToAdd = calloc(1, sizeof(char));
                strcpy(emptyValueToAdd, "");
                insertBack(toReturn->values, emptyValueToAdd);
            }
        }

        strCopy = strtok(NULL, "{}:,");
    }

    free(strCopyPointer);

    if (toReturn->group == NULL) {
        toReturn->group = calloc(1, sizeof(char));
        strcpy(toReturn->group, "");
    }

    return toReturn;
}

char* dtToJSON(const DateTime* prop) {
    if (prop == NULL) {
        return NULL;
    }

    char* toReturn = NULL;
    toReturn = calloc(11, sizeof(char));
    strcpy(toReturn, "{\"isText\":");

    if (prop->isText == false) {
        toReturn = realloc(toReturn, strlen(toReturn) + 8);
        strcat(toReturn, "false,\"");
    }
    else {
        toReturn = realloc(toReturn, strlen(toReturn) + 7);
        strcat(toReturn, "true,\"");
    }

    if (prop->date[0] != '\0') {
        toReturn = realloc(toReturn, strlen(toReturn) + 19);
        strcat(toReturn, "date\":\"");
        strcat(toReturn, prop->date);
        strcat(toReturn, "\",");
    }
    else {
        toReturn = realloc(toReturn, strlen(toReturn) + 10);
        strcat(toReturn, "date\":\"\",");
    }

    if (prop->time[0] != '\0') {
        toReturn = realloc(toReturn, strlen(toReturn) + 18);
        strcat(toReturn, "\"time\":\"");
        strcat(toReturn, prop->time);
        strcat(toReturn, "\",");
    }
    else {
        toReturn = realloc(toReturn, strlen(toReturn) + 11);
        strcat(toReturn, "\"time\":\"\",");
    }

    if (prop->text[0] != '\0') {
        int textLength = (int)(strlen(prop->text));
        toReturn = realloc(toReturn, strlen(toReturn) + textLength + 11);
        strcat(toReturn, "\"text\":\"");
        strcat(toReturn, prop->text);
        strcat(toReturn, "\",");
    }
    else {
        toReturn = realloc(toReturn, strlen(toReturn) + 11);
        strcat(toReturn, "\"text\":\"\",");
    }

    if (prop->UTC == false) {
        toReturn = realloc(toReturn, strlen(toReturn) + 14);
        strcat(toReturn, "\"isUTC\":false");
    }
    else {
        toReturn = realloc(toReturn, strlen(toReturn) + 13);
        strcat(toReturn, "\"isUTC\":true");
    }

    toReturn = realloc(toReturn, strlen(toReturn) + 2);
    strcat(toReturn, "}");

    return toReturn;
}

DateTime* JSONtoDT(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    //NOTE: REMEMBER TO FREE strCopyPointer
    char* strCopy = calloc(strlen(str) + 1, sizeof(char));
    strcpy(strCopy, str);
    char* strCopyPointer = strCopy;

    bool isText = false;
    bool UTC = false;
    //NOTE: REMEMBER TO FREE date, time, and text
    char* date = calloc(1, sizeof(char));
    strcpy(date, "");
    char* time = calloc(1, sizeof(char));
    strcpy(time, "");
    char* text = calloc(1, sizeof(char));
    strcpy(text, "");

    strCopy = strtok(strCopy, "{}:,");
    while (strCopy) {
        if (strcmp(strCopy, "\"isText\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            //strCopy should be the group now unless empty
            if (strCopy != NULL) {
                if (strcmp(strCopy, "true") == 0 || strcmp(strCopy, "\"true\"") == 0) {
                    isText = true;
                }
                else {
                    isText = false;
                }
            }
        }

        if (strcmp(strCopy, "\"date\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            if (strCopy != NULL) {
                //Remove all quotation marks from string
                removeChar(strCopy, '\"');
                date = realloc(date, strlen(strCopy) + 1);
                strcpy(date, strCopy);
            }
        }

        if (strcmp(strCopy, "\"time\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            if (strCopy != NULL) {
                //Remove all quotation marks from string
                removeChar(strCopy, '\"');
                time = realloc(time, strlen(strCopy) + 1);
                strcpy(time, strCopy);
            }
        }

        if (strcmp(strCopy, "\"text\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            if (strCopy != NULL) {
                //Remove all quotation marks from string
                removeChar(strCopy, '\"');
                text = realloc(text, strlen(strCopy) + 1);
                strcpy(text, strCopy);
            }
        }

        if (strcmp(strCopy, "\"isUTC\"") == 0) {
            strCopy = strtok(NULL, "{}:,");
            //strCopy should be the group now unless empty
            if (strCopy != NULL) {
                if (strcmp(strCopy, "true") == 0 || strcmp(strCopy, "\"true\"") == 0) {
                    UTC = true;
                }
                else {
                    UTC = false;
                }
            }
        }

        strCopy = strtok(NULL, "{}:,");
    }


    DateTime* toReturn = createDateTime((int)(strlen(text)) + 1);
    toReturn->UTC = UTC;
    toReturn->isText = isText;
    strcpy(toReturn->date, date);
    strcpy(toReturn->time, time);
    strcpy(toReturn->text, text);

    free(date);
    free(time);
    free(text);
    free(strCopyPointer);
    return toReturn;
}

Card* JSONtoCard(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    //NOTE: REMEMBER TO FREE strCopyPointer
    char* strCopy = calloc(strlen(str) + 1, sizeof(char));
    strcpy(strCopy, str);
    char* strCopyPointer = strCopy;

    Card* toReturn = initializeCard();
    toReturn->fn = createProperty();
    toReturn->fn->group = calloc(1, sizeof(char));

    strCopy = strtok(strCopy, "{}:\"");
    if (strCopy != NULL) {
        if (stringCaseCompare(strCopy, "FN") == 0) {
            strCopy = strtok(NULL, "{}:\"");
            if (strCopy != NULL) {
                toReturn->fn->name = calloc(3, sizeof(char));
                strcpy(toReturn->fn->name, "FN");

                char* toAddFNValue = calloc(strlen(strCopy) + 1, sizeof(char));
                strcpy(toAddFNValue, strCopy);
                insertBack(toReturn->fn->values, toAddFNValue);
            }
        }
    }

    free(strCopyPointer);
    return toReturn;
}

void addProperty(Card* card, const Property* toBeAdded) {
    if (card == NULL || toBeAdded == NULL) {
        return;
    }

    if (card->optionalProperties == NULL) {
        return;
    }

    insertBack(card->optionalProperties, (void*)(toBeAdded));
}

char* printCard(const Card* obj) {
    if (obj == NULL) {
        return NULL;
    }
    //Get fn Property* structure
    char* returnedFN = printProperty(obj->fn);
    if (returnedFN == NULL) {
        returnedFN = calloc(5, sizeof(char));
        strcpy(returnedFN, "NULL");
    }

    //Get birthday DateTime* structure
    char* returnedBirthday = printDate(obj->birthday);
    if (returnedBirthday == NULL) {
        returnedBirthday = calloc(5, sizeof(char));
        strcpy(returnedBirthday, "NULL");
    }

    //Get anniversary DateTime* structure
    char* returnedAnniversary = printDate(obj->anniversary);
    if (returnedAnniversary == NULL) {
        returnedAnniversary = calloc(5, sizeof(char));
        strcpy(returnedAnniversary, "NULL");
    }

    //Go through list and grab all optionalProperties Property* structures
    char* returnedOptionalProperties = toString(obj->optionalProperties);
    if (returnedOptionalProperties == NULL) {
        returnedAnniversary = calloc(5, sizeof(char));
        strcpy(returnedAnniversary, "NULL");
    }

    int lengthToAllocate = (int)(strlen(returnedFN)) + (int)(strlen(returnedBirthday)) + (int)(strlen(returnedAnniversary)) + (int)(strlen(returnedOptionalProperties)) + 1;

    //Copy/Concatenate strings to a returnable string
    char* toReturn = calloc(lengthToAllocate + 1, sizeof(char));
    strcpy(toReturn, returnedFN);
    strcat(toReturn, returnedBirthday);
    strcat(toReturn, returnedAnniversary);
    strcat(toReturn, returnedOptionalProperties);

    //Free string allocations that are no longer needed
    free(returnedFN);
    free(returnedBirthday);
    free(returnedAnniversary);
    free(returnedOptionalProperties);

    //Return allocated printCard string
    return toReturn;
}

char* printError(VCardErrorCode err) {
    char* toReturn = NULL;
    switch(err) {
        case 0:
            toReturn = calloc(3, sizeof(char));
            strcpy(toReturn, "OK");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        case 1:
            toReturn = calloc(9, sizeof(char));
            strcpy(toReturn, "INV_FILE");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        case 2:
            toReturn = calloc(9, sizeof(char));
            strcpy(toReturn, "INV_CARD");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        case 3:
            toReturn = calloc(9, sizeof(char));
            strcpy(toReturn, "INV_PROP");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        case 4:
            toReturn = calloc(7, sizeof(char));
            strcpy(toReturn, "INV_DT");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        case 5:
            toReturn = calloc(12, sizeof(char));
            strcpy(toReturn, "WRITE_ERROR");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        case 6:
            toReturn = calloc(12, sizeof(char));
            strcpy(toReturn, "OTHER_ERROR");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
        default:
            toReturn = calloc(7, sizeof(char));
            strcpy(toReturn, "FAILED");
            return toReturn;
            //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
    }
}

void deleteProperty(void* toBeDeleted) {
    //Check if passed in void* is NULL
    if (toBeDeleted != NULL) {
        //Typecast toBeDeleted into Property structure
        Property* property = (Property*)(toBeDeleted);
        //Free inner structure strings
        free(property->name);
        free(property->group);
        //Free inner structure list nodes
        freeList(property->parameters);

        freeList(property->values);
        //Free Property structure
        free(property);
    }
}

int compareProperties(const void* first, const void* second) {
    if (first == NULL && second != NULL) {
        return 1;
    }
    if (first != NULL && second == NULL) {
        return -1;
    }
    if (first == NULL && second == NULL) {
        return 0;
    }

    Property* firstProperty = (Property*)(first);
    Property* secondProperty = (Property*)(second);

    char* firstPropertyString = printProperty(firstProperty);
    char* secondPropertyString = printProperty(secondProperty);

    if (strcmp(firstPropertyString, secondPropertyString) == 0) {
        free(firstPropertyString);
        free(secondPropertyString);
        return 0;
    }
    else if (strcmp(firstPropertyString, secondPropertyString) > 0) {
        free(firstPropertyString);
        free(secondPropertyString);
        return 1;
    }
    else {
        free(firstPropertyString);
        free(secondPropertyString);
        return -1;
    }
}

char* printProperty(void* toBePrinted) {
    //Check if passed in void* is NULL
    if (toBePrinted == NULL) {
        return NULL;
    }

    //Typecast toBePrinted into Property* structure
    Property* property = (Property*)(toBePrinted);
    char* name = calloc(1, sizeof(char));
    strcpy(name, "");
    char* group = calloc(1, sizeof(char));
    strcpy(group, "");
    char* parameters = calloc(1, sizeof(char));
    strcpy(parameters, "");
    char* values = calloc(1, sizeof(char));
    strcpy(values, "");

    if (property->name != NULL) {
        int nameLength = (int)(strlen(property->name)) + 1;
        name = realloc(name, nameLength);
        strcpy(name, property->name);
    }

    if (property->group != NULL) {
        int groupLength = (int)(strlen(property->group)) + 1;
        group = realloc(group, groupLength);
        strcpy(group, property->group);
    }

    if (property->parameters->length > 0) {
        char* pList = toString(property->parameters);
        int parametersLength = (int)(strlen(pList)) + 1;
        parameters = realloc(parameters, parametersLength);
        strcpy(parameters, pList);
        free(pList);
    }

    if (property->values->length > 0) {
        char* vList = toString(property->values);
        int valuesLength = (int)(strlen(vList)) + 1;
        values = realloc(values, valuesLength);
        strcpy(values, vList);
        free(vList);
    }

    int lengthToAllocate = (int)(strlen(name)) + (int)(strlen(group)) + (int)(strlen(parameters)) + (int)(strlen(values)) + 1;

    char* toReturn = calloc(lengthToAllocate + 1, sizeof(char));
    strcpy(toReturn, name);
    strcat(toReturn, group);
    strcat(toReturn, parameters);
    strcat(toReturn, values);

    free(name);
    free(group);
    free(parameters);
    free(values);

    return toReturn;
}

void deleteParameter(void* toBeDeleted) {
    //Check if passed in void* is NULL
    if (toBeDeleted != NULL) {
        //Typecast toBeDeleted into Parameter structure
        Parameter* parameter = (Parameter*)(toBeDeleted);
        //Set inner structure static strings to \0
        parameter->name[0] = '\0';
        parameter->value[0] = '\0';
        //Free Parameter structure
        free(parameter);
    }
}

int compareParameters(const void* first, const void* second) {
    if (first == NULL && second != NULL) {
        return 1;
    }
    if (first != NULL && second == NULL) {
        return -1;
    }
    if (first == NULL && second == NULL) {
        return 0;
    }

    Parameter* firstParameter = (Parameter*)(first);
    Parameter* secondParameter = (Parameter*)(second);

    char* firstParameterString = printParameter(firstParameter);
    char* secondParameterString = printParameter(secondParameter);

    if (strcmp(firstParameterString, secondParameterString) == 0) {
        free(firstParameterString);
        free(secondParameterString);
        return 0;
    }
    else if (strcmp(firstParameterString, secondParameterString) > 0) {
        free(firstParameterString);
        free(secondParameterString);
        return 1;
    }
    else {
        free(firstParameterString);
        free(secondParameterString);
        return -1;
    }
}

char* printParameter(void* toBePrinted) {
    //Check if passed in void* is NULL
    if (toBePrinted == NULL) {
        return NULL;
    }

    //Typecast toBePrinted into Parameter* structure
    Parameter* parameter = (Parameter*)(toBePrinted);
    char* name = calloc(1, sizeof(char));
    strcpy(name, "");
    char* value = calloc(1, sizeof(char));
    strcpy(value, "");

    if (parameter->name[0] != '\0') {
        int nameLength = (int)(strlen(parameter->name)) + 1;
        name = realloc(name, nameLength);
        strcpy(name, parameter->name);
    }

    if (parameter->value[0] != '\0') {
        int valueLength = (int)(strlen(parameter->value)) + 1;
        value = realloc(value, valueLength);
        strcpy(value, parameter->value);
    }

    int lengthToAllocate = (int)(strlen(name)) + (int)(strlen(value)) + 1;

    char* toReturn = calloc(lengthToAllocate + 1, sizeof(char));
    strcpy(toReturn, name);
    strcat(toReturn, value);

    free(name);
    free(value);

    return toReturn;
}

void deleteValue(void* toBeDeleted) {
    if (toBeDeleted != NULL) {
        free(toBeDeleted);
    }
}

int compareValues(const void* first, const void* second) {
    if (first == NULL && second != NULL) {
        return 1;
    }
    if (first != NULL && second == NULL) {
        return -1;
    }
    if (first == NULL && second == NULL) {
        return 0;
    }

    char* firstString = (char*)(first);
    char* secondString = (char*)(second);

    if (strcmp(firstString, secondString) == 0) {
        return 0;
    }
    else if (strcmp(firstString, secondString) > 0) {
        return 1;
    }
    else {
        return -1;
    }
}

char* printValue(void* toBePrinted) {
    if (toBePrinted != NULL) {
        char* print = (char*)(toBePrinted);
        char* toReturn = calloc(strlen(print) + 1, sizeof(char));
        strcpy(toReturn, print);
        return toReturn;
    }

    return NULL;
}

void deleteDate(void* toBeDeleted) {
    //Check if passed in void* is NULL
    if (toBeDeleted != NULL) {
        //Typecast toBeDeleted into DateTime structure
        DateTime* dateTime = (DateTime*)(toBeDeleted);
        //Set inner structure static strings to \0
        dateTime->date[0] = '\0';
        dateTime->time[0] = '\0';
        dateTime->text[0] = '\0';
        //Free DateTime structure
        free(dateTime);
    }
}

int compareDates(const void* first, const void* second) {
    if (first == NULL && second != NULL) {
        return 1;
    }
    if (first != NULL && second == NULL) {
        return -1;
    }
    if (first == NULL && second == NULL) {
        return 0;
    }

    DateTime* firstDate = (DateTime*)(first);
    DateTime* secondDate = (DateTime*)(second);

    char* firstDateString = printDate(firstDate);
    char* secondDateString = printDate(secondDate);

    if (strcmp(firstDateString, secondDateString) == 0) {
        free(firstDateString);
        free(secondDateString);
        return 0;
    }
    else if (strcmp(firstDateString, secondDateString) > 0) {
        free(firstDateString);
        free(secondDateString);
        return 1;
    }
    else {
        free(firstDateString);
        free(secondDateString);
        return -1;
    }
}

char* printDate(void* toBePrinted) {
    //Check if passed in void* is NULL
    if (toBePrinted == NULL) {
        return NULL;
    }

    //Typecast toBePrinted into DateTime* structure
    DateTime* dateTime = (DateTime*)(toBePrinted);
    char* UTC = NULL;
    char* isText = NULL;
    char* date = calloc(1, sizeof(char));
    strcpy(date, "");
    char* time = calloc(1, sizeof(char));
    strcpy(time, "");
    char* text = calloc(1, sizeof(char));
    strcpy(text, "");

    if (dateTime->UTC == true) {
        UTC = calloc(10, sizeof(char));
        strcpy(UTC, "UTC: True");
    }
    else {
        UTC = calloc(11, sizeof(char));
        strcpy(UTC, "UTC: False");
    }

    if (dateTime->isText == true) {
        isText = calloc(13, sizeof(char));
        strcpy(isText, "isText: True");
    }
    else {
        isText = calloc(14, sizeof(char));
        strcpy(isText, "isText: False");
    }

    if (dateTime->date[0] != '\0') {
        int dateLength = (int)(strlen(dateTime->date)) + 1;
        date = realloc(date, dateLength);
        strcpy(date, dateTime->date);
    }

    if (dateTime->time[0] != '\0') {
        int timeLength = (int)(strlen(dateTime->time)) + 1;
        time = realloc(time, timeLength);
        strcpy(time, dateTime->time);
    }

    if (dateTime->text[0] != '\0') {
        int textLength = (int)(strlen(dateTime->text)) + 1;
        text = realloc(text, textLength);
        strcpy(text, dateTime->text);
    }


    int lengthToAllocate = (int)(strlen(UTC)) + (int)(strlen(isText)) + (int)(strlen(date)) + (int)(strlen(time)) + (int)(strlen(text)) + 1;

    char* toReturn = calloc(lengthToAllocate + 1, sizeof(char));
    strcpy(toReturn, UTC);
    strcat(toReturn, isText);
    strcat(toReturn, date);
    strcat(toReturn, time);
    strcat(toReturn, text);

    free(UTC);
    free(isText);
    free(date);
    free(time);
    free(text);

    //NOTE: REMEMBER TO FREE toReturn IN CALLING FUNCTION
    return toReturn;
}
