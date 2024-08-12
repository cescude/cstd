#pragma once

utf8_char_t utf8CharFromC(char c);

/*
  Assumes the front and back of the string end on valid character
  alignments. Use utf8Sync(Front/Back) if you're unsure.
 */
str_t utf8DropChars(str_t s, size count);

/* Will abort() if head isn't the start of a utf8 character */
size utf8BytesNeeded(char head);

/* How many bytes are used by this character? */
size utf8CharLen(utf8_char_t ch);
size utf8StrLen(str_t s);

utf8_char_t utf8FirstChar(str_t s);
utf8_char_t utf8CharAt(str_t s, size index);
size utf8Length(str_t s);
  
bool utf8CharEquals(utf8_char_t a, utf8_char_t b);

