#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
/*
 *enum for the states of the DFA
 */
enum Flags{
	/*
	 * This flag is set when a '/' character is detected. It doesn't
	 * necessarily indicate a beginning of a comment nor an end.It could
	 * just be a division operator or an escaped character within a
	 * string.
	 */
	SINGLE_SLASH_DETECTED,
	/* 
     * This flag is set when a '*' character is detected. If not with in a
	 * string when it is preceeded by a '/' token it indicates that it is
	 * a begining of a multi-line comment.
	 */
	SINGLE_ASTERISK_DETECTED,
	/* 
     * This flag is set when the program is parsing a string and has
	 * been doing so for sometime.That means when a ' or " token is de-
	 * tected this is not immediatly set and is set when the first cha-
	 * racter is read.
	 */
	PARSING_STRING,
	/* 
     * This flag is set when the contents of a multi-line comment is
	 * being read or has just been read.That is this is set after
	 * the '*' token of a beginning of a multi-line comment is read
     */
	PARSING_MULTILINE_COMMENT,
	/* 
   * This flag represents two different states:
	 * -When a token that is't significant to the operation of the
	 * program is being read. Or
	 * -When ' or " token of a begining of a string is read.
	 */
	NONE
};
/* 
 * function declarations that handles each of the above states 
 */
void start(int flag,FILE *from,FILE *to,FILE *err,int currentLine);
void parsingString(int flag,FILE *from,FILE *to,FILE *err,int currentLine);
void singleSlash(int flag,FILE *from,FILE *to,FILE *err,int currentLine);
void singleAstrisk(int flag,FILE *from,FILE *to,FILE *err,int currentLine);
void endofFile(int flag,FILE *from,FILE *to,FILE *err,int currentLine);
void midState(int flag,FILE *from,FILE *to,FILE *err,int currentLine);
/* 
 * The main function takes a file from standard input and 
 * outputs the decommented file in a standard output file.
 */
int main(int argc,char *argv[]){
 FILE *file;
 file=stdin;
 if(file==NULL){
  fprintf(stderr,"Unable to load file the file. it does't exist or the "
		  "program does't have permission to access it");
  exit(EXIT_FAILURE);
 }
 start(NONE,file,stdout,stderr,1);
 return 0;
}
/*
 * This function will be called when the program is initialized and
 * according to the first token encountered will then call the
 * appropriate states.
 * flag:always a value NONE indicating that no flags are set.
 * from:the file pointer to read from
 * to:the file pointer to write the output to.
 * err:the file pointer to write errors should there be any.
 * currentLine:the current line being parsed from the from file.
 */
void start(int flag,FILE *from,FILE *to,FILE *err,int currentLine){
 char aCharacter=getc(from);
 if(aCharacter==EOF){
  endofFile(flag,from,to,err,currentLine);
 }
 if(aCharacter=='\'' || aCharacter=='"'){
   putc(aCharacter,to);
   parsingString(PARSING_STRING,from,to,err,currentLine);
 }else if(aCharacter=='/'){
   singleSlash(SINGLE_SLASH_DETECTED,from,to,err,currentLine);
  }else{
   if(aCharacter=='\n'){
	currentLine++;
   }
   putc(aCharacter,to);
   midState(flag,from,to,err,currentLine);
  }
}
/*
 * This function represents the state when a string is being parsed.
 * flag:either NONE or PARSING_STRING. Distinction helps
 *      differenciate between the begining and end of string literal
 *      respectively.
 * from:the file pointer to read from
 * to:the file pointer to write the output to.
 * err:the file pointer to write errors should there be any.
 * currentLine:the current line being parsed from the from file.
*/
void parsingString(int flag,FILE *from,FILE *to,FILE *err,int currentLine){
 char aCharacter=getc(from);
 static int escapeNext;
 if(aCharacter==EOF){
  endofFile(flag,from,to,err,currentLine);
 }
 if(flag!=PARSING_STRING){
  escapeNext=0;
 }
 if((aCharacter=='\'' || aCharacter=='"')&& !escapeNext){
  if(flag==PARSING_STRING){
   putc(aCharacter,to);
   midState(NONE,from,to,err,currentLine);
  }
  else{
   putc(aCharacter,to);
   parsingString(PARSING_STRING,from,to,err,currentLine);
  }
 }
 else if(aCharacter=='\\' && !escapeNext){
  putc(aCharacter,to);
  escapeNext=1;
  parsingString(PARSING_STRING,from,to,err,currentLine);
 }
 else if(escapeNext){
  putc(aCharacter,to);
  parsingString(PARSING_STRING,from,to,err,currentLine);
  escapeNext=0;
 }else if(aCharacter=='\n'){
   currentLine++;
   putc(aCharacter,to);
   parsingString(PARSING_STRING,from,to,err,currentLine);
 }
 else{
   putc(aCharacter,to);
   parsingString(PARSING_STRING,from,to,err,currentLine);
 }
}
/* This function represents a state when a forward slash is detected.
 * flag:either SINGLE_SLASH_DETECTED or PARSING_MULTILINE_COMMENT.
 *      Distinction helps differenciate between the begining and end
 *      of comment respectively.
 * from:the file pointer to read from
 * to:the file pointer to write the output to.
 * err:the file pointer to write errors should there be any.
 * currentLine:the current line being parsed from the from file.
 */
