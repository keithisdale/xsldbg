
/**************************************************************************
                          utils.c  -  misc utils

                             -------------------
    begin                : Thur Jan 31 2002
    copyright            : (C) 2001 by Keith Isdale
    email                : k_isdale@tpg.com.au
 **************************************************************************/

/**************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/

#include "utils.h"

/**
 * trimString:
 * @text : A valid string with leading or trailing spaces
 *
 * Remove leading and trailing spaces off @text
 *         stores result back into @text
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
trimString(xmlChar * text)
{
    int result = 0;
    xmlChar *start, *end;

    if (text) {
        start = text;
        end = text + xmlStrLen(text) - 1;
        while (IS_BLANK(*start) && (start <= end))
            start++;

        while (IS_BLANK(*end) && (end >= start))
            end--;

        /* copy  to @text */
        while (start <= end) {
            *text = *start;
            text++;
            start++;
        }

        *text = '\0';
        result = 1;
    }
    return result;
}


/**
 * splitString:
 * @textIn: The string to split
 * @maxStrings: The max number of strings to put into @out
 * @out: Is valid and at least the size of @maxStrings
 *
 * Split string by white space and put into @out
 *
 * Returns 1 on success,
 *         0 otherwise
 */
int
splitString(xmlChar * textIn, int maxStrings, xmlChar ** out)
{
    int wordCount = 0;
    int foundQuote = 0;

    while ((*textIn != '\0') && (wordCount < maxStrings)) {
        /*skip the first spaces ? */
        while (IS_BLANK(*textIn))
            textIn++;

        if (*textIn == '\"') {
            textIn++;
            foundQuote = 1;
        }
        out[wordCount] = textIn;

        /* look for end of word */
        if (foundQuote == 0) {
            while (!IS_BLANK(*textIn) && (*textIn != '\0'))
                textIn++;

            if (*textIn != '\0') {
                *textIn = '\0';
                textIn++;
            }

            if (xmlStrLen(out[wordCount]) > 0) {
                wordCount++;
            }
        } else {
            /* look for ending quotation mark */
            while ((*textIn != '\0') && (*textIn != '\"'))
                textIn++;
            if (*textIn == '\0') {
                xsltGenericError(xsltGenericErrorContext,
                                 "Error: Unmatched quotes in input\n");
		wordCount = 0;
		break;
            }
            *textIn = '\0';
            textIn++;           /* skip the '"' which is now a '\0' */
            foundQuote = 0;
            wordCount++;
        }

    }

    if (*textIn != '\0')
        wordCount = 0;             /* We have not processed all the text givent to us */
    return wordCount;
}
