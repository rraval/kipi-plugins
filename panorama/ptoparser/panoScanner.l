/*============================================================
 * 
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Description : Hugin pto file lexer
 *
 * Copyright (C) 2007 Daniel M German <dmgerman at uvic doooot ca>
 * Copyright (C) 2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================ */

%option noyywrap
%option nounput
%option noinput

%{

/* need this for the call to strtold() below */
#include <math.h>
#include <string.h>

#include "tparserdebug.h"
#include "tparserprivate.h"
#include "panoParser.h"

char strBuffer[PT_TOKEN_MAX_LEN + 1];
char *strBuffer_ptr;
int first_eof;

#define YY_INPUT(buf,result,max_size)  {\
    result = panoScriptScannerGetNextChar(buf, max_size); \
    if (  result <= 0  ) {\
      result = YY_NULL; \
    }\
    }

%}

DIGIT    [0-9]
LINETYPE [a-zA-Z]
ID       [a-zA-Z]

%x str
%x parm
%x hugin
%x mask
%x maskcoordinate
%x projparms


%%

  /* Simple rules for all purpose */

^[ \t]*\r?\n { /* Ignore empty lines */
    DEBUG_1("Empty line");
}

    /* Simple rules to read numbers */


<INITIAL,parm,maskcoordinate,projparms>[-+]?{DIGIT}+"."{DIGIT}*(e-?{DIGIT}+)? { /* float point with period */
    BEGIN(INITIAL);
    panoScriptScannerTokenBegin(yytext);
    yylval.fVal = strtod(yytext, NULL);
    DEBUG_3("A  float: %s (%lg)", yytext, yylval.fVal);
    return PT_TOKEN_NUMBER_FLOAT;
}

<INITIAL,parm,maskcoordinate,projparms>[-+]?{DIGIT}+ { /* Simple integer */
    BEGIN(INITIAL);
    panoScriptScannerTokenBegin(yytext);
    yylval.iVal = atoi(yytext);
    DEBUG_3("An integer: %s (%d)", yytext, yylval.iVal);
    return PT_TOKEN_NUMBER_INT;
}

[ \t]*\r?\n { /* END OF LINE is an important marker when processing a rule */
    DEBUG_1("End of line");
    panoScriptScannerTokenBegin(yytext);
    first_eof = 0;
    return PT_TOKEN_EOL;
}

<*><<EOF>> { /* We are done */
    DEBUG_1("End of file");
    panoScriptScannerTokenBegin(yytext);
    if (first_eof) {
        first_eof = 0;
        return PT_TOKEN_EOL;
    }
    return 0;
}

<maskcoordinate,projparms>" "+ {
    panoScriptScannerTokenBegin(yytext); 
    return PT_TOKEN_COMMA;
}

<*>[ \t]+ {  /* token separator */
    panoScriptScannerTokenBegin(yytext); 
    return PT_TOKEN_SEP;
}


  /******************************************/
  /* These are the types of lines supported */
  /******************************************/

^p {    /* panorama line */
    DEBUG_1("Panorama line");
    panoScriptScannerTokenBegin(yytext);
    first_eof = 1;
    return PT_TOKEN_PANO_LINE;
}

^[io] {    /* input line */
    panoScriptScannerTokenBegin(yytext);
    first_eof = 1;
    return PT_TOKEN_INPUT_LINE;
}

^m {    /* Optimization options line */
    panoScriptScannerTokenBegin(yytext);
    first_eof = 1;
    return PT_TOKEN_OPTIMIZE_OPT_LINE;
}

^v {    /* Var optimization line */
    panoScriptScannerTokenBegin(yytext);
    first_eof = 1;
    return PT_TOKEN_OPTIMIZE_VARS_LINE;
}

^k {    /* Mask line */
    panoScriptScannerTokenBegin(yytext);
    first_eof = 1;
    BEGIN(mask);
    return PT_TOKEN_MASK_PT_LINE;
}

^c {    /* Control point line */
    panoScriptScannerTokenBegin(yytext);
    first_eof = 1;
    return PT_TOKEN_CONTROL_PT_LINE;
}

^#[^\n]* {
    DEBUG_2("Comment "%s"", yytext);
    panoScriptScannerTokenBegin(yytext);
    strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
    yylval.strVal[PT_TOKEN_MAX_LEN] = 0;
    first_eof = 1;
    return PT_TOKEN_COMMENT;
}


    /******************************************/
    /* Main tokens in different libpano lines */
    /******************************************/
<parm>"," {
    panoScriptScannerTokenBegin(yytext);
    BEGIN(parm);
    return PT_TOKEN_COMMA;
}