void singleSlash(int flag,FILE *from,FILE *to,FILE *err,int currentLine){
 char aCharacter=getc(from);
 if(aCharacter==EOF){
  endofFile(flag,from,to,err,currentLine);
 }
 if(flag==PARSING_MULTILINE_COMMENT){
  putc(' ',to);
  ungetc(aCharacter,from);
  midState(PARSING_MULTILINE_COMMENT,from,to,err,currentLine);
 }
 if(aCharacter=='*'){
  singleAstrisk(SINGLE_ASTERISK_DETECTED,from,to,err,currentLine);
 }
 else{
  putc('/',to);
  putc(aCharacter,to);
  if(aCharacter=='\n'){
  	currentLine++;
  }
  midState(NONE,from,to,err,currentLine);
 }
}
/* 
 * This function reprsents the state when * token of the begining of a
 * comment is read.This function is always called after the singleSlash
 * method is called. When the * token as a multiplication operator is
 * being parsed it wont be called.
 * flag: either SINGLE_ASTERISK_DETECTED or PARSING_MULTILINE_COMMENT.
 *       Distinction differenciates between the begining and end
 *       of comment respectively.
 * from:the file pointer to read from
 * to:the file pointer to write the output to.
 * err:the file pointer to write errors should there be any.
 * currentLine:the current line being parsed from the from file
 */
void singleAstrisk(int flag,FILE *from,FILE *to,FILE *err,int currentLine){
 char aCharacter=getc(from);
 static int beginComment;
 if(aCharacter==EOF){
  endofFile(flag,from,to,err,beginComment);
 }
 if(flag==SINGLE_ASTERISK_DETECTED){
  beginComment=currentLine;
 }
 if(aCharacter=='*' && flag==PARSING_MULTILINE_COMMENT){
  char nextCharacter=getc(from);
  if(nextCharacter=='/'){
   beginComment=0;
   singleSlash(PARSING_MULTILINE_COMMENT,from,to,err,currentLine);
  }else{
	ungetc(nextCharacter,from);
	singleAstrisk(PARSING_MULTILINE_COMMENT,from,to,err,currentLine);
  }
 }
 else if(aCharacter=='\n'){
  currentLine++;
  putc('\n',to);
  singleAstrisk(PARSING_MULTILINE_COMMENT,from,to,err,currentLine);
 }
 else{
  singleAstrisk(PARSING_MULTILINE_COMMENT,from,to,err,currentLine);
 }
}
/*
 * This function is called when an EOF is detected in the program
 * file being parsed.
 * flag: it could be any of the values depending on at which state the
 *       program was before the EOF is read.But when the value is
 *       PARSING_MULTILINE_COMMENT it indicates that an opened comment
 *       is not closed.
 * from:the file pointer to read from
 * to:the file pointer to write the output to.
 * err:the file pointer to write errors should there be any.
 * currentLine:When flag is PARSING_MULTILINE_COMMENT it will contain
 * 	           the line number of the begining of the unclosed comment.
 * 	           else the current line being parsed from the from file,
 * 	           i.e the last line. But the second case is't relevant
 * 	           to the operation of the program, it is just a placeholder.
 */
void endofFile(int flag,FILE *from,FILE *to,FILE *err,int currentLine){
 if(flag==PARSING_MULTILINE_COMMENT){
  fprintf(err,"Unterminated comment at line %d",currentLine);
  fclose(from);
  fclose(to);
  fclose(err);
  exit(EXIT_FAILURE);
 }
 fclose(from);
 fclose(to);
 fclose(err);
 exit(EXIT_SUCCESS);
}
void midState(int flag,FILE *from,FILE *to,FILE *err,int currentLine){
 char aCharacter;
 aCharacter=getc(from);
 if(aCharacter==EOF){
  endofFile(flag,from,to,err,currentLine);
 }
 if(aCharacter=='"' || aCharacter=='\''){
  putc(aCharacter,to);
  parsingString(NONE,from,to,err,currentLine);
 }
 else if(aCharacter=='/' && flag==PARSING_MULTILINE_COMMENT){
  midState(NONE,from,to,err,currentLine);
 }
 else if(aCharacter=='/'){
  singleSlash(SINGLE_SLASH_DETECTED,from,to,err,currentLine);
 }else{
   if(aCharacter=='\n'){
	   currentLine++;
   }
   putc(aCharacter,to);
   midState(NONE,from,to,err,currentLine);
 }
}