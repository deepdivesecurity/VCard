/**
 * @file HelperFunctions.h
 * @brief This file contains the VCard file's helper function definitions.
 * @author ADD LATER
 */

#ifndef ASSIGNMENT_1_HELPERFUNCTIONS_H
#define ASSIGNMENT_1_HELPERFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "LinkedListAPI.h"
#include "VCardParser.h"

/**
 * readFileToString: Takes in a file destination and converts the contents into one char*
 * @param fileName: Destination/Name of file to be converted
 * @return char*: A dynamically created string of the file content
 */
char* readFileToString(char* fileName);

/**
 * split: Takes in a string, a length (Number of lines), and delimiters to tokenize the string and place into a 2d array
 * @param stringToSplit: String that will be tokenized
 * @param length: Number of lines added to the 2d array
 * @param delimiters: Delimiters used to break the line apart
 * @return char**: A 2d string array containing the contents of the tokenized string
 */
char** split(char* stringToSplit, int* length, char* delimiters, bool whiteSpaceCheck);

/**
 * verifyFileName: Verifies the fileName passed in is not NULL, and contains the correct extension (.vcf/.vcard)
 * @param fileName: Destination/Name of file to be verified
 * @return bool: True on good file/file extension, false on bad file/file extension
 */
bool verifyFileName(char* fileName);

/**
 * initializeCard: Card* constructor
 * @return Card*: An initialized Card* structure
 */
Card* initializeCard();

/**
 * createDateTime: DateTime* structure constructor
 * @return DateTime*: An initialized DateTime* structure
 */
DateTime* createDateTime(int flexSize);

/**
 * createParameter: Parameter* structure constructor
 * @return Parameter*: An initialized Parameter* structure
 */
Parameter* createParameter(int flexSize);

/**
 * createProperty: Property* structure constructor
 * @return Property*: An initialized Property* structure
 */
Property* createProperty();

/**
 * deepCopyDateTime: Creates a deep copy of a DateTime* structure
 * @param toCopy: DateTime* structure to be copied
 * @return DateTime*: A deep copy of a DateTime* structure
 */
//DateTime* deepCopyDateTime(DateTime* toCopy);

/**
 * deepCopyParameter: Creates a deep copy of a Parameter* structure
 * @param toCopy: Parameter* structure to be copied
 * @return Parameter*: A deep copy of a Parameter* structure
 */
//Parameter* deepCopyParameter(Parameter* toCopy);

/**
 * deepCopyProperty: Creates a deep copy of a Property* structure
 * @param toCopy: Property* structure to be copied
 * @return Property*: A deep copy of a Property* structure
 */
//Property* deepCopyProperty(Property* toCopy);

/**
 * stringUpper: Converts a string into all uppercase characters
 * @param string: char* String to be converted
 */
void stringUpper(char* string);

/**
 * numberOfCharacters: Returns the frequency of a specified character found in a string
 * @param string: char* String to search
 * @param toSearch: char Character to look for
 * @return int: Frequency of toSearch in string
 */
int numberOfCharacters(char* string, char toSearch);

/**
 * verifyStringEndOfLines: Verifies that every \r is followed by a \n
 * @param string: char* String to search
 * @return bool: True on valid string, False on invalid string
 */
bool verifyStringEndOfLines(char* string);

/**
 * removeBackslash: Removes all backslashes before commas in a quotation enclosed section
 * Function also swaps all semi-colons inside quotes with a '^' character
 * @param string: char* String to remove backslashes from
 */
void removeBackslash(char* string);

/**
 * stringCaseCompare: strcasecmp implementation for portable C
 * @param string1: char* first string to be compared
 * @param string2: char* second string to be compared
 * @return int: 0 on equal
 */
int stringCaseCompare(const char* string1, const char* string2);

/**
 * noParamOneValue: Handles the parsing on data lines with no parameters, but one value
 * @param mainCard: Card* structure to store property into
 * @param toStoreIn: Property* structure to store data into
 * @param sectionOfLine: char* section of the data line to be parsed
 * @param values: char* values of the data line to be parsed
 * @param groupSet: bool indicating if the data line has a group
 */
void noParamOneValue(Card* mainCard, Property* toStoreIn, char* sectionOfLine, char* values, bool groupSet);

/**
 * noParamMultipleValues: Handles the parsing on data lines with no parameters, but multiple values
 * @param mainCard: Card* structure to store property into
 * @param toStoreIn: Property* structure to store data into
 * @param sectionOfLine: char* section of the data line to be parsed
 * @param values: char* values of the data line to be parsed
 * @param groupSet: bool indicating if the data line has a group
 */
void noParamMultipleValues(Card* mainCard, Property* toStoreIn, char* sectionOfLine, char* values, bool groupSet);

