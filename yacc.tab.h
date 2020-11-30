/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_YACC_TAB_H_INCLUDED
# define YY_YY_YACC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    IDENTIFIER = 258,
    OPERATOR = 259,
    VALUE_INT = 260,
    VALUE_STRING = 261,
    DATABASE = 262,
    DATABASES = 263,
    TABLE = 264,
    SHOW = 265,
    CREATE = 266,
    DROP = 267,
    USE = 268,
    PRIMARY = 269,
    KEY = 270,
    NOT = 271,
    NULLTOKEN = 272,
    INSERT = 273,
    INTO = 274,
    VALUES = 275,
    DELETE = 276,
    FROM = 277,
    WHERE = 278,
    UPDATE = 279,
    SET = 280,
    SELECT = 281,
    IS = 282,
    INTTOKEN = 283,
    VARCHARTOKEN = 284,
    DEFAULT = 285,
    CONSTRAINT = 286,
    CHANGE = 287,
    ALTER = 288,
    ADD = 289,
    RENAME = 290,
    DESC = 291,
    REFERENCES = 292,
    INDEX = 293,
    AND = 294,
    FLOATTOKEN = 295,
    FOREIGN = 296,
    CHARTOKEN = 297,
    DATETOKEN = 298,
    TABLES = 299
  };
#endif

/* Value type.  */


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_YACC_TAB_H_INCLUDED  */