<parm>= {   /* Reference to a variable */
    panoScriptScannerTokenBegin(yytext);
    return PT_TOKEN_REFERENCE;
}


<parm>a-zA-Z[0-9a-zA-Z]+ {
    /* Read the rest of the parameter as a string by default */
    /* move back to default mode */
    BEGIN(INITIAL);
    panoScriptScannerTokenBegin(yytext);
    strncat(strBuffer, yytext, PT_TOKEN_MAX_LEN - 1);
    strncpy(yylval.strVal, strBuffer, PT_TOKEN_MAX_LEN);
    yylval.strVal[PT_TOKEN_MAX_LEN] = 0;
    return PT_TOKEN_STRING;
}

<mask>[it] {
    DEBUG_2("An identifier for a variable: %s", yytext);
    /* move into parameter mode */
    BEGIN(parm);
    panoScriptScannerTokenBegin(yytext);
    yylval.cVal = yytext[0];
    return PT_TOKEN_KEYWORD;
}

<mask>p\" {
    DEBUG_2("An identifier for a variable: %s", yytext);
    /* move into parameter mode */
    BEGIN(maskcoordinate);
    panoScriptScannerTokenBegin(yytext);
    yylval.cVal = yytext[0];
    return PT_TOKEN_KEYWORD_MASK;
}

<maskcoordinate>\" {
    panoScriptScannerTokenBegin(yytext);
    BEGIN(mask);
}

Tr[XYZ]|Eev|E[rb]|V[mfabcdxy]|R[abcde] {
    DEBUG_2("An identifier for a variable: %s", yytext);
    panoScriptScannerTokenBegin(yytext);
    strncpy(yylval.strVal, yytext, PT_TOKEN_MAX_LEN);
    yylval.strVal[PT_TOKEN_MAX_LEN] = 0;
    BEGIN(parm);
    return PT_TOKEN_KEYWORD_MULTICHAR;
}

[CS] {  /* These are the C and S parameters that contain multiple values */
    DEBUG_2("An identifier for a variable: %s", yytext);
    BEGIN(parm);
    yylval.cVal = yytext[0];
    return PT_TOKEN_KEYWORD_CROPPING;
}

P\" {
    DEBUG_2("An identifier for a variable: %s", yytext);
    /* move into parameter mode */
    BEGIN(projparms);
    panoScriptScannerTokenBegin(yytext);
    yylval.cVal = yytext[0];
    return PT_TOKEN_KEYWORD_PROJPARAMS;
}

{ID} {
    DEBUG_2("An identifier for a variable: %s", yytext);
    /* move into parameter mode */
    BEGIN(parm);
    panoScriptScannerTokenBegin(yytext);
    yylval.cVal = yytext[0];
    return PT_TOKEN_KEYWORD;
}

  /*********************************/
   /* From this on process strings */
  /*********************************/

<projparms>\" {
    BEGIN(INITIAL);
}

<parm>\" { /* Begin of string */
    panoScriptScannerTokenBegin(yytext);
    strBuffer_ptr = strBuffer;
    BEGIN(str);
}

<str>\" { /* saw closing quote - all done */
    BEGIN(INITIAL);
    *strBuffer_ptr = '\0';
    /* return string constant token type and value to parser */
    strncpy(yylval.strVal, strBuffer, PT_TOKEN_MAX_LEN);
    yylval.strVal[PT_TOKEN_MAX_LEN] = 0;
    return PT_TOKEN_STRING;
}

<str>\n {
    /* error - unterminated string constant */
    /* generate error message */
    panoScriptParserError("non-terminated string: %s", yytext);
    return -0;
}

<str>\\n  *strBuffer_ptr++ = '\n';
<str>\\t  *strBuffer_ptr++ = '\t';
<str>\\r  *strBuffer_ptr++ = '\r';
<str>\\b  *strBuffer_ptr++ = '\b';
<str>\\f  *strBuffer_ptr++ = '\f';

<str>\\(.|\n)  *strBuffer_ptr++ = yytext[1];

<str>[^\\\n\"]+ {
    int currentLen = strlen(strBuffer);
    char *yptr = yytext;
    while (*yptr && ++currentLen < PT_TOKEN_MAX_LEN)
        *strBuffer_ptr++ = *yptr++;
    panoScriptScannerTokenBegin(yytext);
}




  /********************************/
  /* Catch errors */
. {
    panoScriptParserError("Unrecognized character: [%s]", yytext);
    return PT_TOKEN_ERROR;
}



%%