/**
 * params: Handles the parsing on data lines with multiple parameters and possibly multiple values
 * @param mainCard: Card* structure to store property into
 * @param toStoreIn: Property* structure to store data into
 * @param parameters: List* of parameters to parse
 * @param sectionOfLine: char* section of the data line to be parsed
 * @param values: char* values of the data line to be parsed
 * @param groupSet: bool indicating if the data line has a group
 */
void params(Card* mainCard, Property* toStoreIn, List* parameters, char* sectionOfLine, char* values, bool groupSet);

/**
 * addParametersToProperty: Handles adding parameters to a property
 * @param toStoreIn: Property* to store parameters into
 * @param parameters: List* list of parameters
 */
VCardErrorCode addParametersToProperty(Property* toStoreIn, List* parameters);

/**
 * parseDate: Creates a DateTime* structure based on a Property* structure
 * @param currentProperty: Property* to be conv
 * @return DateTime*: A newly created DateTime* structure
 */
DateTime* parseDate(Property* currentProperty);

/**
 * groupCheck: Looks for a group in the line
 * @param line: char* Line to be parsed for a group
 * @return char*: If a group is found, returns the group, otherwise, returns NULL
 */
char* groupCheck(char* line);

/**
 * propertyCheck: Checks if a property is NULL or empty
 * @param line: char* Line to check property for
 * @return VCardErrorCode: OK on valid property, INV_PROP otherwise
 */
VCardErrorCode propertyCheck(char* line);

/**
 * verifyOnlyOnce: Makes sure that BEGIN/END/VERSION only happens once in the card
 * @param line: char* Line to be verified
 * @return bool: false on bad line, true on good line
 */
bool verifyOnlyOnce(char* line);

/**
 * validateDateTime: Performs validations on a DateTime* structure
 * @param currentDateTime: DateTime* to be verified
 * @return bool: false on bad DateTime, true on good DateTime
 */
bool validateDateTime(DateTime* currentDateTime);

/**
 * writeProperty: Handles the writing to file for Property* structures
 * @param file: FILE* to write to
 * @param toWrite: Property* to be written
 */
void writeProperty(FILE* file, Property* toWrite);

/**
 * writeDateTime: Handles the writing to file for DateTime* properties
 * @param file: FILE* to write to
 * @param toWrite: DateTime* to be written
 * @param isAnniversary: bool true on Anniversary DateTime*, false otherwise
 * @param isBDay: bool true on BDay DateTime*, false otherwise
 */
void writeDateTime(FILE* file, DateTime* toWrite, bool isAnniversary, bool isBDay);

/**
 * populatePropertyList: Populates a list with all valid Property* structure names
 * @param toPopulate: List* to populate
 */
void populatePropertyList(List* toPopulate);

/**
 * errorCheckProperty: Handles the validation checks of a Property* structure
 * @param toCheck: Property* to validate
 * @return VCardErrorCode: OK on valid Property*, INV_PROP otherwise
 */
VCardErrorCode errorCheckProperty(Property* toCheck);

/**
 * errorCheckDateTime: Handles the validation checks of a DateTime* structure
 * @param toCheck: DateTime* to validate
 * @return VCardErrorCode: OK on valid DateTime*, INV_DT otherwise
 */
VCardErrorCode errorCheckDateTime(DateTime* toCheck);

/**
 * checkPropertyValueCount: Handles the validation check of a Property* value count
 * @param toCheck: Property* to validate
 * @return VCardErrorCode: OK on valid Property* count, INV_PROP otherwise
 */
VCardErrorCode checkPropertyValueCount(Property* toCheck);

/**
 * listPropertyNameSearch: String search function for a List* - specifically designed for Property* name validation
 * @param validPropertyList: List* of valid property names
 * @param propertyName: char* Current name to search for
 * @return bool: true on found string, false otherwise
 */
bool listPropertyNameSearch(List* validPropertyList, char* propertyName);

/**
 * substring: Grabs a substring inside of a string given a startIndex
 * @param startIndex: int Index to start collecting characters from
 * @param str: char* To grab substring from
 * @return char*: Substring
 */
char* substring(int startIndex, char* str);

/**
 * removeChar: Removes all occurrences of a character from a string
 * @param stringToCheck: char* To remove character from
 * @param characterToRemove: char To remove from the string
 */
void removeChar(char* stringToCheck, char characterToRemove);

/**
 * getFileLog: Retrieves the Individual's Name & Additional Properties count in JSON format
 * @param fileName: char* File to retrieve data from
 * @return char*: JSON formatted string of the individual's name and additional property count
 */
char* getFileLog(char* fileName);

char* getCardView(char* fileName);

#endif //ASSIGNMENT_1_HELPERFUNCTIONS_H
