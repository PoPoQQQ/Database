%{
/*本lex的生成文件是lex.yy.c
lex文件由3段组成，用2个%%行把这3段隔开。
第1段是声明段，包括：
1-C代码部分：include头文件、函数、类型等声明，这些声明会原样拷到生成的.c文件中。
2-状态声明，如%x COMMENT。
3-正则式定义，如digit ([0-9])。
第2段是规则段，是lex文件的主体，包括每个规则(如identifier)是如何匹配的，以及匹配后要执行的C代码动作。
第3段是C函数定义段，如yywrap()的定义，这些C代码会原样拷到生成的.c文件中。该段内容可以为空*/

//第1段：声明段
#include "parser.h"		//lex和yacc要共用的头文件，里面包含了一些头文件，重定义了YYSTYPE
#include "yacc.tab.h"	//用yacc编译yacc.y后生成的C头文件，内含%token、YYSTYPE、yylval等定义(都是C宏)，供lex.yy.c和yacc.tab.c使用

extern "C"				//为了能够在C++程序里面调用C函数，必须把每一个需要使用的C函数，其声明都包括在extern "C"{}块里面，这样C++链接时才能成功链接它们。extern "C"用来在C++环境下设置C链接类型。
{						//yacc.y中也有类似的这段extern "C"，可以把它们合并成一段，放到共同的头文件main.h中
	int yywrap(void);
	int yylex(void);	//这个是lex生成的词法分析函数，yacc的yyparse()里会调用它，如果这里不声明，生成的yacc.tab.c在编译时会找不到该函数
}
%}
/* %option case-insensitive 可以用这个语句来使词法解析完全不匹配大小写，不过我担心这可能会出现一些问题，于是先老老实实一个个手写关键词吧
*/
/*lex的每个正则式前面可以带有"<状态>"，例如下面的"<COMMENT>\n"。每个状态要先用%x声明才能使用。
当lex开始运行时，默认状态是INITIAL，以后可在C代码里用"BEGIN 状态名;"切换到其它状态(BEGIN是lex/yacc内置的宏)。
这时，只有当lex状态切换到COMMENT后，才会去匹配以<COMMENT>开头的正则式，而不匹配其它状态开头的。
也就是说，lex当前处在什么状态，就考虑以该状态开头的正则式，而忽略其它的正则式。
其应用例如，在一段C代码里，同样是串"abc"，如果它写在代码段里，会被识别为标识符，如果写在注释里则就不会。所以对串"abc"的识别结果，应根据不同的状态加以区分。
本例子需要忽略掉文本中的行末注释，行末注释的定义是：从某个"//"开始，直到行尾的内容都是注释。其实现方法是：
1-lex启动时默认是INITIAL状态，在这个状态下，串"abc"会识别为标识符，串"123"会识别为整数等。
2-一旦识别到"//"，则用BEGIN宏切换到COMMENT状态，在该状态下，abc这样的串、以及其它字符会被忽略。只有识别到换行符\n时，再用BEGIN宏切换到初始态，继续识别其它记号。*/
%x	SINGLELINE_COMMENT 
%x 	MULTILINE_COMMENT
%x	S 
/* 解析 String */

/* 一个或一段连续的空白符 */
WHITESPACE	([ \t]+)

/* keywords */
DATABASE	(?i:DATABASE)
DATABASES	(?i:DATABASES)
TABLE		(?i:TABLE)
TABLES		(?i:TABLES)
SHOW		(?i:SHOW)
CREATE		(?i:CREATE)
DROP		(?i:DROP)
USE			(?i:USE)
PRIMARY		(?i:PRIMARY)
KEY			(?i:KEY)
NOT			(?i:NOT)
NULL		(?i:NULL)
INSERT		(?i:INSERT)
INTO		(?i:INTO)
VALUES		(?i:VALUES)
DELETE		(?i:DELETE)
FROM		(?i:FROM)
WHERE		(?i:WHERE)
UPDATE		(?i:UPDATE)
SET			(?i:SET)
SELECT		(?i:SELECT)	
IS			(?i:IS)	
INT			(?i:INT)	
VARCHAR		(?i:VARCHAR)
DEFAULT		(?i:DEFAULT)
CONSTRAINT	(?i:CONSTRAINT)
CHANGE		(?i:CHANGE)
ALTER		(?i:ALTER)
ADD			(?i:ADD)
RENAME		(?i:RENAME)
DESC		(?i:DESC)
REFERENCES 	(?i:REFERENCES)
INDEX		(?i:INDEX)
AND			(?i:AND)
DATE		(?i:DATE)     	
FLOAT		(?i:FLOAT)
FOREIGN		(?i:FOREIGN)
ON  		(?i:ON)
TO  		(?i:TO)
CHAR		(?i:CHAR)

NOTEQUAL	(<>)
GEQUAL		(>=)
LEQUAL		(<=)


/* pattern */
NEWLINE			(\r|\n|\r\n)
IDENTIFIER		([A-Za-z][_0-9A-Za-z]*)
IDENTIFIER2 	(`[^`]*`)
VALUE_INT		([0-9]+)
VALUE_FLOAT		((-?[0-9]+)(\.[0-9]+)?)
VALUE_STRING	('[^']*')

SINGLELINE_COMMENT_SIGN (--)|#
MULTILINE_COMMENT_BEGIN (\/\*)
MULTILINE_COMMENT_END (\*\/)

/*下面%%后开始第2段：规则段*/
/*

*/
%%

{WHITESPACE}			{ /* Just ignore */ }
{NEWLINE}				{ yylineno++;	    }

{DATABASE}				{ return DATABASE;	}
{DATABASES}				{ return DATABASES;	}
{TABLE}					{ return TABLE;		}	
{TABLES}				{ 
	yylval.m_sId = yytext;
	return TABLES;	
}
{SHOW}					{ return SHOW;		}	
{CREATE}				{ return CREATE;	}
{DROP}					{ return DROP;		}
{USE}					{ return USE;		}
{PRIMARY}				{ return PRIMARY;	}
{KEY}					{ return KEY;		}
{NOT}					{ return NOT;		}	
{NULL}					{ return NULLTOKEN; }
{INSERT}				{ return INSERT;	}
{INTO}					{ return INTO;		}	
{VALUES}				{ return VALUES;	}
{DELETE}				{ return DELETE;	}
{FROM}					{ return FROM;		}
{WHERE}					{ return WHERE;		}	
{UPDATE}				{ return UPDATE;	}
{SET}					{ return SET;		}
{SELECT}				{ return SELECT;	}
{IS}					{ return IS;		}
{INT}					{ return INTTOKEN;		}	
{VARCHAR}				{ return VARCHARTOKEN;	}
{DEFAULT}				{ return DEFAULT;	}
{CONSTRAINT}			{ return CONSTRAINT;}
{CHANGE}				{ return CHANGE;	}
{ALTER}					{ return ALTER;		}
{ADD}					{ return ADD;		}
{RENAME}				{ return RENAME;	}
{DESC}					{ return DESC;		}
{REFERENCES}			{ return REFERENCES;}
{INDEX}					{ return INDEX;		}
{AND}					{ return AND;		}
{DATE}					{
	yylval.m_sId = yytext;
	return DATETOKEN;	
}
{FLOAT}					{ return FLOATTOKEN;}
{FOREIGN}				{ return FOREIGN;	}
{ON}					{ return ON;		}
{TO}					{ return TO; 		}
{CHAR}					{ return CHARTOKEN; }

{NOTEQUAL}				{ return NOTEQUAL;	}
{GEQUAL}				{ return GEQUAL;	}
{LEQUAL}				{ return LEQUAL;	}

{VALUE_INT}		{							//匹配整数串
	yylval.m_nInt = atoi(yytext);//把识别出的整数串，转换为整型值，存储到yylval的整型成员里，到了yacc里用$n方式引用
	return VALUE_INT;			//向yacc返回: 识别出的记号类型是INTEGER
}
{VALUE_FLOAT}	{
	yylval.m_nFloat = atof(yytext);
	return VALUE_FLOAT;
}
{VALUE_STRING}	{
	yylval.m_nString = yytext;
	yylval.m_nString = yylval.m_nString.substr(1, yylval.m_nString.length() - 2);
	return VALUE_STRING;
}
{SINGLELINE_COMMENT_SIGN}	{
	BEGIN SINGLELINE_COMMENT;			//用BEGIN宏切换到注释状态，去过滤这段注释，下一次lex将只匹配前面带有<COMMENT>的正则式
}
{MULTILINE_COMMENT_BEGIN} {
	BEGIN MULTILINE_COMMENT;
}
{IDENTIFIER}	{							//匹配标识符串，此时串值由yytext保存
	yylval.m_sId = yytext;	//通过yylval向yacc传递识别出的记号的值，由于yylval已定义为struct Type，这里就可以把yytext赋给其m_sId成员，到了yacc里就可以用$n的方式来引用了
	return IDENTIFIER;		//向yacc返回: 识别出的记号类型是IDENTIFIER
}

{IDENTIFIER2}	{
	yylval.m_sId=yytext;
	yylval.m_sId = yylval.m_sId.substr(1, yylval.m_sId.length() - 2);
	return IDENTIFIER;
}

.				{							//.表示除\n以外的其它字符，注意这个规则要放在最后，因为一旦匹配了.就不会匹配后面的规则了(以其它状态<>开头的规则除外)
	yylval.m_cOp = yytext[0];	//由于只匹配一个字符，这时它对应yytext[0]，把该字符存放到yylval的m_cOp成员里，到了yacc里用$n方式引用
	return yytext[0];		//向yacc返回: 识别出的记号类型是OPERATOR
}

<MULTILINE_COMMENT>{MULTILINE_COMMENT_END}	{
	BEGIN INITIAL;
}

<MULTILINE_COMMENT>{NEWLINE}	{
	yylineno++;
}

<MULTILINE_COMMENT>.	{// 忽略注释中的内容
}

<SINGLELINE_COMMENT>{NEWLINE}	{	//注释状态下的规则，只有当前切换到COMMENT状态才会去匹配
	yylineno++;
	BEGIN INITIAL;			//在注释状态下，当遇到换行符时，表明注释结束了，返回初始态
}

<SINGLELINE_COMMENT>.	{//在注释状态下，对其它字符都忽略，即：注释在lex(词法分析层)就过滤掉了，不返回给yacc了
}

%%

//第3段：C函数定义段
int yywrap(void)
{
	puts("-----the file is end");
	return 1;								//返回1表示读取全部结束。如果要接着读其它文件，可以这里fopen该文件，文件指针赋给yyin，并返回0
}
